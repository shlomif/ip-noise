#
# This is a random number generator
#

package IP::Noise::Rand;

use strict;

use integer;

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

    my $seed = int(shift);

    $self->{'hold_rand'} = $seed;
}

sub srand
{
    my $self = shift;

    my $seed = int(shift);
    
    $self->{'hold_rand'} = $seed;

    return
}

sub rand_15
{
    my $self = shift;

    $self->{'hold_rand'} = $self->{'hold_rand'} * 214013 + 2531011;

    return (($self->{'hold_rand'} >> 16) & 0x7FFF);    
}

sub rand
{
    my $self = shift;
    
    my $one = $self->rand_15();
    my $two = $self->rand_15();
    
    return $one | ($two << 15);
}

1;


