package IP::Noise::Arb;

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

    my $conn = shift;

    $self->{'conn'} = $conn;

    return 0;
}

sub read_opcode
{
    my $self = shift;
    
    my $conn = $self->{'conn'};
    
    my $opcode_proto = $conn->conn_read(4);

    return unpack("V", $opcode_proto);
}

my %operations =
(
    0x19 =>
    {
        'params' => [],
        'handler' => \&handler_clear_all,
    }
    
);

sub handler_clear_all
{
    my $self = shift;

    # TODO: Really clear all the chains.

    print "Clear All!\n";

    return 0;
}

sub loop
{
    my $self = shift;

    my $conn = $self->{'conn'};

    while(1)
    {
        my $opcode = $self->read_opcode(); 

        if (! exists($operations{$opcode}))
        {
            $conn->conn_write(pack("V", 0x2));
        }

        my $record = $operations{$opcode};

        my @in_param_types = @{$record->{'params'}};
        my @params = ();

        foreach my $param_type (@in_param_types)
        {
            push @params, $self->read_param_type($param_type);
        }
        my ($ret_code, @ret) = 
            $record->{'handler'}->(
                $self,
                @params
                );

        $conn->conn_write(pack("V", $ret_code));

        if (exists($record->{'out_params'}))
        {
            for(my $i=0;$i<scalar(@{$record->{'out_params'}});$i++)
            {
                $self->write_param_type(
                    $record->{'out_params'}->[$i],
                    $ret[$i]
                    );
            }
        }
    }
}

1;


