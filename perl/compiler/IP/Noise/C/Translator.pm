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

my %transactions =
(
    'clear_all' =>
    {
        'opcode' => 0x19,
        'params' => [],
        'out_params' => [],
    },
);

sub pack_int32
{
    my $value = shift;

    return pack("V", $value);
}

sub pack_opcode
{
    my $opcode = shift;

    return pack_int32($opcode);
}


sub read_retvalue
{
    my $self = shift;

    my $conn = $self->{'conn'};

    my $value = $conn->conn_read(4);

    return unpack("V", $value);
}


sub transact
{
    my $self = shift;

    my $name = shift;

    if (!exists($transactions{$name}))
    {
        die "No such transaction";
    }

    my $record = $transactions{$name};

    my $conn = $self->{'conn'};

    $conn->conn_write(pack_opcode($record->{'opcode'}));

    foreach my $param (@{$record->{'params'}})
    {
        # TODO: Write each parameter according to what was inputted in the 
        # function arguments.
    }

    my $ret_value = $self->read_retvalue();

    my (@ret);
    foreach my $param (@{$record->{'out_params'}})
    {
        #TODO: read output params from the line and return them.
    }

    return ($ret_value, \@ret);
}


sub load_arbitrator
{
    my $self = shift;

    my $ds = $self->{'arb_ds'};

    my ($ret_value, $other_args) = $self->transact("clear_all");

    if ($ret_value != 0)
    {
        die "The arbitrator failed or refused to clear all the chains!\n";
    }

    foreach my $chain (values(%{$ds->{'chains'}}))
    {
        print $chain->{'name'}, "\n";
    }
}


1;
