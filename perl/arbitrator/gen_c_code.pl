#!/usr/bin/perl

use strict;

use IP::Noise::Arb::IFace;

my %operations = %IP::Noise::Arb::IFace::operations;

open I, "<IP/Noise/Arb/IFace.pm";

while (<I>)
{
    if (/\%operations/)
    {
        last;
    }
}

my $functions = "";

my $opcode;
my $func_name;


my %param_types =
(
    "string" => { qw(pt STRING c), "char *", qw(u string) },
    "int" => { qw(pt INT c int u _int) },
    "chain" => { qw(pt CHAIN c int u chain) },
    "ip_packet_filter" => { qw(pt IP_FILTER u ip_filter c), "ip_noise_ip_spec_t *" },
    "bool" => {qw(pt BOOL u bool c int) },
    "which_packet_length" => { qw(pt WHICH_PACKET_LENGTH u which_packet_length c int) },
    "state" => { qw(pt STATE u state c int) },
    "prob" => { qw(pt PROB u prob c ip_noise_prob_t) },
    "delay_function_type" => { qw(pt DELAY_FUNCTION_TYPE u delay_function_type c int) },
    "split_linear_points" => { qw(pt SPLIT_LINEAR_POINTS u split_linear_points c ip_noise_split_linear_function_t) },
    "lambda" => { qw(pt LAMBDA u lambda c int) },
    "delay_type" => { qw(pt DELAY_TYPE u delay_type c int) },
);

sub get_c_param_type
{
    my $t = shift;
    
    if (!exists($param_types{$t}))
    {
        die "Unknown param type $t!\n";
    }
    return "PARAM_TYPE_" . $param_types{$t}->{'pt'};
}

sub get_c_type
{
    my $t = shift;
    
    if (!exists($param_types{$t}))
    {
        die "Unknown param type $t!\n";
    }
    return $param_types{$t}->{'c'};    
}

sub get_c_union_member
{
    my $t = shift;
    
    if (!exists($param_types{$t}))
    {
        die "Unknown param type $t!\n";
    }
    return $param_types{$t}->{'u'};    
}


while (<I>)
{
    if (/0x/)
    {
        $opcode = $_;
        chomp($opcode);
        $opcode =~ s/^[^0]*//;
        $opcode =~ s/[^0-9A-Fa-f]*$//;
        $opcode = hex($opcode);
    }
    if (/'handler'/)
    {
        /\\\&handler_(\w+)/;
        $func_name = $1;

        $operations{$opcode}->{'func_name'} = $func_name;
    }
    if (/;/)
    {
        last;
    }
}

close(I);

open O, ">" . $ENV{'HOME'} . "/Docs/Univ/cvs/C/arbitrator/iface_handlers.c";

foreach $opcode (sort { $a <=> $b } keys(%operations))
{
    print O "static int ip_noise_arbitrator_iface_handler_" . $operations{$opcode}->{'func_name'} . "(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);\n";
}

print O "\n\n\n";

print O "#define NUM_OPERATIONS " . scalar(keys(%operations)) . "\n";
print O "operation_t operations[NUM_OPERATIONS] = \n";
print O "{\n";

foreach $opcode (sort { $a <=> $b } keys(%operations))
{
    my @params = @ { $operations{$opcode}->{'params'} };
    my @out_params;
    if (exists($operations{$opcode}->{'out_params'}))
    {
        @out_params = @ { $operations{$opcode}->{'out_params'} };
    }
    else
    {
        @out_params = ();
    }
    print O "\t{\n";
    print O "\t\t" . sprintf("0x%X", $opcode) . ",\n";
    foreach my $pars (\@params, \@out_params)
    {
        print O "\t\t" . scalar(@$pars) . ",\n";
        print O "\t\t{" . join(", ", ((map { get_c_param_type($_); } @$pars), ("PARAM_TYPE_NONE") x (4 - scalar(@$pars)))) . "},\n";
    }
    my $func_name = $operations{$opcode}->{'func_name'};
    print O "\t\tip_noise_arbitrator_iface_handler_" . $operations{$opcode}->{'func_name'} . "\n";

    print O "\t},\n";

    $functions .= "static int ip_noise_arbitrator_iface_handler_" . $operations{$opcode}->{'func_name'} . "(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)\n";

    $functions .= "{\n";

    my @param_names = ();

    open I, "<IP/Noise/Arb/IFace.pm";
    while(<I>)
    {
        if (/^\s*sub handler_$func_name/)
        {
            last;
        }
    }
    while(<I>)
    {
        if (/^\s*my\s+\$(\w+)\s*=\s*shift\s*;/)
        {
            if ($1 ne "self")
            {
                push @param_names, $1;
                if (scalar(@param_names) == scalar(@params))
                {
                    last;
                }
            }
        }        
    }
    close(I);    
    for(my $i=0;$i<scalar(@params);$i++)
    {
        $functions .= 
            "\t" . &get_c_type($params[$i]) . 
            " "  . $param_names[$i] . " = " . 
            "params[$i]." . &get_c_union_member($params[$i]) . ";\n";
    }
    
    $functions .= "\n\n\n\t/* FILL IN WITH BODY*/\n\n\n";

    $functions .= "\treturn 0;\n";

    $functions .= "}\n\n\n\n";
}

print O "};\n";


print O $functions;
close(O);
