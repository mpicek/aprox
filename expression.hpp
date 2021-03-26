#ifndef EXPRESSION_HPP_
#define EXPRESSION_HPP_

#include <stack>
#include "distribution.hpp"
#include <set>

// #define DEBUG_BUILD
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

// TODO dat Token do jinyho filu

template <typename real>
class Token{

    std::unique_ptr<Distribution<real>> dist_ptr;
    real number;
    char op; // operator
    int priority;
    bool is_number;
    bool is_operator;
    bool is_distribution;
    

public:

    bool error_occurred;

    Token() : number(0), is_number(true), is_operator(false), is_distribution(false), error_occurred(false) {}

    Token(std::unique_ptr<Distribution<real>> ptr) : dist_ptr(std::move(ptr)), is_number(false), is_operator(false), is_distribution(true){
        error_occurred = dist_ptr->error_occurred;
    }

    Token(real number) : number(number), is_number(true), is_operator(false), is_distribution(false), error_occurred(false) {}

    Token(char op, int priority) : op(op), priority(priority), is_number(false), is_operator(true), is_distribution(false), error_occurred(false) {}


    /**
     * MOVE ASSIGNMENT
     */
    Token<real>& operator=(Token<real>&& second){
        DEBUG("Assignment called");
        if(&second == this)
            return *this;

        dist_ptr = std::move(second.dist_ptr);
        number = second.number;
        op = second.op;
        priority = second.priority;
        is_number = second.is_number;
        is_operator = second.is_operator;
        is_distribution = second.is_distribution;
        error_occurred = second.error_occurred;
        
        return *this;
    }

    Token(Token<real>&& second){
        DEBUG("Move constructor called");
        
        dist_ptr = std::move(second.dist_ptr);
        number = second.number;
        op = second.op;
        priority = second.priority;
        is_number = second.is_number;
        is_operator = second.is_operator;
        is_distribution = second.is_distribution;
        error_occurred = second.error_occurred;

    }

    void print(){
        if(is_number){
            std::cerr << "Token je Cislo " << number << std::endl;
            return;
        }
        if(is_operator){
            std::cerr << "Token je operator: " << op << std::endl;
            return;
        }
        if(is_distribution){
            std::cerr << "Token je distribuce: " << std::endl;
            dist_ptr->print(std::cerr, 20);
        }
    }

