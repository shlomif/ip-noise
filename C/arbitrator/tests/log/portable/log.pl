#!/usr/bin/perl

use strict;

use POSIX;

my $e_const = exp(1);
my $e_const_reci = (1/$e_const);

my $number = shift || 516;

my $myval = mylog($number);
my $sysval = log($number);
my $percent_diff = (abs($myval-$sysval)/$sysval);
my $log_error = POSIX::log10($percent_diff);
printf("%.80lf\n", $myval);
printf("%.80lf\n", $sysval);
print(int($log_error), "\n");

sub mylog
{
    my $number = shift;
    my $sign_rev = 0;
    if ($number <= 0)
    {
        # Return -google. Close to -inf.
        return -10e100;
    }
    my $exp_base = 0;
    if ($number < 1)
    {
        while ($number < 0.5)
        {
            $number *= $e_const;
            $exp_base--;
        }
    }
    else
    {
        while ($number > 2)
        {
            $number *= $e_const_reci;
            $exp_base++;
        }
    }
    
    if ($number == 1)
    {
        return 0+$exp_base;
    }
    elsif ($number < 1)
    {
        $number = 1/$number;
        $sign_rev = 1;
    }

    my $expr = (($number-1)/($number+1));
    my $expr_squared = $expr*$expr;
    
    my $x_to_the_power_of_n = 2*$expr;

    my $result = 0;

    for(my $a = 1; $a < 29 ; $a++, $a++)
    {
        $result += $x_to_the_power_of_n/$a;
        $x_to_the_power_of_n *= $expr_squared;
    }

    if ($sign_rev)
    {
        $result = -$result;
    }

    $result += $exp_base;

    return $result;   
}
