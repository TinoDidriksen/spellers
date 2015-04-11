#!/usr/bin/env perl
# -*- mode: cperl; indent-tabs-mode: nil; tab-width: 3; cperl-indent-level: 3; -*-
# Copyright (C) 2015, Tino Didriksen <mail@tinodidriksen.com>
# Licensed under the GNU GPL version 3 or later; see http://www.gnu.org/licenses/
use strict;
use warnings;
use utf8;
BEGIN {
	$| = 1;
	binmode(STDIN, ':encoding(UTF-8)');
	binmode(STDOUT, ':encoding(UTF-8)');
}
use open qw( :encoding(UTF-8) :std );

if (! defined $ARGV[0]) {
   die "Must provide implementation name!\n";
}

my $wxs = 'speller.wxs';
my $ini = 'impls/'.$ARGV[0].'.ini';
foreach my $f (($wxs, $ini)) {
   if (! -s $f) {
      die "Could not find $f!\n";
   }
}

# Yields versions such as 2015.101.755 for timestamp 2015-04-11 12:35:00 UTC
my $h = `date -u '+\%H'`;
my $m = `date -u '+\%M'`;
my $ver_dot = `date -u '+\%Y.\%j.'`;
chomp($ver_dot);
$ver_dot .= ($h*60 + $m);
my $ver_comma = $ver_dot;
$ver_comma =~ s/\./,/g;

my %conf;
open FILE, $ini or die "Could not open $ini: $!\n";
while (<FILE>) {
   s/^\s+//g;
   s/\s+$//g;
   if (!$_ || $_ =~ /^#/) {
      next;
   }
   my ($k,$v) = ($_ =~ m/^([^=\s]+)\s*=\s*(.+)$/);
   $conf{$k} = $v;
}
close FILE;

{
	local $/ = undef;
	open FILE, $wxs or die "Could not open $wxs: $!\n";
	$wxs = <FILE>;
   close FILE;
}

my $icon = '';
if (-s 'impls/'.$ARGV[0].'.ico') {
   $icon .= "    <Icon Id='{NAME}.ico' SourceFile='{NAME}.ico'/>\n";
   $icon .= "    <Property Id='ARPPRODUCTICON' Value='{NAME}.ico'/>\n";
}
$wxs =~ s/{ICON}\n?/$icon/g;

$wxs =~ s/{NAME}/$conf{NAME}/g;
$wxs =~ s/{LOCALE}/$conf{LOCALE}/g;
$wxs =~ s/{VERSION_DOT}/$ver_dot/g;
$wxs =~ s/{VERSION_COMMA}/$ver_comma/g;

for (my $i=0 ; $i<9 ; $i++) {
   $conf{UUID} =~ s/.$/$i/;
   $wxs =~ s/{UUID$i}/$conf{UUID}/g;
}

# <File Id='FomaEXE' Name='flookup.exe' DiskId='1' Source='backend/hfst-ospell.exe' KeyPath='yes' />

print $wxs;
