# This class serves as a connection to the arbitrator
package IP::Noise::Conn;

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

    my $is_arbitrator = shift || 0;
    
    local(*OUT,*IN);
    
    if ($is_arbitrator)
    {
        open *OUT, ">./pipes/from_arb";
        open *IN, "<./pipe/to_arb";
    }
    else
    {
        open *OUT, ">./pipes/to_arb";
        open *IN, "<./pipes/from_arb";
    }

    $self->{'out'} = \*OUT;
    $self->{'in'} = \*IN;
    
    return 0;
}

sub destroy
{
    my $self = shift;

    close(*{$self->{'out'}});
    close(*{$self->{'in'}});
}

sub DESTROY
{
    my $self = shift;

    $self->destroy();
}

sub conn_write
{
    my $self = shift;

    my $data = shift;

    print *{$self->{'out'}} , $data;
    &flush(*{$self->{'out'}});
    
    return 0;
}

sub conn_read
{
    my $self = shift;

    my $how_much = shift;

    my $data;

    read(*{$self->{'in'}}, $data, $how_much);

    return $data;
}

1;
