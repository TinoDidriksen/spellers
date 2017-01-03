#!/usr/bin/env perl
# -*- mode: cperl; indent-tabs-mode: nil; tab-width: 3; cperl-indent-level: 3; -*-
# Copyright (C) 2015, Tino Didriksen <mail@tinodidriksen.com>
# Licensed under the GNU GPL version 3 or later; see http://www.gnu.org/licenses/
use strict;
use warnings;
use utf8;
use File::Basename;
use File::stat;
BEGIN {
	$| = 1;
	binmode(STDIN, ':encoding(UTF-8)');
	binmode(STDOUT, ':encoding(UTF-8)');
}
use open qw( :encoding(UTF-8) :std );

sub fill_file {
   my ($if, $of, %conf) = @_;
   my $data;
   {
      local $/ = undef;
      open FILE, $if or die "Could not open $if: $!\n";
      $data = <FILE>;
      close FILE;
   }
   reset(%conf);
   while (my ($k,$v) = each(%conf)) {
      $data =~ s@\Q{$k}\E\n?@$v@g;
   }
   open FILE, ">$of" or die "Could not open $of: $!\n";
   print FILE $data;
   close FILE;
   print "Filled $if -> $of\n";
}

if (! defined $ARGV[0]) {
   die "Must provide implementation name!\n";
}

my $manifest = 'chrome.manifest';
my $rdf = 'install.rdf';
my $js = 'components/speller.js';
my $ini = 'impls/'.$ARGV[0].'.ini';
foreach my $f (($manifest, $rdf, $js, $ini)) {
   if (! -s $f && ! -s 'mozilla/'.$f) {
      die "Could not find $f!\n";
   }
}

# Load config file
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

my @locales = split / /, $conf{'LOCALES'};
$conf{'LOCALES_JSON'} = "['".join("', '", @locales)."']";

$conf{'LOCALE_LC'} = lc($locales[0]);
$conf{'LOCALE_LC'} =~ s/[^a-z0-9]/_/g;

my $dir = 'build/mozilla/'.$ARGV[0];
print `rm -rfv '$dir'`;
print `mkdir -pv '$dir/components' '$dir/native'`;
print `cp -av '$ini' '$dir/native/speller.ini'`;
print `cp -av mozilla/native/*.dll '$dir/native/'`;

my $tmp = "/tmp/speller-$$";
print `./impls/backend.sh '$tmp' '$conf{SOURCE}'`;
print `mv -v '$tmp/backend' '$dir/'`;
print `rm -rfv '$tmp'`;

# Yields versions such as 86.114.29319 for timestamp 1450340999
my @zhfst = glob("$dir/backend/*.zhfst");
my $t = stat($zhfst[0])->mtime;
my ($ma,$mi,$pa) = ( (($t >> 24) & 0xFF), (($t >> 16) & 0xFF), ($t & 0xFFFF) );
$conf{'VERSION_DOT'} = sprintf('%u.%u.%u', $ma, $mi, $pa);
$conf{'VERSION_COMMA'} = $conf{'VERSION_DOT'};
$conf{'VERSION_COMMA'} =~ s/\./,/g;

$conf{'SKIN'} = '';
$conf{'ICON'} = '';
if (-s 'impls/'.$ARGV[0].'.png') {
   print `mkdir -pv '$dir/skin'`;
   print `cp -av 'impls/$ARGV[0].png' '$dir/skin/icon.png'`;
   $conf{'SKIN'} = "skin speller classic/1.0 skin/\n";
   $conf{'ICON'} = "    <em:iconURL>chrome://speller/skin/icon.png</em:iconURL>\n";
}

for (my $i=0 ; $i<9 ; $i++) {
   $conf{'UUID'.$i} = $conf{'UUID'};
   $conf{'UUID'.$i} =~ s/.$/$i/;
}

fill_file('mozilla/'.$manifest, "$dir/$manifest", %conf);
fill_file('mozilla/'.$rdf, "$dir/$rdf", %conf);
fill_file('mozilla/'.$js, "$dir/$js", %conf);

chdir $dir;
print `find . -type f -name '*.exe' -or -name '*.dll' | xargs -rn1 /opt/mxe/usr/bin/i686-w64-mingw32.shared-strip`;
print `rm -fv '../$ARGV[0]'*.xpi`;
print `zip -9r '../$ARGV[0]-$conf{VERSION_DOT}.xpi' *`;
