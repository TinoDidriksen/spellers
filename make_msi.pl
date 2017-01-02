#!/usr/bin/env perl
# -*- mode: cperl; indent-tabs-mode: nil; tab-width: 3; cperl-indent-level: 3; -*-
# Copyright (C) 2015, Tino Didriksen <mail@tinodidriksen.com>
# Licensed under the GNU GPL version 3 or later; see http://www.gnu.org/licenses/
use strict;
use warnings;
use utf8;
use File::Basename;
BEGIN {
	$| = 1;
	binmode(STDIN, ':encoding(UTF-8)');
	binmode(STDOUT, ':encoding(UTF-8)');
}
use open qw( :encoding(UTF-8) :std );

if (! defined $ARGV[0]) {
   die "Must provide implementation name!\n";
}

sub file_get_contents {
   my ($fn) = @_;
	local $/ = undef;
	open FILE, "<$fn" or die "Could not read $fn: $!\n";
	my $data = <FILE>;
   close FILE;
   return $data;
}

sub file_put_contents {
   my ($fn,$data) = @_;
	open FILE, ">$fn" or die "Could not write $fn: $!\n";
   print FILE $data;
   close FILE;
}

my $wxs = 'speller.wxs';
my $ini = 'impls/'.$ARGV[0].'.ini';
foreach my $f (($wxs, $ini)) {
   if (! -s $f) {
      die "Could not find $f!\n";
   }
}

# Yields versions such as 86.114.29319 for timestamp 1450340999
my $t = time();
my ($ma,$mi,$pa) = ( (($t >> 24) & 0xFF), (($t >> 16) & 0xFF), ($t & 0xFFFF) );
my $ver_dot = sprintf('%u.%u.%u', $ma, $mi, $pa);
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

$conf{UUID_ORG} = $conf{UUID};
if ($ENV{WINX} eq 'win64') {
   $conf{UUID} =~ s@...$@640@;
   $conf{NAME} .= ' (x64)';
}

$wxs = file_get_contents($wxs);

my $dir = 'build/'.$ARGV[0];
print `rm -rfv '$dir'`;
print `mkdir -pv '$dir'`;
print `cp -av '$ini' '$dir/speller.ini'`;
{
	local $/ = undef;
   open FILE, "<$dir/speller.ini" or die "$!\n";
   my $ini = <FILE>;
   close FILE;
   $ini =~ s@$conf{UUID_ORG}@$conf{UUID}@g;
   open FILE, ">$dir/speller.ini" or die "$!\n";
   print FILE $ini;
   close FILE;
}

my $icon = '';
if (-s 'impls/'.$ARGV[0].'.ico') {
   print `cp -av 'impls/$ARGV[0].ico' '$dir/'`;
   $icon .= "    <Icon Id='$ARGV[0].ico' SourceFile='$ARGV[0].ico'/>\n";
   $icon .= "    <Property Id='ARPPRODUCTICON' Value='$ARGV[0].ico'/>\n";
}
$wxs =~ s/{ICON}\n?/$icon/g;

$wxs =~ s/{NAME}/$conf{NAME}/g;
$wxs =~ s/{LOCALES}/$conf{LOCALES}/g;
$wxs =~ s/{VERSION_DOT}/$ver_dot/g;
$wxs =~ s/{VERSION_COMMA}/$ver_comma/g;

for (my $i=0 ; $i<9 ; $i++) {
   $conf{UUID} =~ s/.$/$i/;
   $wxs =~ s/{UUID$i}/$conf{UUID}/g;
}

my $tmp = "/tmp/speller-$$";
print `./impls/backend.sh '$tmp' '$conf{SOURCE}'`;
print `mv -v '$tmp/backend' '$dir/'`;
print `rm -rfv '$tmp'`;

my $backend = '';
foreach my $f (glob("$dir/backend/*")) {
   $f =~ s@^$dir/@@;
   my $base = basename($f);
   my $id = $base;
   $id =~ s/[^A-Za-z0-9]+/_/g;
   $backend .= "              <File Id='Backend_$id' Name='$base' DiskId='1' Source='$f' />\n";
}
$backend =~ s@ />@ KeyPath='yes' />@;
if ($backend !~ m@\.zhfst@) {
   die "Backend did not contain a ZHFST file!\n";
}
$wxs =~ s/{BACKEND_FILES}\n?/$backend/g;

my $lregs = '';
foreach my $locale (split / /, $conf{LOCALES}) {
   my $id = $locale;
   $id =~ s/[^A-Za-z0-9]+/_/g;
   $lregs .= "            <RegistryKey Id='SpellerRegLocal-$id' Root='HKCU' Key='Software\\Microsoft\\Shared Tools\\Proofing Tools\\1.0\\Override\\$locale' ForceDeleteOnUninstall='yes'>\n";
   $lregs .= "              <RegistryValue Type='string' Name='DLL' Value='[INSTALLDIR]office32.dll' />\n";
   $lregs .= "              <RegistryValue Type='string' Name='LEX' Value='[INSTALLDIR]speller.ini' />\n";
   $lregs .= "              <RegistryValue Type='string' Name='DLL64' Value='[INSTALLDIR]office64.dll' />\n";
   $lregs .= "              <RegistryValue Type='string' Name='LEX64' Value='[INSTALLDIR]speller.ini' />\n";
   $lregs .= "            </RegistryKey>\n";
}
$wxs =~ s/{LOCALE_REGS}\n?/$lregs/g;

open FILE, ">$dir/$ARGV[0].wxs" or die "Could not open output wxs: $!\n";
print FILE $wxs;
close FILE;

# print `cp -av '/opt/mxe/usr/i686-w64-mingw32.shared/bin/libstdc++-6.dll' '/opt/mxe/usr/i686-w64-mingw32.shared/bin/libgcc_s_sjlj-1.dll' '$dir/'`;

chdir $dir;
print `find . -type f -name '*.exe' -or -name '*.dll' | xargs -rn1 /opt/mxe/usr/bin/i686-w64-mingw32.shared-strip`;
print `rm -fv *.msi`;
if ($ENV{WINX} eq 'win64') {
   print `find . -type f -name '*.exe' -or -name '*.dll' | xargs -rn1 /opt/mxe/usr/bin/x86_64-w64-mingw32.shared-strip`;
   {
      local $/ = undef;
      open FILE, "<$ARGV[0].wxs" or die "$!\n";
      my $wsx = <FILE>;
      close FILE;
      $wsx =~ s@ProgramFilesFolder@ProgramFiles64Folder@g;
      $wsx =~ s@(<Package [^>]+?)\s*/>@$1 Platform='x64' />@g;
      $wsx =~ s@(<Component [^>]+?)\s*>@$1 Win64='yes'>@g;
      open FILE, ">$ARGV[0].wxs" or die "$!\n";
      print FILE $wsx;
      close FILE;
   }
   print `wixl -a x64 -v '$ARGV[0].wxs'`;
}
else {
   print `wixl -v '$ARGV[0].wxs'`;
}
print `osslsigncode -pkcs12 '/root/.keys/2015-11-23 TDC Code Signing.p12' -readpass '/root/.keys/2015-11-23 TDC Code Signing.key' -t http://timestamp.verisign.com/scripts/timstamp.dll -in '$ARGV[0].msi' -out '$ARGV[0]-$ver_dot-$ENV{WINX}.msi'`;
print `rm -fv '$ARGV[0].msi'`;
