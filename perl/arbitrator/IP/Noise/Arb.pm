package IP::Noise::Arb;

use strict;

# Usage: 
#      my $arb = IP::Noise::Arb->new($connection_handle);
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

        # Check if an operation for this opcode was defined.
        if (! exists($operations{$opcode}))
        {
            # If not report it to the user immidiately.
            $conn->conn_write(pack("V", 0x2));
            # Read the next opcode.
            next;
        }
        
        # The record of the operation.
        my $record = $operations{$opcode};

        my @in_param_types = @{$record->{'params'}};
        my @params = ();
        
        # Read the parameters from the line.
        foreach my $param_type (@in_param_types)
        {
            push @params, $self->read_param_type($param_type);
        }
        
        # Call the handler to perform the operation.
        my ($ret_code, @ret) = 
            $record->{'handler'}->(
                $self,
                @params
                );
        
        # Write the return code.
        $conn->conn_write(pack("V", $ret_code));
        
        # Write the output parameters to the line.
        if (exists($record->{'out_params'}))
        {
            my $i;
            my $num_params = scalar(@{$record->{'out_params'}})
            
            for($i=0 ; $i < $num_params ; $i++)
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


