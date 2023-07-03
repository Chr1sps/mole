#!/usr/bin/bash
cd "$(dirname "${BASH_SOURCE[0]}")/.." &>/dev/null || return
(! find . \( -name "*.cpp" -o -name "*.hpp" \) -not -path "./build/*" -exec clang-format-15 -output-replacements-xml {} \; | grep -c "<replacement ") >/dev/null
