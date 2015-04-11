#!/usr/bin/env perl
use strict;
use warnings;
use utf8;

# Yields versions such as 2015.101.755 for timestamp 2015-04-11 12:35:00 UTC
my $h = `date -u '+\%H'`;
my $m = `date -u '+\%M'`;
my $ver_dot = `date -u '+\%Y.\%j.'`;
chomp($ver_dot);
$ver_dot .= ($h*60 + $m);
my $ver_comma = $ver_dot;
$ver_comma =~ s/\./,/g;

# <File Id='FomaEXE' Name='flookup.exe' DiskId='1' Source='backend/hfst-ospell.exe' KeyPath='yes' />

# <Icon Id='{NAME}.ico' SourceFile='{NAME}.ico'/>
# <Property Id='ARPPRODUCTICON' Value='{NAME}.ico'/>
