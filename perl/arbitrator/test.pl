#!/usr/bin/perl -w

use IP::Noise::Arb;

use IP::Noise::Conn;

my $conn = IP::Noise::Conn->new(1);

my $arb = IP::Noise::Arb->new($conn);

$arb->loop();
