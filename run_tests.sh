#!/usr/bin/env bash

# params:
#   - prefix input
#   - expected output
test_prefix() {
    echo "---------------------------------------------------------------------"
    echo "Input for prefix test is: $1"
    echo "Expected output: $2"
    echo "$1" | ./main -p
    echo "Return code is: $?"
}

echo "################################################ NUMBERS ##################################################"
test_prefix "2.58 2 * 8 / 3.5 *" "2.2575"
test_prefix "0.2 .2 *" "0.04"
test_prefix "0.35" "0.35"
test_prefix "5       3+1./" "8"
test_prefix "5 5 + 5" "ERROR"
test_prefix "0.5.3 2 *" "ERROR"
test_prefix "3 5 * +" "ERROR"

echo "################################################ DISTRIBUTIONS ##################################################"
test_prefix "0 100 ~ 3 * 10 -" "-10 ... 290"
test_prefix "5 20 u 0 10 ~ *" "0 ... 200"
test_prefix "0 4 u 2 - 10 *" "0 ... 2"
test_prefix "50 10 u 5 10 u /" "10"
test_prefix "8 ~ 20 0 /" "ERROR"
test_prefix "7 5 20 ~ 20 - /" "ERROR"