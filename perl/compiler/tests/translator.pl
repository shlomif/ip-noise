#!/usr/bin/perl -w

use strict;

use IP::Noise::Text::Stream::In;
use IP::Noise::C::Parser;

use IP::Noise::C::Translator;
use IP::Noise::Conn;

open I, "<./tests/texts/parse_arbitrator/arbitrator1.txt";
my $stream = IP::Noise::Text::Stream::In->new(\*I);
my $arbitrator_ds = IP::Noise::C::Parser::parse_arbitrator($stream);
close(I);

my $conn = IP::Noise::Conn->new();

my $translator = 
    IP::Noise::C::Translator->new(
        $arbitrator_ds, 
        $conn
        );

$translator->load_arbitrator();

