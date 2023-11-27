#!/bin/bash

# Global parameters.
CURRENT_DIR=`pwd`
SUB_DIR="$CURRENT_DIR/submissions"
EXP_RES_DIR="$CURRENT_DIR/testcases/part1"

# Unexpected error messages.
UN_EXP_MSG="invalid pid
    |invalid state
    |Div-by-zero
    |Bad write address
    |Opps.. out of memory for region
    |Opps.. free address not in region
    |Error in PFN refcounting count
    |PageFault:@ pid
    |\(Sig_Exit\) PF Error @"

echo "MARK GENERATION IS STARTED"

# Find differences of outputs.
ACT_RES_DIR="$SUB_DIR/output/part1/"
i=1
TOTAL_MARKS=0
while [ $i -le 15 ];
do

    EXP_RES_FILE="$EXP_RES_DIR/testcase$i.output"
    ACT_RES_FILE="$ACT_RES_DIR/testcase$i.output"

    # Case 1.0: Give 0 if test is failed.
    echo "  --- Checking for failed case."
    MARKS_PART1=0
    IS_FAILED=`cat $ACT_RES_FILE | grep "failed"`
    if [ -z "$IS_FAILED" ];
    then

        # Case 1.1:  Give 1.5 marks if vm area address range + num pages + permissions are correct.
        echo "  --- Checking for pages diff case."
        EXP_PAGES_OUT=`cat $EXP_RES_FILE | grep "#PAGES"`
        ACT_PAGES_OUT=`cat $ACT_RES_FILE | grep "#PAGES"`
        DIFF_OUT_PAGES=`diff -wB <(echo $EXP_PAGES_OUT) <(echo $ACT_PAGES_OUT)`
        if [ -z "$DIFF_OUT_PAGES" ];
        then
            MARKS_PART1=$(echo "($MARKS_PART1+1.5)" | bc)
        else
            echo "         ++++ Pages diff failed: ($DIFF_OUT_PAGES)"
        fi

        # Case 1.2: Give .5 marks if count of vm areas is correct.
        echo "  --- Checking for vm areas diff case."
        EXP_VM_AREAS_OUT=`cat $EXP_RES_FILE | grep "VM_Area"`
        ACT_VM_AREAS_OUT=`cat $ACT_RES_FILE | grep "VM_Area"`
        DIFF_OUT_VM_AREAS=`diff -wB <(echo $EXP_VM_AREAS_OUT) <(echo $ACT_VM_AREAS_OUT)`
        if [ -z "$DIFF_OUT_VM_AREAS" ];
        then
            MARKS_PART1=$(echo "($MARKS_PART1+0.5)" | bc)
        else
            echo "         ++++ VM Areas diff failed: ($DIFF_OUT_VM_AREAS)"
        fi

        # Case 1.3: Deduct 50% if any unexpected error/ bug message is there.
        echo "  --- Checking for penalty cases."
        GREP_OUT_ERR=`grep -E "$UN_EXP_MSG" "$ACT_RES_FILE"`
        if [ -n "$GREP_OUT_ERR" ];
        then
            DEDUCT_MARK=$(echo "($MARKS_PART1*0.5)" | bc)
            MARKS_PART1=$(echo "($MARKS_PART1-$DEDUCT_MARK)" | bc)
            echo "         ++++ Penalty case found: ($GREP_OUT_ERR)"
        fi

    fi

    TOTAL_MARKS=$(echo "($TOTAL_MARKS+$MARKS_PART1)" | bc)

    # Increment the counter.
    ((i++))

done

# Finally show the marks.
echo "Total Marks: $TOTAL_MARKS"

echo "MARK GENERATION IS ENDED"
