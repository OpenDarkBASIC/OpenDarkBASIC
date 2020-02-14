#!/bin/sh
cd build/odb-compiler || echo "Error: Please run this script from the project's root directory as ./scripts/valgrind-linux.sh"

echo "Started valgrind..."
valgrind --num-callers=50 \
	--leak-resolution=high \
	--leak-check=full \
	--track-origins=yes \
	--time-stamp=yes \
	./odbc_tests -- --test 2>&1 | tee ../../odbc_tests_grind.out
cd .. && cd ..
