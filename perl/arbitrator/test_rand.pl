#!/usr/bin/perl

use strict;

use IP::Noise::Rand;

my $rand = IP::Noise::Rand->new(24);

my $how_much = shift;

print "A = [";
for my $i (1 .. $how_much)
{
    print $rand->rand(), ", ";
}
print " ] ";



