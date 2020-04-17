package IP::Noise::C::Parser;

use strict;

use IP::Noise::C::Parser::Exception;

use IP::Noise;

sub parse_natural_number
{
    my $stream = shift;
    
    my $orig_line = $stream->peak_line();

    my $line = $orig_line;

    # Strip the leading whitespace
    $line =~ s/^\s+//;

    # If there is a decimal integer here
    # We have to make sure it is not followed by anthing else, 
    # such as a decimal point.
    if ($line =~ /^(\d+)/)
    {
        my $number = $1;
        
        # Make sure it is not beyond the int32 limit
        if ($number > (2 ** 31))
        {
            die IP::Noise::C::Parser::Exception->new(
                'text' => "Integer value out of range",
                'line' => $stream->get_line_num(),
                'context' => $number,
                );
        }

        # Remove that from the line;
        $line =~ s/^(\d+)//; 
        $stream->increment_ptr(length($orig_line)-length($line));

        return $number;
    }
    else
    {
        die IP::Noise::C::Parser::Exception->new(
                'text' => "Illegal integer value",
                'line' => $stream->get_line_num(),
                'context' => $line,
                );
    }
}

sub parse_prob
{
    my $stream = shift;
    
    my $orig_line = $stream->peak_line();

    my $line = $orig_line;

    # Strip the leading whitespace
    $line =~ s/^\s+//;

    # If there is a decimal fraction here.
    if ($line =~ /^((\d+\.\d+)|1|0)/)
    {
        my $prob = $1;

        if (($prob < 0) || ($prob > 1))
        {
            die IP::Noise::C::Parser::Exception->new(
                'text' => "Probability value out of range",
                'line' => $stream->get_line_num(),
                'context' => $prob,
                );
        }

        # Remove that from the line;
        $line =~ s/^((\d+\.\d+)|1|0)//; 
        $stream->increment_ptr(length($orig_line)-length($line));

        return $prob;
    }
    else
    {
        die IP::Noise::C::Parser::Exception->new(
                'text' => "Illegal probability value",
                'line' => $stream->get_line_num(),
                'context' => $line,
                );
    }
}

sub parse_id_string
{
    my $stream = shift;
    
    my $orig_line = $stream->peak_line();

    my $line = $orig_line;

    # Strip the leading whitespace
    $line =~ s/^\s+//;

    if ($line =~ /^(\w+)/)
    {
        my $string = $1;
        
        # Make sure it is not beyond the int32 limit
        if (length($string) > IP::Noise::get_max_id_string_len())
        {
            die IP::Noise::C::Parser::Exception->new(
                'text' => "String is too long",
                'line' => $stream->get_line_num(),
                'context' => $string,
                );
        }

        # Remove that from the line;
        $line =~ s/^(\w+)//; 
        $stream->increment_ptr(length($orig_line)-length($line));

        return $string;
    }
    else
    {
        die IP::Noise::C::Parser::Exception->new(
                'text' => "Illegal String ID",
                'line' => $stream->get_line_num(),
                'context' => $line,
                );
    }
}

sub parse_constant_char
{
    my $stream = shift;
    my $char = shift;

    my $orig_line = $stream->peak_line();

    my $line = $orig_line;

    $line =~ s/^\s+//;

    if (substr($line, 0, 1) eq $char)
    {
        $line = substr($line, 1);
        $stream->increment_ptr(length($orig_line)-length($line));
        return 0;
    }
    else
    {
        die IP::Noise::C::Parser::Exception->new(
                'text' => "Expected '$char' but did not encounter it",
                'line' => $stream->get_line_num(),
                'context' => $line,
                );
    }
}

sub parse_move_to
{
    my $stream = shift;

    parse_constant_char($stream, "{");
    
    my %move_tos = ();

    my $sum = 0;

    while (1)
    {
        $stream->read_next_line();

        my $orig_line = $stream->peak_line();

        my $line = $orig_line;

        $line =~ s/^\s+//;

        # { - to satisfy gvim for balancing matching parens
        if ($line =~ /\}/)
        {
            last;            
        }
        
        # If the line is empty skip to the next line
        if ($line eq "")
        {
            next;
        }

        my $prob = parse_prob($stream);
        parse_constant_char($stream, "=");
        my $state = parse_id_string($stream);
        my $state_lc = lc($state);

        $sum += $prob;

        if ($sum > 1)
        {
            die IP::Noise::C::Parser::Exception->new(
                'text' => "Proabibilities' sum in move_to exceed 1",
                'line' => $stream->get_line_num(),
                'context' => $prob,
                );
        }

        if (exists($move_tos{$state_lc}))
        {
            die IP::Noise::C::Parser::Exception->new(
                'text' => "Duplicate state in move_to",
                'line' => $stream->get_line_num(),
                'context' => $state,
                );
        }

        $move_tos{$state_lc} = { 'state' => $state, 'prob' => $prob };
    }

    if ($sum != 1)
    {
        die IP::Noise::C::Parser::Exception->new(
            'text' => "Proabibilities in move_to do not sum to 1",
            'line' => $stream->get_line_num(),
            );
    }

    # Convert the hash to mixed-case
    return { (map { $_->{'state'} => $_->{'prob'} } values(%move_tos)) };
}

