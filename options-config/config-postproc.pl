#!/usr/bin/perl

$usage = "usage: $0 <default config file> <config file>\n";

die "$usage" unless @ARGV;
$defaults = shift @ARGV;
die "$usage" unless @ARGV;
die "Could not open $ARGV[0]" unless -T $ARGV[0];

sub yank {
    @option = grep(!($_ =~ /$_[0]\s*=/), @option);
}

open(DEFAULTS, $defaults) || die "Could not open $defaults\n";

# get the full list of available options using the default config file
$i = 0;
while (<DEFAULTS>) {
    if (/^\s*OPTION_(\w+\s*=.*$)/) {
	$option[$i++] = $1;
    }
}

# now go through the config file, making the necessary changes
while (<>) {
    if (/Linux Kernel Configuration/) {
	# change title
	s/Linux Kernel/Option Groups/;
	print;
    } elsif (/^\s*CONFIG_(\w+)\s*=/) {
	# this is an explicit option set line, change CONFIG_ to OPTION_
	# before printing and remove this option from option list
	$opt = $1;
	yank($opt);
	s/CONFIG_/OPTION_/g;
	print;
    } elsif (/^\s*#\s+CONFIG_(\w+) is not set/) {
	# this is a comment line for an unset boolean option, change CONFIG_
	# to OPTION_, remove this option from option list, and convert to
	# explicit OPTION_FOO=n
	$opt = $1;
	yank($opt);
	s/CONFIG_/OPTION_/g;
	print "OPTION_$opt=n\n";
    } else {
	print;
    }
}

# any boolean options left in @options, are options that were not mentioned in
# the config file, and implicitly that means the option must be set =n,
# so do that here.
foreach $opt (@option) {
    if ($opt =~ /=\s*[yn]/) {
	$opt =~ s/=\s*[yn]/=n/;
	print "OPTION_$opt\n";
    }
}
