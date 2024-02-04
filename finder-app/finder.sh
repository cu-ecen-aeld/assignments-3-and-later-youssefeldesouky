#!/usr/bin/bash

if [ $# -lt 2 ]; then
    echo "Usage: ./finder filesdir searchstr"
    exit 1
fi

filesdir=$1
searchstr=$2

if [ ! -d "$filesdir" ]; then
    echo "Error: $1 : No such directory"
    exit 1
fi

files_count=`find $filesdir -type f | wc -l`
lines_count=`grep -o $searchstr $filesdir/* 2>/dev/null | wc -l`

echo "The number of files are $files_count and the number of matching lines are $lines_count"

exit 0