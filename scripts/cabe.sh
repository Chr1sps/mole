#!/bin/bash

cc_limit=10
cc_margin=2

stmt_limit=15
stmt_margin=3

result=0
printed=0

red='\033[38;5;160m' # ANSI escape sequence for red color
yellow='\033[38;5;214m'
reset='\033[0m' # ANSI escape sequence to reset color

cd "$(dirname "${BASH_SOURCE[0]}")/.." &>/dev/null || return
output=$(pmccabe src/*.cpp | sort -nr -k1,1 -k2,2 -k3,3)
warning_lines=""
clear_lines=""

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
print_header() {

    if [ $printed -eq 0 ]; then
        echo -e "ModCC\tTradCC\tStmts\tLine\tCount"
        printed=1
    fi
}

while IFS= read -r line; do
    IFS=$'\t' read -r col1 col2 col3 col4 col5 col6 <<<"$line"

    if [[ $col1 -gt $cc_limit || $col2 -gt $cc_limit || $col3 -gt $stmt_limit ]]; then
        if [ $result -eq 0 ]; then
            result=1
        fi

    fi
    if
        [[ "$col1" -gt $((cc_limit - cc_margin)) ||
            "$col2" -gt $((cc_limit - cc_margin)) ||
            "$col3" -gt $((stmt_limit - stmt_margin)) ]]
    then
        ncol1=$(color_column "$col1" "$cc_limit" "$cc_margin")
        ncol2=$(color_column "$col2" "$cc_limit" "$cc_margin")
        ncol3=$(color_column "$col3" "$stmt_limit" "$stmt_margin")
        if
            [[ "$col1" -gt cc_limit ||
                "$col2" -gt cc_limit ||
                "$col3" -gt stmt_limit ]]
        then
            print_header
            echo -e "${ncol1}\t${ncol2}\t${ncol3}\t${col4}\t${col5}\t${col6}"
        else
            warning_lines+="${ncol1}\t${ncol2}\t${ncol3}\t${col4}\t${col5}\t${col6}\n"
        fi

    else
        clear_lines+="$line\n"
    fi

done <<<"$output"
print_header
echo -en "$warning_lines"
if [ "$1" = "all" ]; then
    echo -en "$clear_lines"
fi
exit $result