sub parse_delay_type
{
    my $stream = shift;
 
    my $id = parse_id_string($stream);

    $id = lc($id);

    if ($id eq "e")
    {
        parse_constant_char($stream, "(");
        my $lambda = parse_natural_number($stream);
        parse_constant_char($stream, ")");

        return 
            {
                'type' => "exponential",
                'lambda' => $lambda,
            };
    }
    elsif ($id eq "u")
    {
        parse_constant_char($stream, "(");
        my $t_start = parse_natural_number($stream);
        parse_constant_char($stream, ",");
        my $t_end = parse_natural_number($stream);
        parse_constant_char($stream, ")");

        return
            {
                'type' => "uniform",
                'start' => $t_start,
                'end' => $t_end,
            };
    }
    elsif ($id eq "generic")
    {
        parse_constant_char($stream, "{");

        my @points;
        # We initialize it to -1 so it will be lesser than 0.
        my $prev_prob = -1;
        while(1)
        {
            $stream->read_next_line();

            my $orig_line = $stream->peak_line();
            
            my $line = $orig_line;

            $line =~ s/^\s+//;
    
            # { - to satisfy gvim for balancing matching parens
            if ($line =~ /\}/)
            {
                last;            
            }
            
            # If the line is empty skip to the next line
            if ($line eq "")
            {
                next;
            }

            my $prob = parse_prob($stream);
            
            # Check that the first probability which is inputted is 0.
            if (scalar(@points) == 0)
            {
                if ($prob != 0)
                {
                    die IP::Noise::C::Parser::Exception->new(
                        'text' => "Probabilities in generic type should start from 0",
                        'line' => $stream->get_line_num(),
                        'context' => $prob,
                        );
                }            
            }
            
            # Verify that the probabilities are in ascending order
            if ($prob < $prev_prob)
            {
                die IP::Noise::C::Parser::Exception->new(
                    'text' => "Probabilities in generic type must be ascending",
                    'line' => $stream->get_line_num(),
                    'context' => $prob,
                    );
            }
            
            parse_constant_char($stream, "=");
            my $delay = parse_natural_number($stream);


            push @points, { 'prob' => $prob, 'delay' => $delay };

            $prev_prob = $prob;
        }
    
        # Verify that the last probability is 1
        if ($points[$#points]->{'prob'} != 1)
        {
            die IP::Noise::C::Parser::Exception->new(
                'text' => "Probabilities in generic type should end at 1",
                'line' => $stream->get_line_num(),
                );            
        }


        return 
            {
                'type' => "generic",
                'points' => [ @points ],
            };
    }
    else    # A different id
    {
        die IP::Noise::C::Parser::Exception->new(
                'text' => "Unknown delay type",
                'line' => $stream->get_line_num(),
                'context' => $id,
                );
    }
}

sub parse_state
{
    my $stream = shift;

    my $state_name = parse_id_string($stream);

    my ($drop_prob, $delay_prob, $time_factor, $delay_type, $move_to);

    # Some initial values:
    $drop_prob = 0;
    $delay_prob = 0;
    
    parse_constant_char($stream, "{");

    while (1)
    {
        $stream->read_next_line();
        
        my $line = $stream->peak_line();

        $line =~ s/^\s+//;
        # { - to satisfy gvim

        if ($line =~ /}/)
        {
            
            # Some Sanity Checks
            if ($delay_prob + $drop_prob > 1)
            {
                die IP::Noise::C::Parser::Exception->new(
                    'text' => "The state's delay and drop probabilites exceed 1.0",
                    'line' => $stream->get_line_num(),
                    );                
            }

            if (!defined($time_factor))
            {
                die IP::Noise::C::Parser::Exception->new(
                    'text' => "The state's time factor was not defined",
                    'line' => $stream->get_line_num(),
                    );                                
            }

            if (!defined($move_to))
            {
                die IP::Noise::C::Parser::Exception->new(
                    'text' => "The state's move_tos were not defined",
                    'line' => $stream->get_line_num(),
                    );
            }

            if (($delay_prob > 0) && (!defined($delay_type)))
            {
                die IP::Noise::C::Parser::Exception->new(
                    'text' => "The state's delay type was not defined",
                    'line' => $stream->get_line_num(),
                    );
            }
            
            # Return the packed state
            return {
                'name' => $state_name,
                'delay_prob' => $delay_prob,
                'drop_prob' => $drop_prob,
                'time_factor' => $time_factor,
                'delay_type' => $delay_type,
                'move_to' => $move_to,
            };
        }

        if ($line eq "")
        {
            next;
        }

        my $id = parse_id_string($stream);

        parse_constant_char($stream, "=");

        $id = lc($id);

        if ($id eq "drop")
        {
            $drop_prob = parse_prob($stream);
        }
        elsif ($id eq "delay")
        {
            $delay_prob = parse_prob($stream);
        }
        elsif ($id eq "time_factor")
        {
            $time_factor = parse_natural_number($stream)
        }
        elsif ($id eq "delay_type")
        {
            $delay_type = parse_delay_type($stream)
        }
        elsif ($id eq "move_to")
        {
            $move_to = parse_move_to($stream)
        }
        else
        {
            die IP::Noise::C::Parser::Exception->new(
                'text' => "Unknown state parameter",
                'line' => $stream->get_line_num(),
                'context' => $line,
                );
        }
    }
}

