# This class serves as a connection to/from the arbitrator
# 
# Or if new() is passed one it can be the a similar connection for
# the arbitrator
package IP::Noise::Conn::Ker;

use strict;
use IO::Handle;

sub O_RDWR () {    02;} 

require 'flush.pl';

#my $pipes_dir = "/home/project" . "/ip-noise/pipes/";
my $dev_path = "/home/project/Docs/Univ/cvs/C/arbitrator/" . "iface_dev";

#local(*OUT,*IN);
local (*DEV);

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

    if (!sysopen(*DEV, $dev_path, O_RDWR))
    {
        die "sysopen $dev_path: $!\n";
    }

    $self->{'out'} = \*DEV;
    $self->{'in'} = \*DEV;
    
    return 0;
}

sub destroy
{
    my $self = shift;

    close(*{$self->{'out'}});
    #close(*{$self->{'in'}});
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
