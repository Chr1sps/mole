#!/bin/sh
cd "$(dirname "$0")/.." 2>/dev/null 1>&2 || return
(! find . \( -name "*.cpp" -o -name "*.hpp" \) -not -path "./build/*" \
   -exec clang-format -output-replacements-xml {} \; |
   grep -c "<replacement ") >/dev/null