sub parse_range_spec
{
    my $stream = shift;

    my $param_name = shift;

    my $orig_line = $stream->peak_line();

    my $line = $orig_line;

    $line =~ s/\s//g;

    if ($line =~ /^(\!?)(\d+)\<$param_name\<(\d+)/)
    {
        my $type = $1;
        my $min = $2;
        my $max = $3;

        if ($max >= $min)
        {
            return 
                { 
                    'type' => (($type eq "!") ? "not-between" : "between"), 
                    'min' => $min, 
                    'max' => $max 
                };
        }
        else
        {
            die IP::Noise::C::Parser::Exception->new(
                'text' => "The minimum of the range is greater than the maximum",
                'line' => $stream->get_line_num(),
                'context' => $orig_line,
            );        
        }
    }
    elsif ($line =~ /^(\!?)(\d+)\>$param_name\>(\d+)/)
    {
        my $type = $1;
        my $max = $2;
        my $min = $3;
        
        if ($max >= $min)
        {
            return 
                { 
                    'type' => (($type eq "!") ? "not-between" : "between"), 
                    'min' => $min, 
                    'max' => $max 
                };
        }
        else
        {
            die IP::Noise::C::Parser::Exception->new(
                'text' => "The minimum of the range is greater than the maximum",
                'line' => $stream->get_line_num(),
                'context' => $orig_line,
            );
        }    
    }
    elsif ($line =~ /^$param_name(<|==|>|<=|>=|\!=)(\d+)$/)
    {
        my $type = $1;
        my $arg = $2;

        if ($type eq "<")
        {
            return { 'type' => "lt", 'max' => $arg };
        }
        elsif ($type eq "==")
        {
            return { 'type' => "eq", "equal_to" => $arg };            
        }
        elsif ($type eq ">")
        {
            return { 'type' => "gt", 'min' => $arg };            
        }
        elsif ($type eq "<=")
        {
            return { 'type' => "le", 'max' => $arg };            
        }
        elsif ($type eq ">=")
        {
            return { 'type' => "ge", 'min' => $arg };
        }
        elsif ($type eq "!=")
        {
            return { 'type' => "ne", 'equal_to' => $arg };
        }
    }

    die IP::Noise::C::Parser::Exception->new(
        'text' => "Incorrect range-type specifier for $param_name",
        'line' => $stream->get_line_num(),
        'context' => $orig_line,
    );

    # TODO : Add "5 < l", "10 > l" , etc.
}

my %protocol_names = 
(
    'tcp' => 6,
    'udp' => 17,
    'icmp' => 1,
    'igmp' => 2,
    'ospf' => 89,
);

sub parse_protocols_list
{
    my $stream = shift;

    my %ret = ();

    my $orig_line = $stream->peak_line();
    my $line = $orig_line;
    my $invert = 0;

    $line =~ s/\s//g;

    if ($line =~ /^\!/)
    {
        $invert = 1;
        $line =~ s/^\!//;
    }

    my @components = split(/,/, $line);
    
    foreach my $prot (@components)
    {
        $prot = lc($prot);
        if ($prot =~ /^\d+$/)
        {
            if ($prot > 255)
            {
                die IP::Noise::C::Parser::Exception->new(
                    'text' => "Protocol number greater than 255",
                    'line' => $stream->get_line_num(),
                    'context' => $orig_line,
                );
            }
            $ret{$prot} = 1;
        }
        else
        {
            if (exists($protocol_names{$prot}))
            {
                $ret{$protocol_names{$prot}} = 1;
            }
            else
            {
                die IP::Noise::C::Parser::Exception->new(
                    'text' => "Unknown Protocol \"$prot\"",
                    'line' => $stream->get_line_num(),
                    'context' => $orig_line,
                );
            
            }
        }
    }

    return 
        { 
            'type' => ($invert ? 'exclude' : 'only'), 
            'protocols' => \%ret,
        };
}

