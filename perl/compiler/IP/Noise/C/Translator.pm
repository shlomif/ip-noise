package IP::Noise::C::Translator;

use strict;

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

    my $arbitrator_data_structure = shift;

    $self->{'arb_ds'} = $arbitrator_data_structure;

    # This is the I/O connection to the arbitrator.
    my $conn = shift;

    $self->{'conn'} = $conn;

    return 0;
}

sub load_arbitrator
{
    my $self = shift;


}


1;
