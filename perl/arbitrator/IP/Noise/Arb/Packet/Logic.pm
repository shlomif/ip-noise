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

    return join("", (map { chr($_) } split(/\./, $dot_quad)));
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
    $ret{'length'} = $ip->{'length'};

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

        my $d = Data::Dumper->new([ $packet_info ], [ "\$packet_info" ] );
        print $d->Dump();

        $data_lock->up_read();        
    
        return { 'action' => "accept" };
    }
    else
    {
        # What should I do with a packet with a data length of 0.
        # I know - accept it.
        return { 'action' => "accept" };
    }
}
1;
