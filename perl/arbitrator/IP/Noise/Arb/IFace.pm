#
# This is the arbitrator's interface which updates the configuration
# of the arbitrator based on instructions it receives from the translator.
#
#

package IP::Noise::Arb::IFace;

use strict;

use IP::Noise;

use IP::Noise::Conn;

#use Thread;

my $arb_string_len = IP::Noise::get_max_id_string_len() + 1;

# Usage: 
#      my $arb = IP::Noise::Arb->new($connection_handle);
sub new
{
    my $class = shift;

    my $self = {};

    bless $self, $class;

    $self->initialize(@_);

    return $self;
}

sub initialize
{
    my $self = shift;

    my $data = shift;

    my $data_lock = shift;

    my $flags = shift;

    #$self->{'conn'} = IP::Noise::Conn->new(1);

    $self->{'data'} = $data;
    $self->{'data_lock'} = $data_lock;
    $self->{'flags'} = $flags;

    $data->{'chains'} = [];
    $data->{'chains_map'} = {};

    # This member specifies whether to continue the loop.
    $self->{'continue'} = 1;

    $self->{'last_chain'} = -1;

    return 0;
}

sub read_opcode
{
    my $self = shift;
    
    my $conn = $self->{'conn'};
    
    my $opcode_proto = $conn->conn_read(4);

    return unpack("V", $opcode_proto);
}

# This table matches each opcode with its arguments, out arguments,
# and handler function.
my %operations =
(
    0x0 =>
    {
        'params' => [ "string" ],
        'out_params' => [ "int" ],
        'handler' => \&handler_new_chain,
    },
    0x1 =>
    {
        'params' => [ "chain", "string"],
        'out_params' => [ "int" ],
        'handler' => \&handler_new_state,
    },
    0x2 =>
    {
        'params' => [ "chain", "int", "int" ],
        'handler' => \&handler_set_move_probs,
    },
    0x3 =>
    {
        'params' => [ "chain", "ip_packet_filter" ],
        'handler' => \&handler_set_source,
    },
    0x4 =>
    {
        'params' => [ "chain", "ip_packet_filter" ],
        'handler' => \&handler_set_dest,
    },
    0x5 =>
    {
        'params' => [ "chain", "int", "bool" ],
        'handler' => \&handler_set_protocol,
    },
    0x6 =>
    {
        'params' => [ "chain", "int", "bool" ],
        'handler' => \&handler_set_tos,
    },
    0x8 =>
    {
        'params' => [ "chain", "int" ],
        'handler' => \&handler_set_length_min,
    },
    0x9 =>
    {
        'params' => [ "chain", "int" ],
        'handler' => \&handler_set_length_max,
    },
    0xa =>
    {   
        'params' => [ "chain", "which_packet_length" ],
        'handler' => \&handler_set_which_length,
    },
    0x19 =>
    {
        'params' => [],
        'handler' => \&handler_clear_all,
    },
    0xE =>
    {
        'params' => [ "chain", "state", "prob", "prob" ],
        'handler' => \&handler_set_drop_delay_prob,
    },
    0xF => 
    {
        'params' => [ "chain", "state", "delay_function_type" ],
        'handler' => \&handler_set_delay_type,
    },
    0x10 =>
    {
        'params' => [ "chain", "state", "split_linear_points" ],
        'handler' => \&handler_set_split_linear_points,
    },
    0x11 =>
    {    
        'params' => [ "chain", "state", "lambda" ],
        'handler' => \&handler_set_lambda,
    },
    0x13 =>
    {
        'params' => [ "chain", "state", "delay_type" ],
        'handler' => \&handler_set_time_factor,
    },
    0x14 =>
    {
        'params' => [ "chain", "state", "prob" ],
        'handler' => \&handler_set_stable_delay_prob,
    },
    0x10000 =>
    {
        'params' => [],
        'handler' => \&handler_end_connection,
    },
);

sub handler_clear_all
{
    my $self = shift;

    my $data = $self->{'data'};

    print "Clear All!\n";
    
    # Delete all the chains.
    @{$data->{'chains'}} = ();
    %{$data->{'chains_map'}} = ();

    return 0;
}

sub handler_new_chain
{
    my $self = shift;

    my $data = $self->{'data'};

    # TODO: Make sure we do not allocate two chains by the same name

    print "New Chain!\n";

    my $name = shift;

    push @{$data->{'chains'}}, 
        { 
            'name' => $name, 
            'states' => [], 
            'states_map' => {},
            'source' => { 'type' => "none" },
            'dest' => { 'type' => "none" },
            'protocols' => ("\xFF" x 32),
            'tos' => ("\xFF" x 8),
            'length' => {
                'type' => "all",
                'min' => 0,
                'max' => 65535
                },
        };
    
    my $index = scalar(@{$data->{'chains'}})-1;

    $data->{'chains_map'}->{lc($name)} = $index;

    $self->{'last_chain'} = $index;

    $self->{'last_state'} = -1;

    return (0, $index);
}

