#!/usr/bin/perl -w

use Data::Dumper;

use IP::Noise::Arb;

use IP::Noise::Conn;

my $conn = IP::Noise::Conn->new(1);

my $arb = IP::Noise::Arb->new($conn);

$arb->loop();

my $d = Data::Dumper->new([ $arb->{'data'} ],  [ '$data' ]);

print $d->Dump();

