package IP::Noise::Arb::Packet::Logic;

use strict;

use NetPacket::IP;
use NetPacket::TCP;
use NetPacket::UDP;

use Data::Dumper;

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
    my $self = shift;
    
    my $ip_filter = shift;
    
    my $ip = shift;
    
    my $port = shift;

    foreach my $spec (@{$ip_filter->{'filters'}})
    {
        my $netmask_width = $spec->{'netmask_width'};

        if (($spec->{'ip'} >> $netmask_width) == ($ip >> $netmask_width))
        {
            # Do nothing
            if ($port == -1)
            {
                # Do nothing
            }
            else
            {
                foreach my $port_spec ($spec->{'ports'})
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
        }
    }
    return 0;
}

sub chain_decide
{
    my $self = shift;

    my $chain_index = shift;

    my $packet_info = shift;

    my $data = $self->{'data'};

    my $chain = $self->{'data'}->{'chains'}->[$chain_index];

    my $unprocessed_ret = { 'action' => "accept", 'flag' => "unprocessed" };

    if (! is_in_ip_filter(
        $chain->{'source'}, 
        $packet_info->{'source_ip'}, 
        $packet_info->{'source_port'}
        ))
    {
        return $unprocessed_ret ;
    }

    if (! is_in_ip_filter(
        $chain->{'dest'}, 
        $packet_info->{'dest_ip'}, 
        $packet_info->{'dest_port'}
        ))
    {
        return $unprocessed_ret ;
    }

    if (! vec($chain->{'tos'}, $packet_info->{'tos'}, 1))
    {
        return $unprocessed_ret ;
    }

    if (! vec($chain->{'protocols'}, $packet_info->{'protocol'}, 1))
    {
        return $unprocessed_ret ;
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
            return $unprocessed_ret ;
        }        
    }
    elsif ($len_type eq "lt")
    {
        if (! ($p_l <= $length->{'max'}))
        {
            return $unprocessed_ret ;
        }
    }
    elsif ($len_type eq "between")
    {
        if (! (($p_l <= $length->{'max'}) && ($p_l >= $length->{'min'})))
        {
            return $unprocessed_ret ;
        }
    }
    elsif ($len_type eq "not-between")
    {
        if ( (($p_l <= $length->{'max'}) && ($p_l >= $length->{'min'})))
        {
            return $unprocessed_ret ;
        }        
    }

    

    return { 'action' => "accept" };
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
            'flag' => "unprocesed"
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
            $global_verdict = $self->chain_decide(0, $packet_info);
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
        my $payload = $msg->payload();

        my $data_lock = $self->{'data_lock'};

        $data_lock->down_read();

        my $packet_info = &get_packet_info($payload);

        my $verdict = $self->decide($packet_info);

        $data_lock->up_read();        
    
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