sub handler_end_connection
{
    my $self = shift;

    $self->{'continue'} = 0;
}

sub handler_new_state
{
    my $self = shift;

    my $chain_index = shift;

    my $state_name = shift;

    print "New State!\n";

    # TODO: Sanity check that there isn't another state by that name.

    my $data = $self->{'data'};

    my $chain = $data->{'chains'}->[$chain_index];

    my $states_ref = $chain->{'states'};

    # One more state was added so we need to have one more entry in every row of the matrix.
    foreach my $state (@$states_ref)
    {
        push @{$state->{'move_tos'}}, 0;
    }
    
    push @$states_ref, 
        { 
            'name' => $state_name , 
            'drop_prob' => 0, 
            'delay_prob' => 0,
            'delay_type' => 
                {
                    'type' => "none",
                },
            'time_factor' => 1000,
            'stable_delay_prob' => 0,
        };

    my $index = scalar(@$states_ref) - 1;

    $chain->{'states_map'}->{lc($state_name)} = $index;

    $self->{'last_state'} = $index;

    $states_ref->[$index]->{'move_tos'} = [ (0) x $index , 1 ]; 

    return (0, $index);
}

sub handler_set_drop_delay_prob
{
    my $self = shift;

    my $chain_index = shift;
    my $state_index = shift;
    my $drop_prob = shift;
    my $delay_prob = shift;

    # TODO: Add sanity check that the sum of drop+delay is lesser than 1.

    my $data = $self->{'data'};

    my $state = $data->{'chains'}->[$chain_index]->{'states'}->[$state_index];

    $state->{'drop_prob'} = $drop_prob;
    $state->{'delay_prob'} = $delay_prob;

    return 0;
}

sub handler_set_delay_type
{
    my $self = shift;
    my $chain_index = shift;
    my $state_index = shift;
    my $delay_type = shift;
    
    my $data = $self->{'data'};

    my $state = $data->{'chains'}->[$chain_index]->{'states'}->[$state_index];

    $state->{'delay_type'}->{'type'} = $delay_type;

    return 0;
}

sub handler_set_split_linear_points
{
    my $self = shift;
    my $chain_index = shift;
    my $state_index = shift;
    my $points = shift;

    my $data = $self->{'data'};

    my $state = $data->{'chains'}->[$chain_index]->{'states'}->[$state_index];

    if ($state->{'delay_type'}->{'type'} eq "generic")
    {
        $state->{'delay_type'}->{'points'} = $points;
    }

    return 0;
}

sub handler_set_lambda
{
    my $self = shift;
    my $chain_index = shift;
    my $state_index = shift;
    my $lambda = shift;
    
    my $data = $self->{'data'};

    my $state = $data->{'chains'}->[$chain_index]->{'states'}->[$state_index];

    if ($state->{'delay_type'}->{'type'} eq "exponential")
    {
        $state->{'delay_type'}->{'lambda'} = $lambda;
    }

    return 0;
}

sub handler_set_time_factor
{
    my $self = shift;
    my $chain_index = shift;
    my $state_index = shift;
    my $time_factor = shift;
    
    my $data = $self->{'data'};

    my $state = $data->{'chains'}->[$chain_index]->{'states'}->[$state_index];

    $state->{'time_factor'} = $time_factor;

    return 0;
}

sub handler_set_stable_delay_prob
{
    my $self = shift;
    my $chain_index = shift;
    my $state_index = shift;
    my $stable_delay_prob = shift;

    print "Set Stable Delay Prob!\n";
    
    my $data = $self->{'data'};

    my $state = $data->{'chains'}->[$chain_index]->{'states'}->[$state_index];

    $state->{'stable_delay_prob'} = $stable_delay_prob;

    return 0;
}

sub handler_set_source
{
    my $self = shift;
    my $chain_index = shift;
    my $source = shift;

    print "Set Source!\n";

    my $data = $self->{'data'};

    my $chain = $data->{'chains'}->[$chain_index];

    $chain->{'source'} = $source;

    return 0;    
}

sub handler_set_dest
{
    my $self = shift;
    my $chain_index = shift;
    my $dest = shift;

    print "Set Dest!\n";

    my $data = $self->{'data'};

    my $chain = $data->{'chains'}->[$chain_index];

    $chain->{'dest'} = $dest;

    return 0;    
}

