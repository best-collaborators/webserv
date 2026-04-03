#!/bin/bash

PARSER=./webserv

PASS_COUNT=0
FAIL_COUNT=0

run_test() {
    FILE=$1
    EXPECTED=$2

    $PARSER "$FILE" > /dev/null 2>&1
    STATUS=$?

    if [[ $EXPECTED == "PASS" && $STATUS -eq 0 ]]; then
        echo "[OK]   $FILE"
        ((PASS_COUNT++))
    elif [[ $EXPECTED == "FAIL" && $STATUS -ne 0 ]]; then
        echo "[OK]   $FILE (correctly failed)"
        ((PASS_COUNT++))
    else
        echo "[FAIL] $FILE"
        ((FAIL_COUNT++))
    fi
}

echo "Running config parser tests..."
echo "--------------------------------"

run_test test02_missing_listen.conf FAIL
run_test test03_invalid_ip.conf FAIL
run_test test04_invalid_port.conf FAIL
run_test test05_invalid_method.conf FAIL
run_test test06_duplicate_location.conf FAIL
run_test test07_negative_body.conf FAIL
run_test test08_invalid_error_code.conf FAIL
run_test test09_invalid_cgi.conf FAIL
run_test test10_invalid_autoindex.conf FAIL

echo "--------------------------------"
echo "Passed: $PASS_COUNT"
echo "Failed: $FAIL_COUNT"
