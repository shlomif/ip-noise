package IP::Noise::Arb::Switcher;

use strict;

use Heap::Binary;

use IP::Noise::Arb::Switcher::Event;
use IP::Noise::Rand;

use Time::HiRes qw(gettimeofday usleep);

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

    $self->{'data'} = $data;

    $self->{'data_lock'} = $data_lock;

    $self->{'flags'} = $flags;

    $self->{'rand'} = IP::Noise::Rand->new(24);
    
    $self->{'pq'} = Heap::Binary->new();

    return 0;
}

sub delete_chain
{
    my $self = shift;

    my $which_chain = shift;

    my $new_pq = Heap::Binary->new();

    my $old_pq = $self->{'pq'};

    my ($item, $sec, $usec, $chain);
    while (defined($item = $old_pq->extract_minimum() ))
    {
        ($sec, $usec, $chain) = @{$item}{'sec','usec','chain'};
        
        if ($chain == $which_chain)
        {
            # Don't add it to the new PQ           
        }
        else 
        {
            if ($chain > $which_chain)
            {
                $chain--;
            }
            $new_pq->add(
                IP::Noise::Arb::Switcher::Event->new($sec, $usec, $chain)
                );
        }
    }

    $self->{'pq'} = $new_pq;

    return 0;
}

sub reinit
{
    my $self = shift;

    my $pq =  Heap::Binary->new();

    $self->{'pq'} = $pq;

    my $data = $self->{'data'};

    my $chains = $data->{'chains'};

    print "Num chains: " . scalar(@$chains) . "!\n";

    my ($sec, $usec) = gettimeofday();

    for my $chain_index (0 .. (scalar(@$chains)-1))
    {
        print "Inputting $chain_index!\n";
        $chains->[$chain_index]->{'current_state'} = 0;

        $pq->add(
            IP::Noise::Arb::Switcher::Event->new(
                $sec, $usec, $chain_index
                )
            );
        print "Hello $chain_index!\n";
    }
}

sub switch_chain
{
    my $self = shift;

    my $chain_index = shift;

    my $data = $self->{'data'};

    my $chain = $data->{'chains'}->[$chain_index];

    my $current_state = $chain->{'current_state'};

    my $move_tos = $chain->{'states'}->[$current_state]->{'move_tos'};

    my $prob = $self->{'rand'}->rand_in_0_1();

    my $move_tos_com_prob = 0;

    my $i;

    for $i (0 .. (scalar(@$move_tos)-1))
    {
        $move_tos_com_prob += $move_tos->[$i];
        if ($prob < $move_tos_com_prob)
        {
            $chain->{'current_state'} = $i;

            print "Switcher: Switching chain No. $chain_index to State No. $i\n";
            last;
        }
    }

    return 0;
}

my $prob_delta = 0.00000000001;

sub get_new_switch_event
{
    my $self = shift;

    my $chain_index = shift;

    my $data = $self->{'data'};

    my $chain = $data->{'chains'}->[$chain_index];

    my $current_state = $chain->{'current_state'};

    my $time_factor =  $chain->{'states'}->[$current_state]->{'time_factor'};

    my $prob = $self->{'rand'}->rand_in_0_1();

    # this is too small and will lead to an infinite delay
    if ($prob < $prob_delta)
    {
        $prob = $prob_delta;
    }
    
    # Get the length of the delay
    my $length = int((-log($prob))*$time_factor);
    
    my ($sec, $usec) = gettimeofday();

    $usec += $length*1000;

    if ($usec > 1000000)
    {
        $sec += int($usec / 1000000);
        $usec %= 1000000;
    }

    return IP::Noise::Arb::Switcher::Event->new(
        $sec, $usec, $chain_index
        );
}

sub loop
{
    my $self = shift;

    my $flags = $self->{'flags'};

    my $data_lock = $self->{'data_lock'};

    my $data = $self->{'data'};

    my $chains = $data->{'chains'};

    while(1)
    {
        #Check if the whole db is valid
        $data_lock->down_read();
        if ($flags->{'reinit_switcher'})
        {
            print "Arb::Switcher : reinit()!\n";
            $self->reinit();
            $flags->{'reinit_switcher'} = 0;
        }

        my ($sec, $usec) = gettimeofday();

        my $current_time_pseudo_event = 
            {
                'sec' => $sec,
                'usec' => $usec,
            };
        
        my ($event, $chain_index);
        
        # Let's try to see if there are any events whose time has come.

        while ($event = $self->{'pq'}->minimum())
        {
            if (($event->cmp($current_time_pseudo_event)) < 0)
            {
                $self->{'pq'}->extract_minimum();
                                
                $chain_index = $event->{'chain'};

                $self->switch_chain($chain_index);
                
                $self->{'pq'}->add(
                    $self->get_new_switch_event($chain_index)
                    );
            }
            else
            {
                last;
            }            
        }


        $data_lock->up_read();

        # Sleep for 50 milliseconds.
        usleep(50000);
    }
}

1;
