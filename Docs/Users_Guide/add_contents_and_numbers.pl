#!/usr/bin/perl -w

use strict;

my $line;

my ($out_text_start, $out_text_end);

my $tree = { 'text' => '', 'subs' => []};

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

        my $depth = $num - 2;
        $current_coords[$depth]++;
        @current_coords = @current_coords[0 .. $depth];
        $line = "<a name=\"" . join(".", (map { $_ } @current_coords)) . "\"></a>\n";
        $line .= "<h$num>" . join(".", (map { $_ } @current_coords)) . ". $text</h$num>\n";
        my $branch = $tree;
        foreach my $c (@current_coords[0 .. $#current_coords-1])
        {
           $branch = $branch->{'subs'}->[$c]; 
        }
        $branch->{'subs'}->[$current_coords[$#current_coords]] = { 'text' => $text, 'subs' => [] } ;
    }
    $out_text_end .= $line;
}

sub render_tree
{
    my $prev_coords = shift;
    my $branch = shift;

    my $ret;

    if (scalar(@$prev_coords) == 0)
    {
        $ret .= "<h2>Table Of Contents</h2>\n";
    }
    else
    {
    
        $ret .= "<a href=\"#" . join(".", @$prev_coords) . "\">" .
            join(".", @$prev_coords) . ". " . $branch->{'text'} . "</a><br>\n";
    }

    my $num_sub_branches = scalar(@{$branch->{'subs'}});
    if ($num_sub_branches)
    {
        my $i;
        $ret .= "<ul>\n";
        for($i=1;$i<$num_sub_branches;$i++)
        {
            $ret .= render_tree([ @$prev_coords, $i ], $branch->{'subs'}->[$i]);
        }
        $ret .= "</ul>\n";
    }

    return $ret;
}

print $out_text_start, render_tree([], $tree) , $out_text_end;

