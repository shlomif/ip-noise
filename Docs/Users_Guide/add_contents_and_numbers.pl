#!/usr/bin/perl -w

use strict;

my $line;

my $out_text_start, $out_text_end;

my $tree = [];

my @current_coords = (0);

open I, "<main.html";
while ($line = <I>)
{
    if ($line =~ /<!-- <<< Insert Contents Here >>> -->/)
    {
        last;
    }
    $out_text_start .= $line;
}

while ($line = <I>)
{
    if ($line =~ /<h([2-6])>/)
    {
        my $num = $1;
        $line =~ />(.*)</;
        my $text = $1;

        $num -= 2;
        $current_coords[$num]++;
        @current_coords = @current_coords[0 .. $num];
        $line = "<h$num>" . join(".", (map { $_+1 } @current_coords)) . "</h$num>\n";
        my $branch = $tree;
        foreach my $c (@current_coords[0 .. $#current_coords-1])
        {
           $branch = $branch->[$c]; 
        }
        $branch->[$current_coords[$#current_coords]] = $text;
    }
    $out_text_end .= $line;
}

print $out_text_state, "\n", $out_text_end;
