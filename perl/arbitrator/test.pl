#!/usr/bin/perl -w

use strict;

use Data::Dumper;

use IP::Noise::Arb::IFace;
use IP::Noise::Conn;
use Thread::RWLock;

#my $conn = IP::Noise::Conn->new(1);

my $data = {};

my %flags = ();

my $data_lock = Thread::RWLock->new();

my $arb_iface = IP::Noise::Arb::IFace->new($data, $data_lock, \%flags);

$arb_iface->loop();

my $d = Data::Dumper->new([ $data ],  [ '$data' ]);

print $d->Dump();

