#!/bin/bash

# Global parameters.
CURRENT_DIR=`pwd`
SUB_DIR="$CURRENT_DIR/submissions"
EXP_RES_DIR="$CURRENT_DIR/testcases/part3"

echo "MARK GENERATION IS STARTED"

# Find differences of outputs for test cases 1 to 5 (2 marks each).
ACT_RES_DIR="$SUB_DIR/output/part3/"
TOTAL_MARKS=0
for i in 1 2 3 4 5
do

    EXP_RES_FILE="$EXP_RES_DIR/testcase$i.output"
    ACT_RES_FILE="$ACT_RES_DIR/testcase$i.output"
    
    # Give 2 marks if matched exactely.
    MARKS_PART3_T1=0
    echo "  --- Checking for exact match ($i)."
    EXP_RES_OUT=`cat $EXP_RES_FILE`
    ACT_RES_OUT=`tail -n +3 $ACT_RES_FILE`
    DIFF_OUT=`diff -wB <(echo $EXP_RES_OUT) <(echo $ACT_RES_OUT)`
    if [ -z "$DIFF_OUT" ];
    then
        MARKS_PART3_T1=$(echo "($MARKS_PART3_T1+2)" | bc)
    else
            echo "         ++++ Exact match failed: ($DIFF_OUT)"
    fi

    TOTAL_MARKS=$(echo "($TOTAL_MARKS+$MARKS_PART3_T1)" | bc)

done

# Find differences of outputs for test cases 6 to 15 (3 marks each).
for i in 6 7 8 9 10 11 12 13 14 15
do

    EXP_RES_FILE="$EXP_RES_DIR/testcase$i.output"
    ACT_RES_FILE="$ACT_RES_DIR/testcase$i.output"
    
    # Give 2 marks if matched exactely.
    MARKS_PART3_T2=0
    echo "  --- Checking for exact match ($i)."
    EXP_RES_OUT=`cat $EXP_RES_FILE`
    ACT_RES_OUT=`tail -n +3 $ACT_RES_FILE`
    DIFF_OUT=`diff -wB <(echo $EXP_RES_OUT) <(echo $ACT_RES_OUT)`
    if [ -z "$DIFF_OUT" ];
    then
        MARKS_PART3_T2=$(echo "($MARKS_PART3_T2+3)" | bc)
    else
            echo "         ++++ Exact match failed: ($DIFF_OUT)"
    fi

    TOTAL_MARKS=$(echo "($TOTAL_MARKS+$MARKS_PART3_T2)" | bc)

done

# Finally show the marks.
echo "Total Marks: $TOTAL_MARKS"

echo "MARK GENERATION IS ENDED"
