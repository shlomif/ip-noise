#!/usr/bin/perl -w

use strict;

open I, "<Protocol.txt";
my $text = join("", <I>);
close(I);

my (@portions, @portions_filtered);
@portions = ($text =~ /<.*?>/g);
#@portions = (map { lc($_) } @portions);
@portions = (grep { ! /^<0x/ } @portions);

my %hash;
@hash{@portions} = @portions;

print join("\n", (sort {lc($a) cmp lc($b)} keys(%hash)));
