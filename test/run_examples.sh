#!/bin/sh
set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$ROOT_DIR"

mkdir -p test/bin test/tmp test/tmp/home

gcc -Wall -Wextra -std=c11 test/shift_numbers.c -o test/bin/shift_numbers
gcc -Wall -Wextra -std=c11 test/sum_pairs.c -o test/bin/sum_pairs
gcc -Wall -Wextra -std=c11 test/wait_and_print.c -o test/bin/wait_and_print

: > test/tmp/shifted.txt
: > test/tmp/pipeline.txt
: > test/tmp/session.out
: > test/tmp/expected_pipeline.txt
: > test/tmp/expected_shifted.txt

printf "5\n14\n32\n" > test/tmp/expected_pipeline.txt
printf "2\n3\n6\n8\n11\n21\n2\n3\n6\n8\n11\n21\n" > test/tmp/expected_shifted.txt

HOME="$ROOT_DIR/test/tmp/home" ./dshell > test/tmp/session.out <<'COMMANDS'
help
pwd
setenv DSHELL_MODE demo
printenv DSHELL_MODE
unsetenv DSHELL_MODE
./test/bin/shift_numbers < test/fixtures/numbers.txt > test/tmp/shifted.txt
./test/bin/shift_numbers < test/fixtures/numbers.txt | ./test/bin/sum_pairs > test/tmp/pipeline.txt
./test/bin/shift_numbers < test/fixtures/numbers.txt >> test/tmp/shifted.txt
./test/bin/wait_and_print &
jobs
sleep 2
jobs
history
exit
COMMANDS

diff -u test/tmp/expected_pipeline.txt test/tmp/pipeline.txt
diff -u test/tmp/expected_shifted.txt test/tmp/shifted.txt

grep -q "DSHELL_MODE" test/tmp/session.out
grep -q "running ./test/bin/wait_and_print &" test/tmp/session.out
grep -q "done(0) ./test/bin/wait_and_print &" test/tmp/session.out

echo "All DShell README example tests passed."
