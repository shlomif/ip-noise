#!/usr/bin/perl -w

use strict;

use Heap::Binary;

use Msg;

my @my_messages = 
    (
        [ qw(message1 900 500)],
        [ qw(message2 500 300)],
        [ qw(message3 900 50)],
        [ qw(message4 1000 6000)],
        [ qw(message5 200 30)],
        [ qw(message6 1000 100)],
        [ qw(message7 20 100)],
    );

my @ops = qw(a a a a a a e e a e e e e e);

my $pointer = 0;

my $heap = Heap::Binary->new();

foreach my $operation (@ops)
{
    if ($operation eq "a")
    {
       my $elem = Msg->new( @{$my_messages[$pointer++]} ); 
       $heap->add($elem);
    }
    elsif ($operation eq "e")
    {
        my $elem = $heap->minimum();
        print ($elem->get_msg(), "\n");
        $heap->extract_minimum();
    }
    else
    {
        die "Unknown operation $operation!\n";
    }
}


