#!/usr/bin/perl

# We tried:
# "Hello"
# 0.7
# 1.3

use strict;

use IP::Noise::Text::Stream::In;

use IP::Noise::C::Parser;

open I, "<tests/parse_id.txt";

my $stream = IP::Noise::Text::Stream::In->new(\*I);

while (! eof(I))
{
    eval {
        my $string = IP::Noise::C::Parser::parse_id_string($stream);
    
        print "String = $string\n";
    };

    if ($@)
    {
        my $e = $@;
        print $e->{'text'}, " at line ", 
            $e->{'line_num'}, ": \"", 
            $e->{'context'}, "\"\n";
    }
    $stream->read_next_line();
}
close(I);

