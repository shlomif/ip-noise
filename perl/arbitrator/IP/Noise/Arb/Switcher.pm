package IP::Noise::Arb::Switcher;

use Heap::Binary;

use IP::Noise::Arb::Switcher::Event;

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
    
    $self->{'pq'} = Heap::Binary->new();

    return 0;
}

sub reinit
{
    my $self = shift;

    $self->{'pq'} = Heap::Binary->new();

    my $data = $self->{'data'};

    my $chains = $data->{'chains'};

    for my $chain_index (0 .. (scalar(@$chains)-1))
    {
        $chains->[$chain_index]->{'current_state'} = 0;

    }
}

sub loop
{
    my $self = shift;

    my $flags = $self->{'flags'};

    my $data_lock = $self->{'data_lock'};
    
    while(1)
    {
        $data_lock->down_read();
        if ($flags->{'reinit_switcher'})
        {
            $self->reinit();
            $flags->{'reinit_switcher'} = 0;
        }

        

        $data_lock->up_read();
   
    }
}

1;
