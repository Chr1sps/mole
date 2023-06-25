#!/usr/bin/bash

out="output.info"
input_dir="."
out_dir="./html"

cd "$(dirname "${BASH_SOURCE[0]}")/.." &>/dev/null || return
lcov --capture --directory $input_dir --output-file $out --rc lcov_branch_coverage=1 --no-external
lcov --zerocounters --directory $input_dir
genhtml --output-directory $out_dir $out --rc gentml_branch_coverage=1
