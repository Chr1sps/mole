#!/bin/sh

out="output.info"
input_dir="."
out_dir="./html"

cd "$(dirname "$0")/.." 2>/dev/null 1>&2 || return
lcov --capture --directory "$input_dir" --output-file $out --rc lcov_branch_coverage=1 --no-external &&
    lcov --zerocounters --directory $input_dir &&
    genhtml --output-directory $out_dir $out --rc genhtml_branch_coverage=1
