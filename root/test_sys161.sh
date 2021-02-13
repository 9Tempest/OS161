#!/bin/bash

TEST=$1
TEST_NUM=$2

if [ $# -ne 2 ];then
    echo "utils: bash test_sys161.sh [test name] [test number] "
fi

for (( i=0; i < ${TEST_NUM}; i++ ))
do
    sys161 kernel "${TEST};q"
done