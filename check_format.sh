#!/usr/bin/bash
! find . \( -name "*.cpp" -o -name "*.hpp" \) -not -path "./build/*" -exec clang-format-15 -output-replacements-xml {} \; | grep -c "<replacement "
