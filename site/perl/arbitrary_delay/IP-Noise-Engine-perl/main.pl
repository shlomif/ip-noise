#!/usr/bin/perl
use strict;

use Packet::Logic;
use Delayer;
use IPTables::IPv4::IPQueue qw(:constants);
use Time::HiRes qw(gettimeofday);

my $packet_logic = Packet::Logic->new();

my $queue = IPTables::IPv4::IPQueue->new(
    'copy_mode' => IPQ_COPY_PACKET,
    'copy_range' => 2048
    );

if (!$queue) 
{
    die IPTables::IPv4::IPQueue->errstr();
}


my $release_callback = 
    sub {
        my $msg = shift;

        $queue->set_verdict($msg->packet_id, NF_ACCEPT);
    };

my $delayer = Delayer->new($release_callback);    

my ($last_sec, $last_usec) = (0, 0);
my $packet_index = 0;
while (1)
{
    my $msg = $queue->get_message();
    my ($sec, $usec) = gettimeofday();

    # We have to do something so it won't overflow...
    $packet_index++;     

    if (! $msg)
    {
        die IPTables::IPv4::IPQueue->errstr();
    }
    
    $delayer->release_packets_poll_function();

    my $verdict = $packet_logic->decide_what_to_do_with_packet($msg);

    if ($verdict->{'action'} eq "accept")
    {
        # Accept immidiately
        $release_callback->($msg);
    }
    elsif ($verdict->{'action'} eq "drop")
    {
        # Drop the packet
        $queue->set_verdict($msg->packet_id, NF_DROP);
    }
    elsif ($verdict->{'action'} eq "delay")
    {
        # Delay the packet for $verdict quanta
        $delayer->delay_packet(
            $msg,
            $sec,
            $usec,
            $packet_index,
            $verdict->{'delay_len'}
            );
    }
    else
    {
        die "Unknown Action!\n";
    }
}


