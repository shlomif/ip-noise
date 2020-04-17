#!/usr/bin/perl

# We tried:
# "Hello"
# 0.7
# 1.3

use strict;

use IP::Noise::Text::Stream::In;

use IP::Noise::C::Parser;

open I, "<tests/parse_natural.txt";

my $stream = IP::Noise::Text::Stream::In->new(\*I);

while (! eof(I))
{
    eval {
        my $number = IP::Noise::C::Parser::parse_natural_number($stream);
    
        print "Natural = $number\n";
    };

    if ($@)
    {
        my $e = $@;
        print $e->{'text'}, " at line ", $e->{'line_num'}, ": \"", $e->{'context'}, "\"\n";
    }
    $stream->read_next_line();
}
close(I);

