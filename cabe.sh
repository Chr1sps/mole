#!/bin/bash

cc_limit=10
cc_margin=2

stmt_limit=15
stmt_margin=3

result=0
printed=0

red='\033[0;31m' # ANSI escape sequence for red color
yellow='\033[38;5;214m'
reset='\033[0m' # ANSI escape sequence to reset color

output=$(pmccabe src/*.cpp)

color_column() {
    col="$1"
    limit="$2"
    margin="$3"

    if [ "$col" -gt "$limit" ]; then
        col="${red}${col}${reset}"
    elif [ "$col" -gt "$((limit - margin))" ]; then
        col="${yellow}${col}${reset}"
    fi

    printf '%s' "$col"
}

while IFS= read -r line; do
    IFS=$'\t' read -r col1 col2 col3 col4 col5 col6 <<<"$line"

    if [[ $col1 -gt $cc_limit || $col2 -gt $cc_limit || $col3 -gt $stmt_limit ]]; then
        if [ $result -eq 0 ]; then
            result=1
        fi

    fi
    if [[ $col1 -gt $((cc_limit - cc_margin)) || $col2 -gt $((cc_limit - cc_margin)) || $col3 -gt $((stmt_limit - stmt_margin)) ]]; then
        if [ $printed -eq 0 ]; then
            echo -e "ModCC\tTradCC\tStmts\tLine\tCount"
            printed=1
        fi

        col1=$(color_column "$col1" "$cc_limit" "$cc_margin")
        col2=$(color_column "$col2" "$cc_limit" "$cc_margin")
        col3=$(color_column "$col3" "$stmt_limit" "$stmt_margin")

        # Echo the line with highlighted exceeding values
        echo -e "${col1}\t${col2}\t${col3}\t${col4}\t${col5}\t${col6}"
    fi
done <<<"$output"
exit $result
