#!/usr/bin/perl

use strict;

use IP::Noise::Text::Stream::In;

use IP::Noise::C::Parser;

use Data::Dumper;

my @files;
while (my $filename = <./tests/texts/parse_arbitrator/*>)
{
    push @files, $filename;
}

@files = (grep { ! /\~$/ } @files);
@files = (grep { ! /CVS/ } @files);

foreach my $filename (@files)
{
    open I, "<$filename";

    print "\n\n$filename:\n";

    my $stream = IP::Noise::Text::Stream::In->new(\*I);

    eval {
        my $arbitrator = IP::Noise::C::Parser::parse_arbitrator($stream);
    
        my $d = Data::Dumper->new([$arbitrator], ["Chain"]);
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
