#!/usr/bin/perl -w

use strict;

use IP::Noise::Conn;

my $is_arbitrator = shift || 0;

my $conn = IP::Noise::Conn->new($is_arbitrator);

if (! $is_arbitrator)
{
    my $data = $conn->conn_read(16);
    $conn->conn_write("You wrote \"$data\"!");
}
