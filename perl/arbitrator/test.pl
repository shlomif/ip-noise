#!/usr/bin/perl -w

use Data::Dumper;

use IP::Noise::Arb::IFace;

use IP::Noise::Conn;

#my $conn = IP::Noise::Conn->new(1);

my $data = {};

my $arb_iface = IP::Noise::Arb::IFace->new($data);

$arb_iface->loop();

my $d = Data::Dumper->new([ $data ],  [ '$data' ]);

print $d->Dump();

