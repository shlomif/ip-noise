package Packet::Logic;

use strict;

use IPTables::IPv4::IPQueue qw(:constants);
use NetPacket::IP;
use NetPacket::TCP;

sub new
{
    my $class = shift;
    my $self = {};

    bless $self, $class;

    $self->initialize(@_);

    return $self;
}

# This function parses the file and splits it into sections where
# each section corresponds to a rule.
#
# Each section contains the rule's arguments in an array.
sub get_rules_args
{
    my $filename = shift;

    my (@rules_list, @rule, $line);
    open I, "<". $filename;
    @rule = ();
    while ($line = <I>)
    {
        chomp ($line);
        # Indicates that we should switch to the next rule.
        if ($line =~ /<new rule>/i)
        {
            # If the rule is not empty
            if (scalar(@rule) > 0)
            {
                # Insert into the rule list a fresh copy of rule
                push @rules_list, [ @rule ];
            }
            # Clean rule so we can read the next rule
            @rule = ();
            # Read the next rule
            next;
        }
        # If the line is not empty.
        if ($line =~ /\S/)
        {
            push @rule, $line;
        }
    }
    push @rules_list, [@rule];
    close(I);

    return \@rules_list;
}

# This function takes _one_ array of a rule's arguments and
# and structures it as a rule hash.
#
# There's some duplicate code inside this function so it may be wise to 
# modulate it a bit.
sub rule_args_to_rule
{
    my %rule = 
    (
        'drop_prob' => 0,
        'delay_prob' => 0,        
    );

    my ($arg);
    my ($start, $end, @components);

    while (scalar(@_))
    {
        $arg = shift;
        if (($arg eq "--source") || ($arg eq "-s"))
        {
            $arg = shift;

            # If it's a range, treat it as such.
            # If not, treat is as a range with one IP.
            if ($arg =~ /-/)
            {                
                @components = split(/ *- */, $arg);
                $start = $components[0];
                $end = $components[1];                
            }
            else
            {
                $start = $end = $arg;
            }

            $rule{'source'} = { 'start' => $start, 'end' => $end };
        }
        elsif (($arg eq "--dest") || ($arg eq "--destination") || ($arg eq "-d"))
        {
            $arg = shift;

            # If it's a range, treat it as such.
            # If not, treat is as a range with one IP.
            if ($arg =~ /-/)
            {                
                @components = split(/ *- */, $arg);
                $start = $components[0];
                $end = $components[1];                
            }
            else
            {
                $start = $end = $arg;
            }

            $rule{'dest'} = { 'start' => $start, 'end' => $end };
        }
        elsif (($arg eq "--drop-probability") || ($arg eq "--drop-prob"))
        {
            $arg = shift;

            if (($arg < 0) || ($arg > 1))
            {
                die "Drop Probability is out of range!\n";
            }
            $rule{'drop_prob'} = $arg;
        }
        elsif (($arg eq "--delay-probability") || ($arg eq "--delay-prob"))
        {
            $arg = shift;

            if (($arg < 0) || ($arg > 1))
            {
                die "Delay Probability is out of range!\n";
            }
            $rule{'delay_prob'} = $arg;
        }
        elsif ($arg eq "--delay-len")
        {
            $arg = shift;

            if ($arg < 0)
            {
                die "Delay Length cannot be less than 0!\n";
            }
            if ($arg !~ /^\d+$/)
            {
                die "Delay length is not an integer!\n";
            }
            $rule{'delay_len'} = $arg;
        }
    }
    
    # TODO: Rule normalization (completing missing arguments)

    if ($rule{'delay_prob'} + $rule{'drop_prob'} > 1)
    {
        die "Drop and Delay probabilities evaluate to more than 1!\n";
    }
    return \%rule;
}

sub parse_rules
{
    my $filename = shift;

    my @rules_args = @{ get_rules_args($filename) };

    my @rules = (map { &rule_args_to_rule(@{$_}); } @rules_args);

    return \@rules;
}

sub initialize
{
    my $self = shift;

    srand(24);

    $self->{'rules'} = parse_rules("logic.txt");

    return 0;
}

sub ip_compare
{
    my $ip1_str = shift;
    my $ip2_str = shift;
    
    my $result;
    
    my @ip1 = split(/\./, $ip1_str);
    my @ip2 = split(/\./, $ip2_str);
    
    for(my $a=0;$a<4;$a++)
    {
        $result = ($ip1[$a] <=> $ip2[$a]);
        if ($result != 0)
        {
            return $result;
        }
    }

    return 0;
}
sub is_in_ip_range
{
    my $range = shift;
    my $ip = shift;

    return ((ip_compare($range->{'start'}, $ip) <= 0) &&
            (ip_compare($ip, $range->{'end'}) <= 0));
}

# This method should return
# $ret == 0 - if the packet should be accepted immidiately
# $ret  > 0 - if the packet should be delayed by $ret quanta
# $ret  < 0 - if the packet should be dropped.
sub decide_what_to_do_with_packet
{
    my $self = shift;

    my $msg = shift;

    if ($msg->data_len())
    {
        my $payload = $msg->payload();

        my $ip = NetPacket::IP->decode($payload);

        my $tcp = NetPacket::TCP->decode(NetPacket::IP::ip_strip($payload));

        my @rules_list = @{$self->{'rules'}};

        # Scan the rules. For each rule, determine if it applies to this
        # packet and if so, determine the packet's outcome.
        foreach my $rule (@rules_list)
        {
            if (exists($rule->{'source'}))
            {
                if (! is_in_ip_range($rule->{'source'}, $ip->{src_ip}))
                {
                    # Skip to the next rule, if the source IP of the packet
                    # is not in this source range.
                    next;
                }               
            }
            if (exists($rule->{'dest'}))
            {
                if (! is_in_ip_range($rule->{'dest'}, $ip->{dest_ip}))
                {
                    # Skip to the next rule, if the destination IP of the 
                    # packet is not in this source range.                
                    next;
                }               
            }

            # This packet matches the criteria of this rule, so let's determine
            # what to do with it.
            my $prob = rand(1);

            if ($prob < $rule->{'drop_prob'})
            {
                return { 'action' => "drop" };
            }
            # That way drop_prob < prob < drop_prob + delay_prob and so
            # the test succeeds in a probability of delay prob.
            elsif ($prob < $rule->{'drop_prob'} + $rule->{'delay_prob'})
            {
                # So far, only a uniform delay is supported.
                my $delay_len = int(rand($rule->{'delay_len'}));

                return 
                    { 
                        'action' => "delay", 
                        'delay_len' => $delay_len
                    }
                    ;
            }
            else
            {
                return { 'action' => "accept" };
            }           
        }

        # None of the tests matched this packet, so let's accept it.

        return { 'action' => "accept" };
    }
    else
    {
        # What should I do with a packet with a data length of 0.
        # I know - accept it.
        return { 'action' => "accept" };
    }
}

1;

