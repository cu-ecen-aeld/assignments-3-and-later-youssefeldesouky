#!/usr/bin/bash

if [ $# -lt 2 ]; then
    echo "Usage: ./writer.sh writefile writestr"
    exit 1
fi

writefile=$1
writestr=$2

dirpath=`dirname $1`

mkdir -p $dirpath && touch $writefile && echo $writestr > $writefile

if [ $? -ne 0 ]; then
    echo "Error: file $writefile could not be created"
    exit 1
fi

exit 0