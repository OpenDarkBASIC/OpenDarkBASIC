#!/bin/bash

MEMORY=150
TESTCASES=../../afl/testcases
DICTIONARY=../../afl/dictionary/dbp.dict
FINDINGS=afl-findings
ODBC=./odbc

afl-fuzz -M fuzzer1 -m "$MEMORY" -i "$TESTCASES" -x "$DICTIONARY" -o "$FINDINGS" "$ODBC" -n --parse-dba @@ &
MASTERPID=$!
for i in $(seq 2 $(nproc))
do
    afl-fuzz -S "fuzzer$i" -m "$MEMORY" -i "$TESTCASES" -x "$DICTIONARY" -o "$FINDINGS" "$ODBC" -n --parse-dba @@ 2>&1 > /dev/null &
done
wait $MASTERPID

