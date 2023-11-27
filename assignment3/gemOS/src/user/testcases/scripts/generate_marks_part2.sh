#!/bin/bash

# Global parameters.
CURRENT_DIR=`pwd`
SUB_DIR="$CURRENT_DIR/submissions"
EXP_RES_DIR="$CURRENT_DIR/testcases/part2"

# Unexpected error messages for type 1 test cases.
UN_EXP_MSG_T1="invalid pid
    |invalid state
    |Div-by-zero
    |Bad write address
    |Opps.. out of memory for region
    |Opps.. free address not in region
    |Error in PFN refcounting count
    |PageFault:@ pid
    |\(Sig_Exit\) PF Error @"

# Unexpected error messages for type 2 test cases.
UN_EXP_MSG_T2="invalid pid
    |invalid state
    |Div-by-zero
    |Bad write address
    |Opps.. out of memory for region
    |Opps.. free address not in region
    |Error in PFN refcounting count
    |PageFault:@ pid"

echo "MARK GENERATION IS STARTED"

# Find differences of outputs for type 1 test cases, i.e., 1, 3, 5, 8, 10, 11 and 12.
ACT_RES_DIR="$SUB_DIR/output/part2/"
TOTAL_MARKS=0
for i in 1 3 5 8 10 11 12
do

    EXP_RES_FILE="$EXP_RES_DIR/testcase$i.output"
    ACT_RES_FILE="$ACT_RES_DIR/testcase$i.output"

    # Case 1.0: Give 0 if test is failed.
    echo "  --- Checking for failed case ($i)."
    MARKS_PART2_T1=0
    IS_FAILED=`cat $ACT_RES_FILE | grep "failed"`
    if [ -z "$IS_FAILED" ];
    then

        # Case 1.1: Give 1.5 marks if page faults counts are correct.
        echo "  --- Checking for page faults diff case."
        EXP_PFS_OUT=`cat $EXP_RES_FILE | grep "MMAP_Page_Faults" | sed 's/\t/ /g' | tr -s ' ' | cut -d '[' -f3`
        ACT_PFS_OUT=`cat $ACT_RES_FILE | grep "MMAP_Page_Faults" | sed 's/\t/ /g' | tr -s ' ' | cut -d '[' -f3`
        DIFF_OUT_PFS=`diff -wB <(echo $EXP_PFS_OUT) <(echo $ACT_PFS_OUT)`
        if [ -z "$DIFF_OUT_PFS" ];
        then
            MARKS_PART2_T1=$(echo "($MARKS_PART2_T1+1.5)" | bc)
        else
                echo "         ++++ Pages faults diff failed: ($DIFF_OUT_PFS)"
        fi

        # Case 1.2: Give 0.5 marks if EOP string is printed.
        echo "  --- Checking for \"Reached end of the program\" string case."
        ACT_EOP_OUT=`cat $ACT_RES_FILE | grep "Reached end of the program"`
        if [ -n "$ACT_EOP_OUT" ];
        then
            MARKS_PART2_T1=$(echo "($MARKS_PART2_T1+0.5)" | bc)
        else
                echo "         ++++ EOP check failed !!!"
        fi

        # Case 1.3: Deduct 50% if any unexpected error/ bug message is there.
        echo "  --- Checking for penalty cases."
        GREP_OUT_ERR=`grep -E "$UN_EXP_MSG_T1" "$ACT_RES_FILE"`
        if [ -n "$GREP_OUT_ERR" ];
        then
            DEDUCT_MARK=$(echo "($MARKS_PART2_T1*0.5)" | bc)
            MARKS_PART2_T1=$(echo "($MARKS_PART2_T1-$DEDUCT_MARK)" | bc)
            echo "         ++++ Penalty case found: ($GREP_OUT_ERR)"
        fi

    fi

    TOTAL_MARKS=$(echo "($TOTAL_MARKS+$MARKS_PART2_T1)" | bc)

done

