package IP::Noise::C::Translator;

use strict;

use IP::Noise;

my $arb_string_len = IP::Noise::get_max_id_string_len() + 1;

sub LAST_CHAIN ()
{
    return { 'type' => 'last' };
}

sub LAST_STATE ()
{
    return { 'type' => 'last' };
}

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
    'new_chain' =>
    {
        'opcode' => 0x0,
        'params' => [ "string" ],
        'out_params' => [ "int" ],
    },
    'end_connection' =>
    {
        'opcode' => 0x10000,
        'params' => [],
        'out_params' => [],
    },
    'new_state' =>
    {
        'opcode' => 0x01,
        'params' => [ "chain", "string" ],
        'out_params' => [ "int" ],
    },
    'set_drop_delay_prob' =>
    {
        'opcode' => 0x0E,
        'params' => [ "chain", "state", "prob", "prob" ],
        'out_params' => [],
    },
    'set_delay_type' =>
    {
        'opcode' => 0x0F,
        'params' => [ "chain", "state", "delay_type" ],
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

sub pack_string
{
    my $string = shift;

    return pack(("a" . $arb_string_len), $string);     
}

sub pack_prob
{
    my $prob = shift;
    
    return pack("d", $prob);
}

sub pack_delay_type
{
    my $delay_type = shift;

    if ($delay_type->{'type'} eq "exponential")
    {
        return pack_int32(0);
    }
    elsif ($delay_type->{'type'} eq "generic")
    {
        return pack_int32(1);
    }
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

    foreach my $param_type (@{$record->{'params'}})
    {
        # TODO: Write each parameter according to what was inputted in the 
        # function arguments.
        my $param = shift;

        if ($param_type eq "string")
        {
            $conn->conn_write(pack_string($param));
        }
        elsif ($param_type eq "chain")
        {
            if ($param->{'type'} eq "last")
            {
                $conn->conn_write(pack_int32(2));
            }
            else
            {
                die "Unknown chain type!\n";
            }
        }
        elsif ($param_type eq "state")
        {
            if ($param->{'type'} eq "last")
            {
                $conn->conn_write(pack_int32(2));
            }
            else
            {
                die "Unknown state type!\n";
            }        
        }
        elsif ($param_type eq "prob")
        {
            $conn->conn_write(pack_prob($param));
        }
        elsif ($param_type eq "delay_type")
        {
            $conn->conn_write(pack_delay_type($param));
        }
        else
        {
            die "Unknown param_type $param_type!\n";
        }
    }

    my $ret_value = $self->read_retvalue();

    my (@ret);
    foreach my $param_type (@{$record->{'out_params'}})
    {
        #TODO: read output params from the line and return them.
        if ($param_type eq "int")
        {
            my $int = $conn->conn_read(4);
            push @ret, unpack("V", $int);
        }
        else
        {
            die "Unknown param_type $param_type!\n";
        }
    }

    return ($ret_value, \@ret);
}

my $ws = "    ";

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
        # Create a new chain by that name in the arbitrator
        print "In chain: " , $chain->{'name'}, "\n";

        ($ret_value, $other_args) = 
            $self->transact(
                "new_chain", 
                $chain->{'name'}
                );

        if ($ret_value != 0)
        {
            die "The arbitrator did not give a valid chain index!\n";
        }

        $chain->{'id'} = $other_args->[0];

        print "Chain ID =  ",  $chain->{'id'} , "!\n";

        foreach my $state_name (keys(%{$chain->{'states'}}))
        {
            my $state = $chain->{'states'}->{$state_name};
            
            print "In State " , $state_name, "\n";
            ($ret_value, $other_args) = 
                $self->transact(
                    "new_state",
                    LAST_CHAIN,
                    $state_name
                    );
            
            if ($ret_value != 0)
            {
                die "The arbitrator did not accept the state!\n";
            }
            # Put the index of the state within the arbitrator in a safe
            # place for safekeeping.
            $state->{'id'} = $other_args->[0];

            # Let's set the drop/delay probabilities of the state.

            ($ret_value, $other_args) = 
                $self->transact(
                    "set_drop_delay_prob",
                    LAST_CHAIN,
                    LAST_STATE,
                    $state->{'drop_prob'},
                    $state->{'delay_prob'}
                    );
                
            if ($ret_value != 0)
            {
                die "The arbitrator did not accept the Drop/Delay probabilites!\n";
            }

            if ($state->{'delay_prob'} > 0)
            {
                # Let's set the delay function type of the state
    
                ($ret_value, $other_args) = 
                    $self->transact(
                        "set_delay_type",
                        LAST_CHAIN,
                        LAST_STATE,
                        $state->{'delay_type'}
                        );
            }
       }
    }

    ($ret_value, $other_args) = $self->transact("end_connection");

    return 0;
}


1;
