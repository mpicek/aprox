#ifndef EXPRESSION_HPP_
#define EXPRESSION_HPP_

#include <stack>
#include "distribution.hpp"
#include <set>

#define DEBUG_BUILD
#ifdef DEBUG_BUILD
#define DEBUG(x) std::cerr << x << std::endl
#define DEBUG2(x, y) std::cerr << x << " " << y << std::endl
#else
#define DEBUG(x)                                                               \
  do {                                                                         \
  } while (0)
#define DEBUG2(x, y)                                                           \
  do {                                                                         \
  } while (0)
#endif

std::set<char> operators = {
    '+', '-',
    '*', '/',
    '^',
    '~', 'n', 'b', 'u'
};

std::set<char> arithmetic_operators = {
    '+', '-',
    '*', '/',
    '^'
};

std::set<char> distribution_operators = {
    '~', 'n', 'b', 'u'
};

template <typename real>
class Token{

    std::unique_ptr<Distribution<real>> dist_ptr;
    real number;
    char op; // operator
    int priority;
    bool is_number;
    bool is_operator;
    bool is_distribution;
    bool error_occurred;

public:

    Token() : number(0), is_number(true), is_operator(false), is_distribution(false), error_occurred(false) {}

    Token(std::unique_ptr<Distribution<real>> ptr) : dist_ptr(std::move(ptr)), is_number(false), is_operator(false), is_distribution(true), error_occurred(false) {}

    Token(real number) : number(number), is_number(true), is_operator(false), is_distribution(false), error_occurred(false) {}

    Token(char op, int priority) : op(op), priority(priority), is_number(false), is_operator(true), is_distribution(false), error_occurred(false) {}

    void print(){
        if(is_number){
            std::cerr << number << std::endl;
            return;
        }
        if(is_operator){
            std::cerr << op << std::endl;
            return;
        }
        if(is_distribution){
            dist_ptr->print(std::cerr, -1);
        }
    }

    /**
     * Gets two tokens and operation (+ other info) and returns Token<real>,
     * where error_occurred is set when an error occurred.
     */
    static Token<real> operation(Token<real>& left, Token<real>& right, char operation, real bin_size, real std_deviation_quotient){

        if(distribution_operators.find(operation) != distribution_operators.end() && 
           left.is_number && right.is_number){
            
            Token<real> result(std::make_unique(Distribution<real>(operation, left, right, bin_size, std_deviation_quotient)));
            if(result.dist_ptr->error_occurred) result.error_occurred = true;
            return result;
            
        }
        else if(arithmetic_operators.find(operation) != arithmetic_operators.end() &&
                !left.is_operator && !right.is_operator){

            //TODO ARITHMETIC OPERATION
            return result;

        }
        else{
            Token<real> result(0);
            result.error_occurred = true;
            return result;
        }
    }

};

template <typename real>
class Expression{

    std::stack<Token<real>> prefix_stack;
    real bin_size;
    real std_deviation_quotient;

public:

    Expression(){}

    Expression(real bin_size, real std_deviation_quotient) : bin_size(bin_size), std_deviation_quotient(std_deviation_quotient){}

    void print_stack(){
        while(!prefix_stack.empty()){
            prefix_stack.top().print();
            prefix_stack.pop();
        }
    }

    bool process_operator(char op){
        if(prefix_stack >= 2){
            Token<real> right = prefix_stack.top();
            prefix_stack.pop();
            Token<real> left = prefix_stack.top();
            prefix_stack.pop();

            Token<real>::operation(left, right, op, bin_size, std_deviation_quotient);
        }
        else
            return false;

        return true;
    }

    bool evaluate_postfix_input(std::stringstream& input){
        std::string input_string;
        std::getline(input, input_string);
        input_string.erase(std::remove_if(input_string.begin(), input_string.end(), ::isspace), input_string.end());
        std::stringstream new_number;
        double number;

        int state = 1;

        for(long unsigned int i = 0; i < input_string.length(); i++){
            if(state == 1){
                if(std::isdigit(input_string[i]) || input_string[i] == '.'){
                    new_number << input_string[i];
                    state++;
                    continue;
                }
                else if(operators.find(input_string[i]) != operators.end()){
                    prefix_stack.push(Token<real>(input_string[i], 0)); //TODO
                    continue;
                }
                else return false;
            }
            if(state == 2){
                if(std::isdigit(input_string[i]) || input_string[i] == '.'){
                    new_number << input_string[i];
                    if(input_string[i] == '.') state++;
                    continue;
                }
                else if(operators.find(input_string[i]) != operators.end()){
                    new_number >> number;
                    new_number.clear();
                    prefix_stack.push(Token<real>(number));
                    prefix_stack.push(Token<real>(input_string[i], 0)); //TODO
                    state = 1;
                    continue;
                }
                else return false;
            }
            if(state == 3){
                if(std::isdigit(input_string[i])){
                    new_number << input_string[i];
                    continue;
                }
                else if(operators.find(input_string[i]) != operators.end()){
                    new_number >> number;
                    new_number.clear();
                    prefix_stack.push(Token<real>(number));
                    prefix_stack.push(Token<real>(input_string[i], 0)); //TODO
                    state = 1;
                    continue;
                }
                else return false;
            }
        }
        // reads the last number
        if(state > 1){
            new_number >> number;
            // std::cout << number << std::endl;
            prefix_stack.push(Token<real>(number));
        }
        return true;
    }

    bool evaluate_infix_input(std::stringstream& buffer){

        // real from, to;
        // buffer >> from >> to;
        //result = Distribution<real>('n', 5, 20, bin_size, STANDARD_DEVIATION_QUOTIENT);

        return true;
    }
};


#endif