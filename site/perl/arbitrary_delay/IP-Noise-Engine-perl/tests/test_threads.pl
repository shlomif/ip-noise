use strict;

use Thread;

sub mythread_func
{
    while (1)
    {
        print "In the thread!\n";
        sleep 1;
    }
}

my $thr = Thread->new(\&mythread_func);

while(1)
{
    print "Main thread!\n";
    sleep 1;
}
