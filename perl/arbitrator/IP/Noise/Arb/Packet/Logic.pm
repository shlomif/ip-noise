package IP::Noise::Arb::Packet::Logic;

use strict;

use NetPacket::IP;
use NetPacket::TCP;
use NetPacket::UDP;

use Data::Dumper;

use Time::HiRes qw(gettimeofday);

use IP::Noise::Rand;

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

    $self->{'rand'} = IP::Noise::Rand->new(5);

    return 0;
}

sub from_dotquad
{
    my $dot_quad = shift;

    return unpack("N", join("", (map { chr($_) } split(/\./, $dot_quad))));
}

sub get_packet_info
{
    my $payload = shift;

    my $ip = NetPacket::IP->decode($payload);

    my %ret = ();

    $ret{'source_ip'} = &from_dotquad($ip->{'src_ip'});
    $ret{'dest_ip'} = &from_dotquad($ip->{'dest_ip'});
    $ret{'tos'} = $ip->{'tos'};
    $ret{'protocol'} = $ip->{'proto'};
    $ret{'length'} = $ip->{'len'};

    if ($ret{'protocol'} == 6)
    {
        my $tcp = NetPacket::TCP->decode(NetPacket::IP::ip_strip($payload));

        $ret{'source_port'} = $tcp->{'src_port'};
        $ret{'dest_port'} = $tcp->{'dest_port'};
    }
    elsif ($ret{'protocol'} == 17)
    {
        my $udp = NetPacket::UDP->decode(NetPacket::IP::ip_strip($payload));

        $ret{'source_port'} = $udp->{'src_port'};
        $ret{'dest_port'} = $udp->{'dest_port'};
    }
    else
    {
        $ret{'source_port'} = -1;
        $ret{'dest_port'} = -1;
    }

    return \%ret;
}

sub is_in_ip_filter
{
    my $ip_filter = shift;
    
    my $ip = shift;
    
    my $port = shift;

    if ($ip_filter->{'type'} eq "none")
    {
        return 1;
    }

    foreach my $spec (@{$ip_filter->{'filters'}})
    {
        my $netmask_width = $spec->{'netmask_width'};

        if (($spec->{'ip'} >> $netmask_width) == ($ip >> $netmask_width))
        {
            if ($port == -1)
            {
                # This protocol does not have a port, so we don't care about
                # the port specification.
                return 1;
            }
            else
            {
                foreach my $port_spec (@{$spec->{'ports'}})
                {
                    if (($port_spec->{'start'} <= $port) &&
                       ($port <= $port_spec->{'end'}))
                    {
                        return 1;
                    }                   
                }
            }
            
        }
        else
        {
            # Continue to the next filter
        }
    }
    return 0;
}


sub myBsearch
{
	my $key = shift;
	my $array = shift;
	my $len = shift;
	my $compare = shift;
	my $context = shift;

	my $low = 0;
	my $high = $len-1;
	my $mid;
	my $result;

	while($low <= $high)
	{
		$mid = int(($low+$high)/2);

		$result = &{$compare}($context, $key, $array->[$mid]);

		if ($result < 0)
		{
			$high = $mid-1;
		}
		elsif ($result > 0)
		{
			$low = $mid+1;
		}
		else
		{
			return ($mid,1);
		}
	}

	$result = &{$compare}($context, $key, $array->[$high]);

	return ($high+1, 0);
}


my $prob_delta = 0.00000000001;