sub parse_ip_spec
{
    my $stream = shift;

    my $orig_line = $stream->peak_line();
    my $line = $orig_line;
    
    $line =~ s/\s//g;

    my @components = split(/;/, $line);

    my @ip_filters = ();
    
    foreach my $ip_filter (@components)
    {
        my $ip = $ip_filter;
        my $net_mask_width = 1;
        my @port_ranges = ();

        $ip =~ s/(\/|:),*$//;
        if ($ip =~ /^\d+\.\d+\.\d+\.\d+/)
        {
            my @ip_components = split(/\./, $ip);
            foreach my $c (@ip_components)
            {
                if ($c > 255)
                {
                    die IP::Noise::C::Parser::Exception->new(
                        'text' => "IP Components out of range - \"$c\"",
                        'line' => $stream->get_line_num(),
                        'context' => $ip_filter
                        );
                }
            }
        }
        else
        {
            die IP::Noise::C::Parser::Exception->new(
                'text' => "Incorrect IP - \"$ip\"",
                'line' => $stream->get_line_num(),
                'context' => $ip_filter,
                );
        }

        my $rest_of_ip_filter =~ s/^\d+\.\d+\.\d+\.\d+//;

        if ($rest_of_ip_filter =~ /^\//)
        {
            $rest_of_ip_filter =~ s/^\///;
            if ($rest_of_ip_filter =~ /^(\d+)/)
            {               
                $net_mask_width = $1;
                $rest_of_ip_filter =~ s/^\d+//;
                if (($net_mask_width == 0) || ($net_mask_width > 32))
                {
                    die IP::Noise::C::Parser::Exception->new(
                        'text' => "Net Mask Width Out of range in IP Spec",
                        'line' => $stream->get_line_num(),
                        'context' => $ip_filter,
                        );
                }
            }
            else
            {
                die IP::Noise::C::Parser::Exception->new(
                    'text' => "Incorrect Net mask width",
                    'line' => $stream->get_line_num(),
                    'context' => $ip_filter,
                );
            }
        }

        if ($rest_of_ip_filter =~ /^\:/)
        {
            $rest_of_ip_filter =~ s/^\://;

            my @which_ports_are_selected = (0) x 0x10000;

            my @ports = split(/,/, $rest_of_ip_filter);
            
            foreach my $port (@ports)
            {
                if ($port =~ /^(\d+)-(\d+)/)
                {
                    my $start = $1;
                    my $end = $2;

                    if ($start > $end)
                    {
                        die IP::Noise::C::Parser::Exception->new(
                            'text' => "Port range is reversed - \"$port\"",
                            'line' => $stream->get_line_num(),
                            'context' => $ip_filter,
                        );
                    }
                    if ($start > 0xFFFF)
                    {
                        die IP::Noise::C::Parser::Exception->new(
                            'text' => "Port out of range - \"$start\"",
                            'line' => $stream->get_line_num(),
                            'context' => $ip_filter,
                        );                    
                    }
                    if ($end > 0xFFFF)
                    {
                        die IP::Noise::C::Parser::Exception->new(
                            'text' => "Port out of range - \"$end\"",
                            'line' => $stream->get_line_num(),
                            'context' => $ip_filter,
                        );                    
                    }
                    # Everything is OK. Mark the ports in this port range as enabled.

                    @which_ports_are_selected[$start .. $end] = ((1) x 0x100000);
                }
                elsif ($port =~ /^\d+/)
                {
                    if ($port > 0xFFFF)
                    {
                        die IP::Noise::C::Parser::Exception->new(
                            'text' => "Port out of range - \"$port\"",
                            'line' => $stream->get_line_num(),
                            'context' => $ip_filter,
                        );
                    }

                    $which_ports_are_selected[$port] = 1;
                }
                else
                {
                    die IP::Noise::C::Parser::Exception->new(
                        'text' => "Incorrect syntax for port range - \"$port\"",
                        'line' => $stream->get_line_num(),
                        'context' => $ip_filter,
                        );                                        
                }
            }
            my $port = 0;
            while ($port < 0x10000)
            {
                while ((! $which_ports_are_selected[$port]) && ($port < 0x10000))
                {
                    $port++;
                }
                my $start_port = $port;
                while (($which_ports_are_selected[$port]) && ($port < 0x10000))
                {
                    $port++;
                }
                push @port_ranges, { 'start' => $start_port, 'end' => ($port-1) };
            }
        }

        push @ip_filters, {
            'ip' => $ip,
            'ports' => \@port_ranges,
            'netmask_width' => $net_mask_width,            
        };
    }

    return \@ip_filters;
}

sub parse_chain
{
    my $stream = shift;

    my $chain_name = parse_id_string($stream);

    my (%states, $length_specifier, $tos_specifier, $protocols, $source, $dest);
    
    # Assign some default values
    $length_specifier = { 'type' => 'all' };
    $tos_specifier = { 'type' => 'all' };
    $protocols = { 'type' => 'all' };
    $source = { 'type' => 'none' };
    $dest = { 'type' => 'none' };
   
    parse_constant_char($stream, "{");
    while (1)
    {
        $stream->read_next_line();

        my $line = $stream->peak_line();

        $line =~ s/^\s+//;
        # { - to satisfy gvim

        if ($line =~ /}/)
        {
            # Some Sanity Checks
            
            # Check that the move_to's move only to defined states
            my %all_move_tos = 
                (map 
                    { 
                        map { lc($_) => 1 } keys(%{$_->{'move_to'}}) 
                    } 
                    values(%states)
                );

            foreach my $key (keys(%all_move_tos))
            {
                if (! exists($states{$key}))
                {
                    die IP::Noise::C::Parser::Exception->new(
                        'text' => "There is a moves to an undefined state",
                        'line' => $stream->get_line_num(),
                        'context' => $key,
                    );
                }
            }
            
            # Make the hash entries have mixed case.
            my %states_to_ret = (map { $_->{'name'} => $_ } values(%states));

            return {
                'name' => $chain_name,
                'states' => \%states_to_ret, 
                'length_spec' => $length_specifier,
                'tos_spec' => $tos_specifier,
                'protocols' => $protocols,
                'source' => $source,
                'dest' => $dest,
            };
        }

        if ($line eq "")
        {
            next;
        }

        my $id = parse_id_string($stream);

        $id = lc($id);

        if ($id eq "state")
        {
            my $state = parse_state($stream);
            my $state_name_lc = lc($state->{'name'});
            # Check if a state by that name already exists and if so - croak
            if (exists($states{$state_name_lc}))
            {
                die IP::Noise::C::Parser::Exception->new(
                    'text' => "A state by the same name already exists",
                    'line' => $stream->get_line_num(),
                    'context' => $state->{'name'},
                );
            }
            $states{$state_name_lc} = $state;
        }
        elsif ($id eq "length")
        {
            parse_constant_char($stream, "=");
            $length_specifier = parse_range_spec($stream, "l");
            # Sanity check to make sure they are not greater than 64K
            foreach my $param (qw(min max equal_to))
            {
                if (exists($length_specifier->{$param}))
                {
                    if ($length_specifier->{$param} > 0xFFFF)
                    {
                        die IP::Noise::C::Parser::Exception->new(
                            'text' => "Packet Length cannot be greater than 65535",
                            'line' => $stream->get_line_num(),
                            'context' => $line,
                        );
                    }
                }
            }
        }
        elsif ($id eq "tos")
        {
            parse_constant_char($stream, "=");
            $tos_specifier = parse_range_spec($stream, "tos");
            # Sanity check to make sure they are not greater than 64
            foreach my $param (qw(min max equal_to))
            {
                if (exists($tos_specifier->{$param}))
                {
                    if ($tos_specifier->{$param} > 0x40)
                    {
                        die IP::Noise::C::Parser::Exception->new(
                            'text' => "Packet TOS cannot be greater than 64",
                            'line' => $stream->get_line_num(),
                            'context' => $line,
                        );
                    }
                }
            }
        }
        elsif ($id eq "protocol")
        {
            parse_constant_char($stream, "=");
            $protocols = parse_protocols_list($stream);
        }
        elsif ($id eq "source")
        {
            parse_constant_char($stream, "=");
            $source = parse_ip_spec($stream);
        }
        elsif ($id eq "dest")
        {
            parse_constant_char($stream, "=");
            $dest = parse_ip_spec($stream);
        }
        else
        {
            die IP::Noise::C::Parser::Exception->new(
                'text' => "Unknown chain parameter",
                'line' => $stream->get_line_num(),
                'context' => $line,
                );
        }
    }
}


1;
