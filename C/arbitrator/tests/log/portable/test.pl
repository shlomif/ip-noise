#!/usr/bin/perl

use strict;

my $e_const = exp(1);
my $e_const_reci = (1/$e_const);

my $number = shift || 516;

printf("%.80lf\n", mylog($number));
printf("%.80lf\n", log($number));

sub mylog
{
    my $number = shift;
    if ($number == 0)
    {
        # Return -google. Close to -inf.
        return -10e100;
    }
    my $exp_base = 0;
    while ($number > 2)
    {
        $number *= $e_const_reci;
        $exp_base++;
    }
    $number -= 1;
    my $x_to_the_power_of_n = $number;
    my $result = 0;
    for(my $a = 1; $a < 28 ; $a++)
    {
        if ($a & 0x1)
        {
            $result += $x_to_the_power_of_n/$a;
        }
        else
        {
            $result -= $x_to_the_power_of_n/$a;
        }
        $x_to_the_power_of_n *= $number;
        #printf("%i: %.80lf\n", $a, $result);
    }
    return $result+$exp_base;    
}
