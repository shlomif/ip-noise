package IP::Noise::Arb::Switcher::Event;

use strict;

use vars qw(@ISA);

use Heap::Elem;

@ISA = qw(Heap::Elem);

sub new
{
    my $self = shift;
    my $class = ref($self) || $self;

    #$self = SUPER::new($class);
    $self = {};
    bless $self, $class;

    $self->initialize(@_);

    return $self;
}

sub initialize
{
    my $self = shift;

    @{$self}{'sec', 'usec', 'chain'} = @_;

    $self->{'heap_val'} = 0;

    return 0;
}

sub cmp
{
    my $self = shift;
    my $other = shift;

    return (($self->{'sec'} <=> $other->{'sec'}) ||
            ($self->{'usec'} <=> $other->{'usec'}));

}

sub get_chain
{
    my $self = shift;

    return $self->{'chain'};
}

sub heap
{
    my $self = shift;

    my $prev_value = $self->{'heap_val'};
    if (scalar(@_))
    {        
        $self->{'heap_val'} = shift;
    }

    return $prev_value;
}

1;
