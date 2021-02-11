#! /bin/bash

PROGRAM=$1
TEST_NUM=$2
ARGS=$3
if [ $# -ne 3 ];then
    echo "utils: bash test_script.sh [program name] [test number] [args pass to the program]"
fi

echo "begin test~" 
echo "The program is ${PROGRAM}, the test time is ${TEST_NUM}"


for (( i=0; i < ${TEST_NUM}; i++ ))
do
    echo "execute the program for the ${i} time"
    
    echo ${ARGS} | xargs ${PROGRAM}
    if [ $? -ne 0 ]; then
        echo "program error! exited!"
        exit
    fi
done