sub handler_set_protocol
{
    my $self = shift;
    my $chain_index = shift;
    my $index = shift;
    my $enable_or_disable = shift;

    print "Set Protocol [index=$index, enable=$enable_or_disable]!\n";

    my $data = $self->{'data'};

    my $chain = $data->{'chains'}->[$chain_index];

    if ($index > 255)
    {
        $chain->{'protocols'} = ($enable_or_disable ? "\xFF" : "\x00") x 32;
    }
    else
    {
        vec($chain->{'protocols'}, $index, 1) = $enable_or_disable;
    }

    return 0;    
}

sub handler_set_tos
{
    my $self = shift;
    my $chain_index = shift;
    my $index = shift;
    my $enable_or_disable = shift;

    print "Set TOS Bits [index=$index, enable=$enable_or_disable]!\n";

    my $data = $self->{'data'};

    my $chain = $data->{'chains'}->[$chain_index];

    if ($index > 63)
    {
        $chain->{'tos'} = ($enable_or_disable ? "\xFF" : "\x00") x 8;
    }
    else
    {
        vec($chain->{'tos'}, $index, 1) = $enable_or_disable;
    }

    return 0;    
}

sub handler_set_length_min
{
    my $self = shift;
    my $chain_index = shift;
    my $min = shift;
    
    my $data = $self->{'data'};

    my $length = $data->{'chains'}->[$chain_index]->{'length'};

    $length->{'min'} = $min;

    return 0;    
}

sub handler_set_length_max
{
    my $self = shift;
    my $chain_index = shift;
    my $max = shift;
    
    my $data = $self->{'data'};

    my $length = $data->{'chains'}->[$chain_index]->{'length'};

    $length->{'max'} = $max;

    return 0;    
}

sub handler_set_which_length
{
    my $self = shift;
    my $chain_index = shift;
    my $which = shift;

    my $data = $self->{'data'};

    my $length = $data->{'chains'}->[$chain_index]->{'length'};

    $length->{'type'} = $which;

    return 0;
}

sub handler_set_move_probs
{
    my $self = shift;
    my $chain_index = shift;
    my $num_sources = shift;
    my $num_dests = shift;

    my $data = $self->{'data'};

    my $chain = $data->{'chains'}->[$chain_index];

    my @sources;
    my @dests;
    my ($i, $s, $d);

    for($i=0;$i<$num_sources;$i++)
    {
        push @sources, $self->read_param_type("state");
    }

    for($i=0;$i<$num_dests;$i++)
    {
        push @dests, $self->read_param_type("state");        
    }

    # TODO: add sanity check that the some of the dests in any source
    # equals the present sum.

    for($s=0;$s<$num_sources;$s++)
    {
        for($d=0;$d<$num_dests;$d++)
        {
            $chain->{'states'}->[$s]->{'move_tos'}->[$d] = $self->read_param_type("prob");
        }
    }

    return 0;
}