    /**
     * Gets two tokens and operation (+ necessary) other info) and returns Token<real>,
     * where error_occurred is set when an error occurred.
     * 
     * For distribution operators create a distribution.
     * 
     * Otherwise we perform arithmetic operations.
     * 
     * Number op Number ... special case
     * Otherwise we just perform the operation
     * 
     * Returns a Token
     */
    static Token<real> operation(Token<real>&& left, Token<real>&& right, char operation, real bin_size, real std_deviation_quotient){
        DEBUG2(left.number, right.number);
        // Create a distribution from 2 numbers (from .. to)
        if(distribution_operators.find(operation) != distribution_operators.end() && 
           left.is_number && right.is_number){
            
            Token<real> result(std::make_unique<Distribution<real>>(operation, left.number, right.number, bin_size, std_deviation_quotient));
            if(result.dist_ptr->error_occurred) result.error_occurred = true;
            return result;
            
        }
        else if(arithmetic_operators.find(operation) != arithmetic_operators.end()){
            if(left.is_number && right.is_number){

                #define NUMBER_OPERATIONS(OPERATOR) \
                    if(operation == #OPERATOR[0]){ \
                    DEBUG2("Operace (num a num", #OPERATOR); \
                        if((#OPERATOR[0] == '/' && right.number == 0) || right.error_occurred || left.error_occurred){ \
                            DEBUG("TADYYYYYYYYYYY"); \
                            Token<real> result(0); \
                            result.error_occurred = true; \
                            return result; \
                        } \
                        DEBUG2(left.number, right.number); \
                        Token<real> result(left.number OPERATOR right.number); \
                        return result; \
                    } 

                NUMBER_OPERATIONS(+);
                NUMBER_OPERATIONS(-);
                NUMBER_OPERATIONS(*);
                NUMBER_OPERATIONS(/);
            }
            else if((left.is_number || left.is_distribution) && (right.is_number || right.is_distribution)){

                #define MIXED_OPERATIONS(OPERATOR) \
                    \
                    if(operation == #OPERATOR[0]){ \
                        DEBUG2("Operace (num a dist) ", #OPERATOR); \
                        if(right.error_occurred || left.error_occurred){ \
                            Token<real> result(0); \
                            result.error_occurred = true; \
                            return result; \
                        } \
                        Token<real> result; \
                        if(left.is_distribution && right.is_distribution) \
                            result = std::make_unique<Distribution<real>>(*left.dist_ptr OPERATOR *right.dist_ptr); \
                        else if(left.is_distribution && right.is_number) \
                            result = std::make_unique<Distribution<real>>(*left.dist_ptr OPERATOR right.number); \
                        else \
                            result = std::make_unique<Distribution<real>>(left.number OPERATOR *right.dist_ptr); \
                        if(result.dist_ptr->error_occurred) result.error_occurred = true; \
                        return result; \
                    } 
                    /*
                    if(left.is_distribution && right.is_distribution) \
                            Token<real> result(std::make_unique<Distribution<real>>(*left.dist_ptr OPERATOR *right.dist_ptr)); \
                            Token<real> result(std::make_unique(Distribution<real>(*left.dist_ptr OPERATOR *right.dist_ptr))); \
                        else if(left.is_distribution && right.is_number) \
                            Token<real> result(std::make_unique(Distribution<real>(*left.dist_ptr OPERATOR right.number))); \
                        else \
                            Token<real> result(std::make_unique(Distribution<real>(left.number OPERATOR *right.dist_ptr))); \
                        if(result.dist_ptr->error_occurred) result.error_occurred = true; \
                        return result; \
                    } 
                    */

                MIXED_OPERATIONS(+);
                MIXED_OPERATIONS(-);
                MIXED_OPERATIONS(*);
                MIXED_OPERATIONS(/);
            }                
            else{
                Token<real> result(0);
                result.error_occurred = true;
                return result;
            }
        }
        else{
            Token<real> result(0);
            result.error_occurred = true;
            return result;
        }
        return true;
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
        if(prefix_stack.size() >= 2){
            // prefix_stack.top().print();
            Token<real> right(std::move(prefix_stack.top()));
            prefix_stack.pop();
            // prefix_stack.top().print();
            Token<real> left(std::move(prefix_stack.top()));
            prefix_stack.pop();

            // left.print();
            // right.print();
            Token<real> result = Token<real>::operation(std::move(left), std::move(right), op, bin_size, std_deviation_quotient);
            prefix_stack.emplace(std::move(result));
        }
        else
            return false;

        return true;
    }

    bool parse_postfix_input(std::stringstream& input){
        DEBUG("TU");
        std::string input_string;
        std::getline(input, input_string);
        // input_string.erase(std::remove_if(input_string.begin(), input_string.end(), ::isspace), input_string.end());
        std::stringstream new_number;
        double number;

        int state = 1;

        for(long unsigned int i = 0; i < input_string.length(); i++){
            DEBUG2("nacitame ", input_string[i]);
            DEBUG2("Size: ", prefix_stack.size());
            DEBUG("Top: ");
            // if(!prefix_stack.empty()) prefix_stack.top().print();
            DEBUG("-------");
            // if(input_string[i] == '/'){
            //     print_stack();
            //     return false;
            // }
            if(state == 1){
                if(std::isdigit(input_string[i]) || input_string[i] == '.'){
                    new_number << input_string[i];
                    state++;
                    continue;
                }
                else if(operators.find(input_string[i]) != operators.end()){

                    if(!process_operator(input_string[i])) return false;

                    // prefix_stack.push(Token<real>(input_string[i], 0)); //TODO
                    continue;
                }
                else if(input_string[i] == ' '){
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
                    prefix_stack.emplace(number);
                    if(!process_operator(input_string[i])) return false;
                    // prefix_stack.push(Token<real>(input_string[i], 0)); //TODO
                    state = 1;
                    continue;
                }
                else if(input_string[i] == ' '){
                    new_number >> number;
                    new_number.clear();
                    prefix_stack.emplace(number);
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
                    prefix_stack.emplace(number);
                    if(!process_operator(input_string[i])) return false;
                    // prefix_stack.push(Token<real>(input_string[i], 0)); //TODO
                    state = 1;
                    continue;
                }
                else if(input_string[i] == ' '){
                    new_number >> number;
                    new_number.clear();
                    prefix_stack.emplace(number);
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
            prefix_stack.emplace(number);
        }
        return true;
    }

    bool parse_infix_input(std::stringstream& buffer){

        // real from, to;
        // buffer >> from >> to;
        //result = Distribution<real>('n', 5, 20, bin_size, STANDARD_DEVIATION_QUOTIENT);

        return true;
    }

    bool evaluate(){
        return true;
    }
};


#endif