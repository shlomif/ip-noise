# This is the base package for the IP-Noise project.
#
# It contains some constants, globals and definitions
#
package IP::Noise;

# Get the maximal string length an identifier can have.
# An identifier is something like a Chain's or a State's name
sub get_max_id_string_len
{
    return 79;
}

1;