# Find differences of outputs for type 2 test cases, i.e., 2, 4, 6, 7, 9, 13, 14 and 15.
for i in  2 4 6 7 9 13 14 15
do

    EXP_RES_FILE="$EXP_RES_DIR/testcase$i.output"
    ACT_RES_FILE="$ACT_RES_DIR/testcase$i.output"

    # Case 1.0: Give 0 if test is failed.
    echo "  --- Checking for failed case ($i)."
    MARKS_PART2_T2=0
    IS_FAILED=`cat $ACT_RES_FILE | grep "failed"`
    if [ -z "$IS_FAILED" ];
    then

        # Case 1.1: Give 0.5 marks if page faults counts are correct.
        echo "  --- Checking for page faults diff case."
        EXP_PFS_OUT=`cat $EXP_RES_FILE | grep "MMAP_Page_Faults" | sed 's/\t/ /g' | tr -s ' ' | cut -d '[' -f3`
        ACT_PFS_OUT=`cat $ACT_RES_FILE | grep "MMAP_Page_Faults" | sed 's/\t/ /g' | tr -s ' ' | cut -d '[' -f3`
        DIFF_OUT_PFS=`diff -wB <(echo $EXP_PFS_OUT) <(echo $ACT_PFS_OUT)`
        if [ -z "$DIFF_OUT_PFS" ];
        then
            MARKS_PART2_T2=$(echo "($MARKS_PART2_T2+0.5)" | bc)
        else
                echo "         ++++ Pages faults diff failed: ($DIFF_OUT_PFS)"
        fi

        # Case 1.2: Give 0.5 marks if EOP string is printed.
        echo "  --- Checking for \"Reached end of the program\" string case."
        ACT_EOP_OUT=`cat $ACT_RES_FILE | grep "Reached end of the program"`
        if [ -n "$ACT_EOP_OUT" ];
        then
            MARKS_PART2_T2=$(echo "($MARKS_PART2_T2+0.5)" | bc)
        else
                echo "         ++++ EOP check failed !!!"
        fi

        # Case 1.3: Give 1 mark if proper SIG EXIT message is printed.
        echo "  --- Checking for proper SIG EXIT message printing case."
        SIG_EXIT_MSG=""
        case $i in
            2)
                SIG_EXIT_MSG="(Sig_Exit) PF Error @ \[RIP: 0x100000E0F\] \[accessed VA: 0x180204000\] \[error code: 0x7\]"
                ;;
            4)
                SIG_EXIT_MSG="(Sig_Exit) PF Error @ \[RIP: 0x100000E4D\] \[accessed VA: 0x180201000\] \[error code: 0x4\]"
                ;;
            6)
                SIG_EXIT_MSG="(Sig_Exit) PF Error @ \[RIP: 0x100000E7F\] \[accessed VA: 0x180201000\] \[error code: 0x4\]"
                ;;
            7)
                SIG_EXIT_MSG="(Sig_Exit) PF Error @ \[RIP: 0x100000E50\] \[accessed VA: 0x180201000\] \[error code: 0x6\]"
                ;;
            9)
                SIG_EXIT_MSG="(Sig_Exit) PF Error @ \[RIP: 0x100000ED5\] \[accessed VA: 0x180201000\] \[error code: 0x7\]"
                ;;
            13)
                SIG_EXIT_MSG="(Sig_Exit) PF Error @ \[RIP: 0x100000DEB\] \[accessed VA: 0x180202000\] \[error code: 0x6\]"
                ;;
            14)
                SIG_EXIT_MSG="(Sig_Exit) PF Error @ \[RIP: 0x100000DE1\] \[accessed VA: 0x180201000\] \[error code: 0x6\]"
                ;;
            15)
                SIG_EXIT_MSG="(Sig_Exit) PF Error @ \[RIP: 0x100000E37\] \[accessed VA: 0x180201000\] \[error code: 0x7\]"
                ;;
            *)
                echo "ERROR: NOT EXPECTED ($i) FOR TYPE 2 CHECKING !!!"
                ;;
        esac
        ACT_SIG_EXIT_MSG_OUT=`cat $ACT_RES_FILE | grep "$SIG_EXIT_MSG"`
        if [ -n "$ACT_SIG_EXIT_MSG_OUT" ];
        then
            MARKS_PART2_T2=$(echo "($MARKS_PART2_T2+1)" | bc)
        else
                echo "         ++++ SIG EXIT check failed !!!"
        fi

        # Case 1.4: Deduct 50% if any unexpected error/ bug message is there.
        echo "  --- Checking for penalty cases."
        GREP_OUT_ERR=`grep -E "$UN_EXP_MSG_T2" "$ACT_RES_FILE"`
        if [ -n "$GREP_OUT_ERR" ];
        then
            DEDUCT_MARK=$(echo "($MARKS_PART2_T2*0.5)" | bc)
            MARKS_PART2_T2=$(echo "($MARKS_PART2_T2-$DEDUCT_MARK)" | bc)
            echo "         ++++ Penalty case found: ($GREP_OUT_ERR)"
        fi

    fi

    TOTAL_MARKS=$(echo "($TOTAL_MARKS+$MARKS_PART2_T2)" | bc)

done

# Finally show the marks.
echo "Total Marks: $TOTAL_MARKS"

echo "MARK GENERATION IS ENDED"
