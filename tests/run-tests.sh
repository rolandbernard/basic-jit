
passed_count=0
failed_count=0

function runTest {
    if [ ${1: -6} == .basic ]
    then
        for i in $(seq 1 $2)
        do
            echo -n "  "
        done
        if timeout 0.5s $COMPILER $1 &> /tmp/basic-test.out
        then
            if [ ${3::5} == fail. ]
            then
                echo -e "\e[31mFailed\e[m test '$3' should have failed"
                failed_count=$(expr $failed_count + 1)
            else 
                echo -e "\e[32mPassed\e[m test '$3'"
                passed_count=$(expr $passed_count + 1)
            fi
        else
            if [ ${3::5} == fail. ]
            then
                echo -e "\e[32mPassed\e[m test '$3'"
                passed_count=$(expr $passed_count + 1)
            else 
                echo -e "\e[31mFailed\e[m test '$3' at runtime ($?)"
                failed_count=$(expr $failed_count + 1)
                for i in $(seq 0 $2)
                do
                    sed -i "s/^/  /" /tmp/basic-test.out
                done
                cat /tmp/basic-test.out
            fi
        fi
    fi
}

function runTests {
    for i in $(seq 1 $2)
    do
        echo -n "  "
    done
    echo "Running tests in: '$3'"
    for test in $(find $1 -mindepth 1 -maxdepth 1)
    do
        name=$(awk -F/ '{print $NF}' <<< $test)
        if [ ${name::7} != ignore. ]
        then
            if [ -f $test ]
            then
                runTest $test $(expr $2 + 1) $name
            elif [ -d $test ]
            then
                runTests $test $(expr $2 + 1) $name
            fi
        fi
    done
}

if [ $# -gt 1 -a -d $1 ]
then
    echo
    path=$(realpath $1)
    name=$(awk -F/ '{print $NF}' <<< $path)
    shift
    COMPILER=$@
    runTests $path 1 $name
    echo

    echo -e "\e[32mPassed $passed_count tests\e[m"
    echo -e "\e[31mFailed $failed_count tests\e[m"
    if [ $failed_count -gt 0 ]
    then
        exit 1
    else
        exit 0
    fi
else
    echo "Usage: $0 TESTDIR COMPILER"
    exit 2
fi

