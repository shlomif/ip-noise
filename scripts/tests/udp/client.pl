#!/usr/bin/perl -w
use strict;
use Socket;
use Sys::Hostname;

use Time::HiRes qw(usleep);




my ( $count, $hisiaddr, $hispaddr, $histime,
     $host, $iaddr, $paddr, $port, $proto,
     $rin, $rout, $rtime, $SECS_of_70_YEARS);




$SECS_of_70_YEARS      = 2208988800;




$iaddr = gethostbyname(hostname());
$proto = getprotobyname('udp');
#$port = getservbyname('time', 'udp');
$port = 5000;
$paddr = sockaddr_in(0, $iaddr); # 0 means let kernel pick




socket(SOCKET, PF_INET, SOCK_DGRAM, $proto)   || die "socket: $!";
bind(SOCKET, $paddr)                          || die "bind: $!";

$host = "localhost";

$| = 1;
$count = 0;

$hisiaddr = inet_aton($host)    || die "unknown host";
$hispaddr = sockaddr_in($port, $hisiaddr);
while (1)
{
    $count++;
    my $msg = pack("A40", sprintf("%s", $count));
    print "Sending \"$msg\"!\n";
    #if ($count % 100 == 0)
    #{
    #    print "Sending \"$msg\"!\n";
    #}
    defined(send(SOCKET, $msg, 0, $hispaddr))    || die "send $host: $!";
    #sleep(1);
    #usleep(20000);
}
