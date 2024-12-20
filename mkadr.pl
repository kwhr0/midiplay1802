#! /bin/sh

perl -ne 'print "$1\t$2\n" if /\/([ \dA-F]{4}) :\s+_(\w+):/;' $1
