#!/usr/bin/perl -w

use strict;

use Data::Dumper;
use Thread;
use Thread::Queue;
use Thread::RWLock;

use IPTables::IPv4::IPQueue qw(:constants);
use Time::HiRes qw(usleep gettimeofday);

use IP::Noise::Arb::IFace;
use IP::Noise::Arb::Switcher;
use IP::Noise::Conn;

use IP::Noise::Arb::Delayer;
use IP::Noise::Arb::Packet::Logic;

use vars qw(%flags $data $data_lock $arb_iface $update_states);

#my $conn = IP::Noise::Conn->new(1);

my $terminate = 0;

%flags = (
    'reinit_switcher' => 1,
);

$data = {};

$data_lock = Thread::RWLock->new();

sub thread_func_interface
{
    eval {
        print "In thread!\n";
        $arb_iface = IP::Noise::Arb::IFace->new($data, $data_lock, \%flags);    

        print "In thread 2 ! \n";

        $arb_iface->loop();
    };

    if ($@)
    {
        $terminate = 0;
        my $d = Data::Dumper->new([$@], ["\$error"]);
        print "IFace Died!\n";
        print $d->Dump();
    }
    exit(-1);
}



sub thread_func_update_states
{
    eval {
        $update_states = IP::Noise::Arb::Switcher->new($data, $data_lock, \%flags, \$terminate);

        $update_states->loop();
    };

    if ($@)
    {
        $terminate = 0;
        my $d = Data::Dumper->new([$@], ["\$error"]);
        print "Switcher Died!\n";
        print $d->Dump();
    }
    exit(-1);    
}

my $ip_queue = IPTables::IPv4::IPQueue->new(
    'copy_mode' => IPQ_COPY_PACKET,
    'copy_range' => 2048
    );

if (! $ip_queue)
{
    die IPTables::IPv4::IPQueue->errstr();
}

my $packet_logic = 
    IP::Noise::Arb::Packet::Logic->new($data, $data_lock, \%flags);

my $thread_interface = Thread->new(\&thread_func_interface);

my $thread_update_states = Thread->new(\&thread_func_update_states);

# Thread::Queue is a class for a thread-safe queue.
my $packets_to_arbitrate_queue = Thread::Queue->new();

my $release_callback = 
    sub {
        my $msg = shift;

        #print "Release: Release Msg!\n";
        $ip_queue->set_verdict($msg->packet_id, NF_ACCEPT);
    };

my $delayer = IP::Noise::Arb::Delayer->new($release_callback);

sub release_packets_thread_func
{
    eval {
        while (! $terminate)
        {
            {
                lock($delayer);
                $delayer->release_packets_poll_function();
            }
            usleep(500);
        }
    };

    if ($@)
    {
        $terminate = 0;
        my $d = Data::Dumper->new([$@], ["\$error"]);
        print "Releaser Died!\n";
        print $d->Dump();
    }
    exit(-1);
}

sub decide_what_to_do_with_packets_thread_func
{
    my ($verdict);

    eval {

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
                $ip_queue->set_verdict($msg->packet_id, NF_DROP);
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

    };

    if ($@)
    {
        $terminate = 0;

        my $d = Data::Dumper->new([$@], ["\$error"]);
        print "Arbitrator Died!\n";
        print $d->Dump();
    }
    exit(-1);
}

my $release_packets_thread_handle = Thread->new(\&release_packets_thread_func);
my $decide_what_to_do_with_packets_thread_handle = Thread->new(\&decide_what_to_do_with_packets_thread_func);

my ($last_sec, $last_usec) = (0, 0);
my $packet_index = 0;
while (! $terminate)
{
    my $msg = $ip_queue->get_message();

    print "IPQ: Received a message! (" . 
        $packets_to_arbitrate_queue->pending() . 
        ")\n";
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


