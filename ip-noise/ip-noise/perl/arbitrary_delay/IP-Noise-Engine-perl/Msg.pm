package Msg;

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

    @{$self}{'msg', 'sec', 'usec', 'index'} = @_;

    $self->{'heap_val'} = 0;

    return 0;
}

sub cmp
{
    my $self = shift;
    my $other = shift;

    return (($self->{'sec'} <=> $other->{'sec'}) ||
            ($self->{'usec'} <=> $other->{'usec'}) ||
            ($self->{'index'} <=> $other->{'index'}) );
}

sub get_msg
{
    my $self = shift;

    return $self->{'msg'};
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
