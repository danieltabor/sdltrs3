#!/bin/sh
test_dir=`dirname $0`
target="$test_dir/../src/linux/sdltrs"
rom="$test_dir/model3.rom"
disk="$test_dir/metmis2a.dsk"
"$target" -model3 -scale 2 -romfile3 "$rom" -disk0 "$disk"
