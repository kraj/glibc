#!/usr/bin/perl

if (@ARGV) {
    while (<>) {
	s/OPTION_/CONFIG_/g;
	print;
    }
}
