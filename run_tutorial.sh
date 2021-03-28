clearing=1

# "description" "input" "args"
run_example(){
    if [ "$clearing" -eq 1 ]
    then
        clear
    fi
    echo ""
    echo $1
    echo ""
    echo "Input: $2"
    echo "Command: ./aprox $3"
    echo "$2" | ./aprox $3
    read _
}

run_example "Print help for more info" " " "-h"
run_example "Simple prefix notation" "2.5 2 * 8 / 4 *" "-p"
run_example "Simple prefix notation with normal distribution and use `-r` to have a nice and dense graph" "10 20 ~ 8 4 / *" "-p -r 10"
run_example "Simple prefix notation with normal and uniform distributions. WARNING: \
DISTRIBUTIONS DON'T ALWAYS LOOK PERFECT DUE TO ROUNDING ERROR." "10 30 ~ 20 50 u +" "-p -r 20"
run_example "Simple infix notation" "(5 + 3 * 2 - 1) / 2" ""
run_example "No problem.." "-(-(-(-(-1))))" ""
run_example "Works great!" "-(-(-(-1)))" ""
run_example "Weird inputs!" "10   +     (-3)*(1/2)" ""
run_example "Handles errors.." "5 + 3.3.3" ""
run_example "Handles errors.." "5 + / (1 - 1)" ""
run_example "Handles errors.." "5 +* 4" ""
run_example "Handles errors.." "10 ~ 5" ""
run_example "Handles errors.." "10 u 5" ""
run_example "Distribution signs have the highest priority (but parentheses have even higher)" "5 ~ 10 * 2 u 30" ""
run_example "Can't divide by zero in distributions.." "5 ~ 15 / 0 ~ 2" ""
run_example "Can't divide by zero in distributions.." "100 / (-2) ~ 0" ""
run_example "Can't divide by zero in distributions.." "5 ~ 15 / (-2) ~ 2" ""
run_example "Handles errors.." "10 ~ 5" ""
run_example "This is how it should have been written" "(-5) ~ 10" "-r 15"
run_example "Sum with distributions" "0 ~ 10 + (-10) u 10 + (-10) ~ 20" ""
run_example "Multiplication with distributions" "0 ~ 10 * (-10) u 0 * (-10) ~ 20" ""
run_example "Division with distributions - we need to set a lower bin_size for precision" "50 u 100 / 5 ~ 10" "-b 0.1"
run_example "Division with distributions - we need to set a lower bin_size for precision" "500 / 10 ~ 50" "-b 0.1"
run_example "Division with distributions - we need to set a lower bin_size for precision" "10 ~ 50 / 100" "-b 0.001"
run_example "Division with distributions - we need to set a lower bin_size for precision" "50 u 100 / 10 ~ 50 + 2 ~ 10" "-b 0.1"
run_example "Save into output file.." "10 ~ 30" "-o out.txt -r 20"
echo "Now printing file out.txt using `cat out.txt`:"
cat out.txt
rm out.txt
read key
echo "5 + 5 ~ 40" > in.txt
run_example "Reading from input file in.txt. The rounding error is clearly visible." "10 ~ 50" "-i in.txt -r 30"
rm in.txt
run_example "Reallife problem: How much will I earn this summer? Part-time jobs with no qualifications\
pay 110 ~ 150Kc. I will work 8 hours/day, 5 days a week. I want to work 3 weeks. How much \
will I earn? Also problems with rounding error. But a person still can see what is happening :)" "110 ~ 150 * 8 * 5 * 3" "" 
