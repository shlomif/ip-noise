#!/usr/bin/perl -w

use Data::Dumper;

use IP::Noise::Arb::IFace;

use IP::Noise::Arb::Switcher;

use IP::Noise::Conn;

use Thread;

use Thread::RWLock;

use vars qw(%flags $data $data_lock $arb_iface $update_states);

#my $conn = IP::Noise::Conn->new(1);

%flags = (
    'reinit_switcher' => 1,
);

$data = {};

$data_lock = Thread::RWLock->new();

sub thread_func_interface
{
    print "In thread!\n";
    $arb_iface = IP::Noise::Arb::IFace->new($data, $data_lock, \%flags);    

    print "In thread 2 ! \n";

    $arb_iface->loop();
}

my $thread_interface = Thread->new(\&thread_func_interface);

sub thread_func_update_states
{
    $update_states = IP::Noise::Arb::Switcher->new($data, $data_lock, \%flags);

    $update_states->loop();
}

my $thread_update_states = Thread->new(\&thread_func_update_states);


#my $d = Data::Dumper->new([ $data ],  [ '$data' ]);

#print $d->Dump();

