package IP::Noise::Arb::Delayer;

use strict;

use Heap::Binary;
use Time::HiRes qw(gettimeofday);

use IP::Noise::Arb::Msg;

# Our quantom in microseconds
my $quantom = 1000;

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

    # This is a callback to be called for every packet that outght to be 
    # released.
    #
    # Since Perl supports closures it accepts only the packet as a parameter.
    # When doing it in C, this callback should accept a context variable, that
    # will be stored in the delayer.
    my $release_callback = shift;

    $self->{'heap'} = Heap::Binary->new();

    if (ref($release_callback) ne "CODE")
    {
        die "One must pass a reference to a subroutine as the release packet" .
            " callback.\n";
    }

    $self->{'release_callback'} = $release_callback;
    
    return 0;
}

sub delay_packet
{
    my $self = shift;

    my $packet = shift;
    my $sec = shift;
    my $usec = shift;
    my $packet_index = shift;

    my $delay_by = shift;

    # Check if delay_by is only composed of digits
    if ($delay_by !~ /^\d+$/)
    {
        die "\$delay_by must be a positive integer!\n";
    }

    my $delay_to_usec = $usec + $quantom * $delay_by;
    my $delay_to_sec = $sec;
    
    # If delay_to_usec overflows, move the division to sec.
    if ($delay_to_usec > 1000000)
    {
        $delay_to_sec += int($delay_to_usec / 1000000);
        $delay_to_usec %= 1000000;
    }
    
    my $heap_elem = 
        IP::Noise::Arb::Msg->new(
            $packet, 
            $delay_to_sec, 
            $delay_to_usec, 
            $packet_index
            );
    
    $self->{'heap'}->add($heap_elem);

    return 0;
}

sub release_packets_poll_function
{
    my $self = shift;

    my $release_func = $self->{'release_callback'};

    my $heap = $self->{'heap'};

    my ($sec, $usec) = gettimeofday();

    my $current_time_pseudo_msg = 
        { 
            'sec' => $sec, 
            'usec' => $usec , 
            'packet_index' => -1, 
        };

    my $msg;

    # As long as the PQueue is not empty
    while ($msg = $heap->minimum())
    {
        # See if the message should have been sent by now.
        if ($msg->cmp($current_time_pseudo_msg) < 0)
        {
            $release_func->( $msg->get_msg() );
            $heap->extract_minimum();
        }
        else
        {
            last;
        }
    }

    return 0;
}

1;


