#!/usr/bin/perl
use strict;

use Thread;
use Thread::Queue;

use IPTables::IPv4::IPQueue qw(:constants);
use Time::HiRes qw(usleep gettimeofday);

use Packet::Logic;
use Delayer;


my $packet_logic = Packet::Logic->new();

my $queue = IPTables::IPv4::IPQueue->new(
    'copy_mode' => IPQ_COPY_PACKET,
    'copy_range' => 2048
    );

# Thread::Queue is a class for a thread-safe queue.
my $packets_to_arbitrate_queue = Thread::Queue->new();

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

my $terminate = 0;

sub release_packets_thread_func
{
    while (! $terminate)
    {
        {
            lock($delayer);
            $delayer->release_packets_poll_function();
        }
        usleep(500);
    }
}

sub decide_what_to_do_with_packets_thread_func
{
    my ($verdict);

    while (! $terminate)
    {
        my $msg_with_time = $packets_to_arbitrate_queue->dequeue();

        my $msg = $msg_with_time->{'msg'};
        
        $verdict = $packet_logic->decide_what_to_do_with_packet($msg);

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
            lock($delayer);
            $delayer->delay_packet(
                $msg,
                $msg_with_time->{'sec'},
                $msg_with_time->{'usec'},
                $msg_with_time->{'index'},
                $verdict->{'delay_len'},
                );
        }
        else
        {
            $terminate = 1;
            die "Unknown Action!\n";
        }
    }
}

my $release_packets_thread_handle = Thread->new(\&release_packets_thread_func);
my $decide_what_to_do_with_packets_thread_handle = Thread->new(\&decide_what_to_do_with_packets_thread_func);

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
        $terminate = 1;
        die IPTables::IPv4::IPQueue->errstr();
    }

    $packets_to_arbitrate_queue->enqueue(
        {
            'msg' => $msg,
            'sec' => $sec,
            'usec' => $usec,
            'index' => $packet_index,
        }
        );
    

}


