#!/usr/bin/perl

use strict;

use IP::Noise::Text::Stream::In;

use IP::Noise::C::Parser;

use Data::Dumper;

my @files;
while (my $filename = <./tests/texts/parse_chain/*>)
{
    push @files, $filename;
}

@files = (grep { ! /\~$/ } @files);

foreach my $filename (@files)
{
    open I, "<$filename";

    print "\n\n$filename:\n";

    my $stream = IP::Noise::Text::Stream::In->new(\*I);

    eval {
        my $chain = IP::Noise::C::Parser::parse_chain($stream);
    
        my $d = Data::Dumper->new([$chain], ["Chain"]);
        print $d->Dump();
    };

    if ($@)
    {
        my $e = $@;
        
        print $e->{'text'}, " at line ", 
            $e->{'line_num'}, ": \"", 
            $e->{'context'}, "\"\n";
    }

    close(I);
}
