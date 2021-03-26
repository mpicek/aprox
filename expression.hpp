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

/**
 * Represents distribution, number or an operator
 */
template <typename real>
class Token{

    // dist_ptr because the Token needs to have one certain size
    std::unique_ptr<Distribution<real>> dist_ptr;
    real number;
    char op; // operator
    int priority; // priority of operator

    // flags indicating what type is stored in the Token
    bool is_number;
    bool is_operator;
    bool is_distribution;
    

public:

    bool get_is_operator(){
        return is_operator;
    }

    bool error_occurred; // public flag indicating that something wrong happened

    Token() : number(0), is_number(true), is_operator(false), is_distribution(false), error_occurred(false) {}

    Token(std::unique_ptr<Distribution<real>> ptr) : dist_ptr(std::move(ptr)), is_number(false),
                                                    is_operator(false), is_distribution(true){
        error_occurred = dist_ptr->error_occurred;
    }

    Token(real number) : number(number), is_number(true),
                        is_operator(false), is_distribution(false),
                        error_occurred(false) {}

    Token(char op, int priority) : op(op), priority(priority), is_number(false),
                                    is_operator(true), is_distribution(false),
                                    error_occurred(false) {}


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

    /**
     * MOVE CONSTRUCTOR
     */
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

    /**
     * Prints a token.
     */
    void print(std::ostream& ostr, int num_of_result_bins){
        if(is_number){
            ostr << number << std::endl;
            return;
        }
        if(is_operator){
            ostr << op << std::endl;
            return;
        }
        if(is_distribution){
            dist_ptr->print(ostr, num_of_result_bins);
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
        // Perform an arithmetic operation
        else if(arithmetic_operators.find(operation) != arithmetic_operators.end()){
            if(left.is_number && right.is_number){

                // Define macro so that we don't need to copy code
                #define NUMBER_OPERATIONS(OPERATOR) \
                    if(operation == #OPERATOR[0]){ \
                        DEBUG2("Operace (num a num", #OPERATOR); \
                        if((#OPERATOR[0] == '/' && right.number == 0) || right.error_occurred || left.error_occurred){ \
                            Token<real> result(0); \
                            result.error_occurred = true; \
                            return result; \
                        } \
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

                MIXED_OPERATIONS(+);
                MIXED_OPERATIONS(-);
                MIXED_OPERATIONS(*);
                MIXED_OPERATIONS(/);
            }                
            else{ // left or right is an operator => error
                Token<real> result(0);
                result.error_occurred = true;
                return result;
            }
        }
        else{ // left or right is an operator => error
            Token<real> result(0);
            result.error_occurred = true;
            return result;
        }
        return true;
    }

};

/**
 * Class representing an expression.
 * Can parse and evaluate the expression.
 * In general returns 0 on failure and 1 on success.
 */
template <typename real>
class Expression{

    std::stack<Token<real>> prefix_stack;
    real bin_size;
    real std_deviation_quotient;

public:

    Expression(){}

    Expression(real bin_size, real std_deviation_quotient) : bin_size(bin_size), 
                                std_deviation_quotient(std_deviation_quotient){}

    /**
     * Prints stack and erases it (used for debugging)
     */
    void print_stack(){
        while(!prefix_stack.empty()){
            prefix_stack.top().print();
            prefix_stack.pop();
        }
    }

    /**
     * Processes operator with 2 tokens on the top of the stack
     * Returns bool (success)
     */
    bool process_operator(char op){

        // operators are binary
        if(prefix_stack.size() >= 2){
            Token<real> right(std::move(prefix_stack.top()));
            prefix_stack.pop();
            Token<real> left(std::move(prefix_stack.top()));
            prefix_stack.pop();

            // perform operation
            Token<real> result = Token<real>::operation(std::move(left), std::move(right), op, bin_size, std_deviation_quotient);
            prefix_stack.emplace(std::move(result));
        }
        else
            return false;

        return true;
    }

    /**
     * Parsing postfix based on states.
     * Description in program documentation.
     * Input: stringstream
     * Returns: bool (success)
     */
    bool parse_postfix_input(std::stringstream& input){
        std::string input_string;
        std::getline(input, input_string);
        std::stringstream new_number;
        real number;

        int state = 1;

        for(long unsigned int i = 0; i < input_string.length(); i++){
            // DEBUG2("nacitame ", input_string[i]);
            // DEBUG2("Size: ", prefix_stack.size());
            // DEBUG("Top: ");
            // // if(!prefix_stack.empty()) prefix_stack.top().print();
            // DEBUG("-------");

            if(state == 1){ // first state - nothing read
                if(std::isdigit(input_string[i]) || input_string[i] == '.'){
                    new_number << input_string[i];
                    state++;
                    continue;
                }
                else if(operators.find(input_string[i]) != operators.end()){

                    if(!process_operator(input_string[i])) return false;
                    continue;
                }
                else if(input_string[i] == ' '){
                    continue;
                }
                else return false;
            }
            // second state - in process of reading a number,
            //                no decimal point encountered yet
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
            // third state - in process of reading a number,
            //               a decimal point encountered
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
        // if we started to read a number, gets the last number
        if(state > 1){
            new_number >> number;
            prefix_stack.emplace(number);
        }
        return true;
    }

    bool parse_infix_input(std::stringstream& buffer){

        return true;
    }

    /**
     * Prints result (that is the last Token in the stack).
     */
    bool print_result(std::ostream& ostr, int num_of_result_bins){

        // In the stack there has to be just one Token and it has to be
        // a distribution or a number
        if(prefix_stack.size() == 1){
            if(prefix_stack.top().get_is_operator()) return false;
            prefix_stack.top().print(ostr, num_of_result_bins);
        }
        else{
            return false;
        }
        return true;
    }
};


#endif