sub read_param_type
{
    my $self = shift;

    my $param_type = shift;

    my $conn = $self->{'conn'};
    
    if ($param_type eq "string")
    {
        my $ret = $conn->conn_read($arb_string_len);

        $ret =~ s/\x00.*$//;

        return $ret;
    }
    elsif ($param_type eq "chain")
    {
        my $which = $conn->conn_read(4);

        $which = unpack("V", $which);

        # TODO : Implement the other <Chain> types.
        if ($which == 2)
        {
            return $self->{'last_chain'};
        }
        else
        {
            die "Unknown which $which!\n";
        }
    }
    elsif ($param_type eq "state")
    {
        my $which = $conn->conn_read(4);

        $which = unpack("V", $which);

        # TODO : Implement the other state types
        if ($which == 0)
        {
            my $index = $conn->conn_read(4);
            
            return unpack("V", $index);
        }
        elsif ($which == 2)
        {
            return $self->{'last_state'};
        }
        else
        {
            die "Uknown state which $which!\n";
        }
    }
    elsif ($param_type eq "prob")
    {
        my $prob = $conn->conn_read(8);

        $prob = unpack("d", $prob);

        if (($prob < 0) || ($prob > 1))
        {
            $prob = 0;
        }
        
        return $prob;
    }
    elsif ($param_type eq "delay_function_type")
    {
        my $delay_type = $conn->conn_read(4);

        $delay_type = unpack("V", $delay_type);

        if ($delay_type == 0)
        {
            return "exponential";
        }
        elsif ($delay_type == 1)
        {
            return "generic";
        }
        else
        {
            return "none";
        }
    }
    elsif ($param_type eq "split_linear_points")
    {
        my ($prob, $delay);

        my (@ret);

        my $do_first = 1;
        while ($do_first || ($prob < 1))
        {
            $do_first = 0;
            # TODO: Sanity check that 
            $prob = unpack("d", $conn->conn_read(8));
            $delay = unpack("V", $conn->conn_read(4));

            if (($prob < 0) || ($prob > 1))
            {
                $prob = 0;
            }
            
            push @ret, {'prob' => $prob, 'delay' => $delay };
        }

        return \@ret;
    }
    elsif ($param_type eq "lambda")
    {
        my $lambda = $conn->conn_read(4);

        $lambda = unpack("V", $lambda);

        return $lambda;      
    }
    elsif ($param_type eq "delay_type")
    {
        my $delay = $conn->conn_read(4);

        $delay = unpack("V", $delay);

        return $delay;
    }
    elsif ($param_type eq "ip_packet_filter")
    {
        my ($ip);
        my @ret;

        # TODO: Add sanity checks.

        $ip = "";
        
        while ($ip ne "\xFF\xFF\xFF\xFF")
        {
            $ip = $conn->conn_read(4);
            my $netmask = unpack("V", $conn->conn_read(4));
            my @ports = ();
            while (1)
            {
                my ($start, $end) = unpack("SS", $conn->conn_read(4));
                if ($start > $end)
                {
                    last;
                }
                push @ports, { 'start' => $start, 'end' => $end };
            }
            push @ret, 
                { 
                    'ip' => $ip, 
                    'ports' => [ @ports ],
                    'netmask_width' => $netmask
                };
        }

        return { 'type' => 'pass', 'filters' => \@ret};
    }
    elsif ($param_type eq "int")
    {
        return unpack("V", $conn->conn_read(4));
    }
    elsif ($param_type eq "bool")
    {
        return (unpack("V", $conn->conn_read(4)) != 0);
    }
    elsif ($param_type eq "which_packet_length")
    {
        my %map = (
            0 => 'all',
            1 => 'gt',
            2 => 'lt',
            3 => 'between',
            4 => 'not-between'
            );
        return $map{unpack("V", $conn->conn_read(4))};
    }
    else
    {
        die "Unknown param type: $param_type!\n";
    }
}

sub write_param_type
{
    my $self = shift;

    my $param_type = shift;

    my $value = shift;

    my $conn = $self->{'conn'};

    if ($param_type eq "int")
    {
        $conn->conn_write(pack("V", $value));
    }
    else
    {
        die "Unkown param_type \"$param_type\"!\n";
    }
}

sub loop
{
    my $self = shift;

    my $flags = $self->{'flags'};

    my $data_lock = $self->{'data_lock'};

    # We keep trying to open a new connection, by this blocking this thread.
    while (1)
    {        
        $self->{'continue'} = 1;
        print "Trying to open a connection!\n";
        $self->{'conn'} = IP::Noise::Conn->new(1);

        # Gain writer permission to the data

        print "IFace : down_write()!\n";
        $data_lock->down_write(); 

        print "IFace : gained down_write()!\n";
        
        my $conn = $self->{'conn'};

        while($self->{'continue'})
        {
            my $opcode = $self->read_opcode(); 

            # Check if an operation for this opcode was defined.
            if (! exists($operations{$opcode}))
            {
                print "Uknown opcode $opcode!\n";
                # If not report it to the user immidiately.
                $conn->conn_write(pack("V", 0x2));
                # Read the next opcode.
                next;
            }
            
            # The record of the operation.
            my $record = $operations{$opcode};

            my @in_param_types = @{$record->{'params'}};
            my @params = ();
            
            # Read the parameters from the line.
            foreach my $param_type (@in_param_types)
            {
                push @params, $self->read_param_type($param_type);
            }
            
            # Call the handler to perform the operation.
            my ($ret_code, @ret) = 
                $record->{'handler'}->(
                    $self,
                    @params
                    );
            
            # Write the return code.
            $conn->conn_write(pack("V", $ret_code));
            
            # Write the output parameters to the line.
            if (exists($record->{'out_params'}))
            {
                my $i;
                my $num_params = scalar(@{$record->{'out_params'}});
                
                for($i=0 ; $i < $num_params ; $i++)
                {
                    $self->write_param_type(
                        $record->{'out_params'}->[$i],
                        $ret[$i]
                        );
                }
            }
        }

        $flags->{'reinit_switcher'} = 1;

        # Release the data for others to use.
        $data_lock->up_write();

        # Destroy the connection.
        
        print "Closing a connection!\n";
        delete($self->{'conn'});
    }
}

1;


