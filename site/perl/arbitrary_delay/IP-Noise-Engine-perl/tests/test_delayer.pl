#!/usr/bin/perl -w
use strict;

use Delayer;

sub print_message
{
    my $message = shift;

    print "$message\n";

    return 0;
}

my @messages = ('msg1', 1000, 'msg2', 50, 'msg3', 900, 'msg4', 4000, 'msg5', 30);
my $d = Delayer->new(\&print_message);

while (scalar(@messages))
{
    my $msg = shift(@messages);
    my $delay = shift(@messages);
    $d->delay_packet($msg, $delay);
}

while (1)
{
    $d->release_packets_poll_function();
}
