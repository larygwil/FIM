#!/bin/sh

# tests font functionality in fim
# this script shall work whatever its configure options (even if fim was compiled 'dumb')

f=src/fim 
if test x != "$top_srcdir"; then true ; else top_srcdir=./ ; fi
ff="$top_srcdir/media/fim.png media/icon_smile.gif"
fa="-b -c quit $ff "
fv="-V "

fail()
{
	echo "[!] $@";
	exit -1;
}

resign()
{
	echo "[~] $@";
	exit 0;
}
succeed()
{
	echo "[*] $@";
	exit 0;
}

which grep || fail "we don't go anywhere without grep in our pocket"
g="`which grep` -i"

export FBFONT=''
e='s/^.*://g'

POD='\(sdl\|fb\|imlib2\)'
$f $fv 2>&1 | $g -i 'supported output devices.*:' | $g "$POD"|| resign "missing a pixel oriented driver (as one of $POD)"
$f $fv 2>&1 | $g 'supported file formats:'   | $g '\(png\|gif\)' | resign "missing adequate file format support"

export FBFONT=/dev/null
if $f $fa ; then fail "$f $fa does not fail as it should on wrong font file" ; 
else echo "$f $fa correctly recognizes an invalid FBFONT variable and exits" ; fi

export FBFONT=$top_srcdir/var/fonts/Lat15-Terminus16.psf
if ! $f $fa ; then fail "$f $fa fails, but it should not, as a correct font was provided" ; 
else echo "$f $fa correctly recognizes a valid font file" ; fi

succeed "Font environment variables check PASSED"
exit 0