sub is_in_chain_filter
{
    my $self = shift;

    my $chain_index = shift;

    my $packet_info = shift;

    my $data = $self->{'data'};

    my $chain = $data->{'chains'}->[$chain_index];    

    if (! is_in_ip_filter(
        $chain->{'source'}, 
        $packet_info->{'source_ip'}, 
        $packet_info->{'source_port'}
        ))
    {
        return 0 ;
    }

    if (! is_in_ip_filter(
        $chain->{'dest'}, 
        $packet_info->{'dest_ip'}, 
        $packet_info->{'dest_port'}
        ))
    {
        return 0 ;
    }

    if (! vec($chain->{'tos'}, $packet_info->{'tos'}, 1))
    {
        return 0 ;
    }

    if (! vec($chain->{'protocols'}, $packet_info->{'protocol'}, 1))
    {
        return 0 ;
    }

    my $length = $chain->{'length'};
    my $len_type = $length->{'type'};
    my $p_l = $packet_info->{'length'};

    if ($len_type eq "all")
    {
        # Do nothing
    }
    elsif ($len_type eq "gt")
    {
        if (! ($p_l >= $length->{'min'}))
        {
            return 0 ;
        }        
    }
    elsif ($len_type eq "lt")
    {
        if (! ($p_l <= $length->{'max'}))
        {
            return 0 ;
        }
    }
    elsif ($len_type eq "between")
    {
        if (! (($p_l <= $length->{'max'}) && ($p_l >= $length->{'min'})))
        {
            return 0 ;
        }
    }
    elsif ($len_type eq "not-between")
    {
        if ( (($p_l <= $length->{'max'}) && ($p_l >= $length->{'min'})))
        {
            return 0 ;
        }        
    }

    return 1;
}

sub chain_decide
{
    my $self = shift;

    my $chain_index = shift;

    my $packet_info = shift;

    my $ignore_filter = shift || 0;

    my $unprocessed_ret = { 'action' => "accept", 'flag' => "unprocessed" };

    if (! $ignore_filter)
    {
        if (! $self->is_in_chain_filter($chain_index, $packet_info))
        {
            return $unprocessed_ret;
        }
    }

    my $chain = $self->{'data'}->{'chains'}->[$chain_index];

    my $current_state = $chain->{'states'}->[$chain->{'current_state'}];

    my $which_prob = $self->{'rand'}->rand_in_0_1();
    if ($which_prob < $current_state->{'drop_prob'})
    {
        return { 'action' => "drop" };
    }
    elsif ($which_prob <= $current_state->{'drop_prob'} + $current_state->{'delay_prob'})
    {
        # Delay

        my $delay;

        if ($current_state->{'delay_type'}->{'type'} eq "exponential")
        {
            my $prob = $self->{'rand'}->rand_in_0_1();

            if ($prob < $prob_delta)
            {
                $prob = $prob_delta;
            }

            my $lambda = $current_state->{'delay_type'}->{'lambda'};
            $delay = int((-log($prob)) * $lambda);            
        }
        elsif ($current_state->{'delay_type'}->{'type'} eq "generic")
        {
            my $prob = $self->{'rand'}->rand_in_0_1();

            my $points = $current_state->{'delay_type'}->{'points'};

            my $compare_func = sub {
                my $context = shift;
                my $prob = shift;
                my $item = shift;

                return ($prob <=> $item->{'prob'});
            };

            my ($index, $is_precise) = 
                &myBsearch( 
                    $prob,
                    $points,
                    scalar(@$points),
                    $compare_func,
                    0
                    );
            

            if ($is_precise == 1)
            {
                $delay = $points->[$index]->{'delay'};                
            }
            else
            {
                # This is the formula for linear interpolation.
                my ($x1, $y1) = @{$points->[$index]}{'prob','delay'};
                my ($x2, $y2) = @{$points->[$index+1]}{'prob','delay'};
                
                $delay = int((($prob-$x1)*$y1+($x2-$prob)*$y2)/($x2-$x1));
            }

            #print "Generic Delay = $delay\n";
        }

        my $do_a_stable_delay_prob = $self->{'rand'}->rand_in_0_1();

        my ($sec, $usec) = &gettimeofday();

        if (! exists($chain->{'last_packet_release_time'}))
        {
            $chain->{'last_packet_release_time'} = 
                {
                    'sec' => $sec,
                    'usec' => $usec,
                };                  
        }

        if ($do_a_stable_delay_prob < $current_state->{'stable_delay_prob'})
        {
            # $last_sec and $last_usec are the times in which the current
            # packet will be released. It is calculated by taking the release
            # time of the last packet and adding the delay.
            my ($last_sec, $last_usec) = @{$chain->{'last_packet_release_time'}}{'sec','usec'};
            #print "P::L : \$last_sec = $last_sec ; \$last_usec = $last_usec ; \$sec = $sec ; \$usec = $usec\n";

            $last_usec += $delay * 1000;
            if ($last_usec > 1000000)
            {
                $last_sec += int($last_usec/1000000);
                $last_usec %= 1000000;
            }
            # Calculate a modified delay
            $delay = ($last_sec-$sec)*1000+int(($last_usec-$usec)/1000);
            
            # If the last packet was already sent, we should send this one,
            # now.
            if ($delay < 0)
            {
                $delay = 0;
            }
        }

        $usec += $delay * 1000;
        if ($usec > 1000000)
        {
            $sec += int($usec/1000000);
            $usec %= 1000000;
        }
        $chain->{'last_packet_release_time'} = 
            { 
                'sec' => $sec, 
                'usec' => $usec 
            };
        
        return { 'action' => "delay", 'delay_len' => $delay };
    }
    else
    {
        return { 'action' => "accept" , 'flag' => "processed" };
    }
}

