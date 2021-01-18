#!/bin/bash

if [ $# -ne 1 ]; then
    echo "invalid numbers of arguments, please input ASSTX"
    exit
fi 

ASST=$1
old_add=$(pwd)

#step1
function conf {
    cd ../os161-1.99/kern/conf/
    ./config ${ASST}
    echo "configuration done~"
}

#step2
function build {
    cd ../compile/${ASST}
    bmake depend
    bmake
    bmake install
    echo "building done~"
}

conf
build
