package IP::Noise::C::Parser::Prob;

use IP::Noise::C::Parser::Exception;

sub parse_prob
{
    my $stream = shift;
    
    my $orig_line = $stream->peak_line();

    my $line = $orig_line;

    # Strip the leading whitespace
    $line =~ s/^\s+//;

    # If there is a decimal fraction here.
    if ($line =~ /^(\d+\.\d+)/)
    {
        my $prob = $1;

        if (($prob < 0) || ($prob > 1))
        {
            die IP::Noise::C::Parser::Exception->new(
                'text' => "Probability value out of range",
                'line' => $stream->get_line_num(),
                'context' => $prob,
                );
        }

        # Remove that from the line;
        $line =~ s/^(\d+\.\d+)//; 
        $stream->increment_ptr(length($orig_line)-length($line));

        return $prob;
    }
    else
    {
        die IP::Noise::C::Parser::Exception->new(
                'text' => "Illegal probability value",
                'line' => $stream->get_line_num(),
                'context' => $line,
                );
    }
}

1;