sub decide
{
    my $self = shift;

    my $packet_info = shift;

    my $data = $self->{'data'};

    my $chains = $data->{'chains'};

    my $global_verdict = 
        { 
            'action' => 'accept' , 
            'delay_len' => 0,
            'flag' => "unprocessed"
        };

    for my $chain_index (1 .. (scalar(@$chains)-1))
    {
        my $chain_verdict = $self->chain_decide($chain_index, $packet_info);

        if ($chain_verdict->{'action'} eq "drop")
        {
            $global_verdict->{'action'} = "drop";
            $global_verdict->{'flag'} = "processed";
        }
        elsif ($chain_verdict->{'action'} eq "delay")
        {
            if ($global_verdict->{'action'} eq "drop")
            {
                # Do nothing
            }
            elsif ($global_verdict->{'action'} eq "delay")
            {
                $global_verdict->{'delay_len'} += $chain_verdict->{'delay_len'};
            }
            elsif ($global_verdict->{'action'} eq "accept")
            {
                $global_verdict = $chain_verdict;
                $global_verdict->{'flag'} = "processed";
            }
            else
            {
                print "Unknown \$global_verdict->{'action'} - " . $global_verdict->{'action'} . "!\n";
                die "Unknown \$global_verdict->{'action'} - " . $global_verdict->{'action'} . "!\n";
            }
        }
        elsif ($chain_verdict->{'action'} eq "accept")
        {
            # Do nothing - this is neutral.
            if ($chain_verdict->{'flag'} ne "unprocessed")
            {
                $global_verdict->{'flag'} = "processed";
            }
        }
        else
        {
            print "Unknown \$chain_verdict->{'action'} - " . $chain_verdict->{'action'} . "!\n";
            die "Unknown \$chain_verdict->{'action'} - " . $chain_verdict->{'action'} . "!\n";        
        }
    }

    if ($global_verdict->{'flag'} eq "unprocessed")
    {
        if (scalar(@$chains) > 0)
        {
            $global_verdict = $self->chain_decide(0, $packet_info, 1);
        }
    }

    # TODO: If the packet was processed by one or more other chains,
    # should it still pass through the default chain?

    if (($global_verdict->{'action'} eq "delay") &&
        ($global_verdict->{'delay_len'} == 0)
       )
    {
        return { 'action' => "accept" };
    }
       
    return $global_verdict;
}

sub decide_what_to_do_with_packet
{
    my $self = shift;

    my $msg = shift;

    if ($msg->data_len())
    {
        my $verdict;

        eval {
            my $payload = $msg->payload();

            my $data_lock = $self->{'data_lock'};

            $data_lock->down_read();

            my $packet_info = &get_packet_info($payload);

            $verdict = $self->decide($packet_info);

            #my $d = Data::Dumper->new([ $verdict, $packet_info], [ "\$verdict", "\$packet_info"]);
            #print $d->Dump();
       

            $data_lock->up_read();        

        };

        if ($@)
        {
            print $@, "\n";
        }
    
        return $verdict;
    }
    else
    {
        # What should I do with a packet with a data length of 0.
        # I know - accept it.
        return { 'action' => "accept" };
    }
}
1;
