# This class serves as a connection to the arbitrator
package IP::Noise::Conn;

use strict;

require 'flush.pl';

#my $pipes_dir = $ENV{'HOME'} . "/ip-noise/pipes/";
my $pipes_dir = "/home/project" . "/ip-noise/pipes/";

local(*OUT,*IN);

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
    
    if ($is_arbitrator)
    {
        open *OUT, ">$pipes_dir/from_arb";
        open *IN, "<$pipes_dir/to_arb";
    }
    else
    {        
        open *IN, "<$pipes_dir/from_arb";
        open *OUT, ">$pipes_dir/to_arb";
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

    # This is a kludge, but it's here to make sure we do not re-open
    # the already open connection immidiately
    sleep(1);
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

    print {*{$self->{'out'}}} $data;
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
