#!/usr/bin/perl

use strict;

my @files = @ARGV;

foreach my $f (@files)
{
    open I, "<".$f;
    open O, ">$f.new";

    while (<I>)
    {
        if (/^#endif/)
        {
            print O "\n";
            print O "#ifdef __cplusplus\n";
            print O "}\n";
            print O "#endif\n";
            print O "\n";
        }
        print O $_;
        chomp($_);
        if (/^#define\s+__\w+_H\s*$/)
        {
            print O "\n";
            print O "#ifdef __cplusplus\n";
            print O "extern \"C\" {\n";
            print O "#endif\n";
            print O "\n";
        }

    }
    close(I);
    close(O);
}

