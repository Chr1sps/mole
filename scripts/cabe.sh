#!/bin/bash

cc_error=10
cc_warning=8

stmt_error=15
stmt_warning=12

line_error=35
line_warning=25

result=0
printed=0

red='\033[38;5;160m' # ANSI escape sequence for red color
yellow='\033[38;5;214m'
reset='\033[0m' # ANSI escape sequence to reset color

cd "$(dirname "${BASH_SOURCE[0]}")/.." &>/dev/null || return

if ! command -v pmccabe >/dev/null; then
    echo "\`pmccabe\` command not found. Exiting."
    exit
fi

output=$(pmccabe -c src/*.cpp | sort -nr -k1,1 -k2,2 -k3,3)
warning_lines=""
clear_lines=""

color_column() {
    col="$1"
    error="$2"
    warning="$3"

    if [ "$col" -gt "$error" ]; then
        col="${red}${col}${reset}"
    elif [ "$col" -gt "$warning" ]; then
        col="${yellow}${col}${reset}"
    fi

    printf '%s' "$col"
}
print_header() {

    if [ $printed -eq 0 ]; then
        echo -e "ModCC\tTradCC\tStmts\tLines"
        printed=1
    fi
}

while IFS= read -r line; do
    IFS=$'\t' read -r col1 col2 col3 _ col5 col6 <<<"$line"

    if [[ $col1 -gt $cc_error || $col2 -gt $cc_error || $col3 -gt $stmt_error || $col5 -gt $line_error ]]; then
        if [ $result -eq 0 ]; then
            result=1
        fi

    fi
    if
        [[ "$col1" -gt $cc_warning ||
            "$col2" -gt $cc_warning ||
            "$col3" -gt $stmt_warning ||
            "$col5" -gt $line_warning ]]
    then
        ncol1=$(color_column "$col1" "$cc_error" "$cc_warning")
        ncol2=$(color_column "$col2" "$cc_error" "$cc_warning")
        ncol3=$(color_column "$col3" "$stmt_error" "$stmt_warning")
        ncol5=$(color_column "$col5" "$line_error" "$line_warning")
        if
            [[ "$col1" -gt cc_error ||
                "$col2" -gt cc_error ||
                "$col3" -gt stmt_error ||
                "$col5" -gt line_error ]]
        then
            print_header
            echo -e "${ncol1}\t${ncol2}\t${ncol3}\t${ncol5}\t${col6}"
        else
            warning_lines+="${ncol1}\t${ncol2}\t${ncol3}\t${ncol5}\t${col6}\n"
        fi

    else
        clear_lines+="${col1}\t${col2}\t${col3}\t${col5}\t${col6}\n"
    fi

done <<<"$output"
if [ -n "$warning_lines" ]; then
    print_header
else
    echo "No problems found."
fi
echo -en "$warning_lines"
if [ "$1" = "all" ]; then
    echo -en "$clear_lines"
fi
exit $result
