package IP::Noise::Arb;

use strict;

use IP::Noise;

my $arb_string_len = IP::Noise::get_max_id_string_len() + 1;

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

    $self->{'data'} = {
        'chains' => [],
        'chains_map' => {},
    };

    # This member specifies whether to continue the loop.
    $self->{'continue'} = 1;

    $self->{'last_chain'} = -1;

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
    0x0 =>
    {
        'params' => [ "string" ],
        'out_params' => [ "int" ],
        'handler' => \&handler_new_chain,
    },
    0x1 =>
    {
        'params' => [ "chain", "string"],
        'out_params' => [ "int" ],
        'handler' => \&handler_new_state,
    },
    0x19 =>
    {
        'params' => [],
        'handler' => \&handler_clear_all,
    },
    0xE =>
    {
        'params' => [ "chain", "state", "prob", "prob" ],
        'handler' => \&handler_set_drop_delay_prob
    },
    0x10000 =>
    {
        'params' => [],
        'handler' => \&handler_end_connection,
    },
);

sub handler_clear_all
{
    my $self = shift;

    my $data = $self->{'data'};

    print "Clear All!\n";
    
    # Delete all the chains.
    @{$data->{'chains'}} = ();
    %{$data->{'chains_map'}} = ();

    return 0;
}

sub handler_new_chain
{
    my $self = shift;

    my $data = $self->{'data'};

    # TODO: Make sure we do not allocate two chains by the same name

    print "New Chain!\n";

    my $name = shift;

    push @{$data->{'chains'}}, 
        { 
            'name' => $name, 
            'states' => [], 
            'states_map' => {},
        };
    
    my $index = scalar(@{$data->{'chains'}})-1;

    $data->{'chains_map'}->{lc($name)} = $index;

    $self->{'last_chain'} = $index;

    $self->{'last_state'} = -1;

    return (0, $index);
}

sub handler_end_connection
{
    my $self = shift;

    $self->{'continue'} = 0;
}

sub handler_new_state
{
    my $self = shift;

    my $chain_index = shift;

    my $state_name = shift;

    # TODO: Sanity check that there isn't another state by that name.

    my $data = $self->{'data'};

    my $chain = $data->{'chains'}->[$chain_index];

    my $states_ref = $chain->{'states'};

    push @$states_ref, 
        { 
            'name' => $state_name , 
            'drop_prob' => 0, 
            'delay_prob' => 0 
        };

    my $index = scalar(@$states_ref) - 1;

    $chain->{'states_map'}->{lc($state_name)} = $index;

    $self->{'last_state'} = $index;

    return (0, $index);
}

sub handler_set_drop_delay_prob
{
    my $self = shift;

    my $chain_index = shift;
    my $state_index = shift;
    my $drop_prob = shift;
    my $delay_prob = shift;

    # TODO: Add sanity check that the sum of drop+delay is lesser than 1.

    my $data = $self->{'data'};

    my $state = $data->{'chains'}->[$chain_index]->{'states'}->[$state_index];

    $state->{'drop_prob'} = $drop_prob;
    $state->{'delay_prob'} = $delay_prob;
}

sub read_param_type
{
    my $self = shift;

    my $param_type = shift;

    my $conn = $self->{'conn'};
    
    if ($param_type eq "string")
    {
        my $ret = $conn->conn_read($arb_string_len);

        $ret =~ s/\x00.*$//;

        return $ret;
    }
    elsif ($param_type eq "chain")
    {
        my $which = $conn->conn_read(4);

        $which = unpack("V", $which);

        # TODO : Implement the other <Chain> types.
        if ($which == 2)
        {
            return $self->{'last_chain'};
        }
        else
        {
            die "Unknown which $which!\n";
        }
    }
    elsif ($param_type eq "state")
    {
        my $which = $conn->conn_read(4);

        $which = unpack("V", $which);

        # TODO : Implement the other state types
        if ($which == 2)
        {
            return $self->{'last_state'};
        }
        else
        {
            die "Uknown state which $which!\n";
        }
    }
    elsif ($param_type eq "prob")
    {
        my $prob = $conn->conn_read(8);

        $prob = unpack("d", $prob);

        if (($prob < 0) || ($prob > 1))
        {
            $prob = 0;
        }
        
        return $prob;
    }
    else
    {
        die "Unknown param type: $param_type!\n";
    }
}

sub write_param_type
{
    my $self = shift;

    my $param_type = shift;

    my $value = shift;

    my $conn = $self->{'conn'};

    if ($param_type eq "int")
    {
        $conn->conn_write(pack("V", $value));
    }
    else
    {
        die "Unkown param_type \"$param_type\"!\n";
    }
}

sub loop
{
    my $self = shift;

    my $conn = $self->{'conn'};

    while($self->{'continue'})
    {
        my $opcode = $self->read_opcode(); 

        # Check if an operation for this opcode was defined.
        if (! exists($operations{$opcode}))
        {
            print "Uknown opcode $opcode!\n";
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
            my $num_params = scalar(@{$record->{'out_params'}});
            
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


