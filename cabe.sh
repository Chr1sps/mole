#!/bin/bash

cc_limit=10
stmt_limit=15
result=0
red='\033[0;31m' # ANSI escape sequence for red color
reset='\033[0m'  # ANSI escape sequence to reset color

output=$(pmccabe src/*.cpp)

while IFS= read -r line; do
    IFS=$'\t' read -r col1 col2 col3 col4 col5 col6 <<<"$line"

    if [[ $col1 -gt $cc_limit || $col2 -gt $cc_limit || $col3 -gt $stmt_limit ]]; then
        if [ $result -eq 0 ]; then
            echo -e "ModCC\tTradCC\tStmts\tLine\tCount"
            result=1
        fi
        if [ "$col1" -gt 10 ]; then
            col1="${red}${col1}${reset}"
        fi
        if [ "$col2" -gt 10 ]; then
            col2="${red}${col2}${reset}"
        fi
        if [ "$col3" -gt 15 ]; then
            col3="${red}${col3}${reset}"
        fi

        # Echo the line with highlighted exceeding values
        echo -e "${col1}\t${col2}\t${col3}\t${col4}\t${col5}\t${col6}"
    fi
done <<<"$output"
exit $result
