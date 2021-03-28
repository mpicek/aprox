#include <iostream>
#include <unistd.h>
#include <sstream>
#include <map>
#include <cmath>
#include <fstream>
#include <string>
#include <boost/math/distributions/normal.hpp>

#include "distribution.hpp"
#include "expression.hpp"

#define NUM_OF_RESULT_BINS_DEFAULT 25
#define STANDARD_DEVIATION_QUOTIENT 2

// real is the type that represents the real number
template <typename real>
struct Parsed_arguments{
    real bin_size; // size of the bins in which the distributions are stored

    // number of bins into which the result will be stored
    int num_of_result_bins;

    bool postfix;

    bool help_flag;

    bool output_flag;
    char* output_file_name;

    bool input_flag;
    char* input_file_name;

    bool error_occurred;

    Parsed_arguments(): bin_size(1),
                        num_of_result_bins(NUM_OF_RESULT_BINS_DEFAULT),
                        postfix(false),
                        help_flag(false),
                        output_flag(false),
                        input_flag(false),
                        error_occurred(false) {}

};

/**
 * Parses arguments using getopt and returns Parsed_arguments<real> with
 * parsed arguments.
 */
template <typename real>
Parsed_arguments<real> parse_arguments(int argc, char **argv){
    int c;
    char* bin_size_char = nullptr;
    char* result_bins_char = nullptr;
    Parsed_arguments<real> args;

    // after argument : = it needs another argument
    // after argument :: = another argument is optional
    while((c = getopt(argc, argv, "i:o:b:r:ph")) != -1){
        bool s_in_switch = false;
        bool r_in_switch = false;
        switch (c){
            case 'i': // input will be loaded from a file
                args.input_flag = true;
                args.input_file_name = optarg; // filename has to be specified
                break;
            case 'o': // output will be printed to a file
                args.output_flag = true;
                args.output_file_name = optarg; // filename has to be specified
                break;
            case 'b': // setting the bin_size
                bin_size_char = optarg; // size of the bin has to follow
                s_in_switch = true;
                break;
            case 'p':
                args.postfix = true;
                break;
            case 'r': // how many bins to use during result presentation
                result_bins_char = optarg;
                r_in_switch = true;
                break;
            case 'h': // print help
                args.help_flag = true;
                // don't read other options, just print help and quit
                args.error_occurred = false;
                return args;
                break;
            default:
                std::cerr << "ERROR: WRONG ARGUMENT." << std::endl;
                args.error_occurred = true;
                return args;
                break;
        }

        // In switch you are unable to create new variables TODO why?? ... this is the solution
        if(s_in_switch){
            std::stringstream tmp(bin_size_char);
            if(!(tmp >> args.bin_size)){
                std::cout << "ERROR: UNABLE TO READ BIN SIZE, SETTING BIN_SIZE TO 1 (DEFAULT)." << std::endl;
                args.bin_size = 1;
            }
        }

        // In switch you are unable to create new variables TODO why?? ... this is the solution
        if(r_in_switch){
            std::stringstream tmp(result_bins_char);
            if(!(tmp >> args.num_of_result_bins)){
                std::cout << "ERROR: UNABLE TO READ NUMBER OF BINS IN RESULT, SETTING NUM_OF_RUSULT_BINS TO 100 (DEFAULT)." << std::endl;
                args.num_of_result_bins = 20;
            }
        }
    }

    // if there are other arguments
    if(optind < argc){
        std::cerr << "ERROR: WRONG ARGUMENT." << std::endl;
        args.error_occurred = true;
        return args;
    }

    args.error_occurred = false;
    return args;
}

/**
 * Reads input from either cin or a file. Saves it into the input_buffer.
 * Returns true on success, false on failure.
 */
template <typename real>
bool read_input(Parsed_arguments<real>& args, std::stringstream& input_buffer){

    if(args.input_flag){ // We put the whole file into the input_buffer
        std::ifstream input_file(args.input_file_name);

        // Check whether the file is correctly opened or not
        if(input_file.is_open()){
            input_buffer << input_file.rdbuf(); // Load the file into the input_buffer
            input_file.close();
        }
        else{
            std::cerr
            << "ERROR: UNABLE TO OPEN THE INPUT FILE "
            << args.input_file_name << std::endl;
            return false;
        }
    }
    else{ // If no file specified, read the input from stdin
        std::string input;
        std::getline(std::cin, input);
        input_buffer << input << std::endl;
    }
    return true;
}

/**
 * If print_help() printed help,
 * the method returns true and therefore the program quits.
 */
template <typename real>
bool print_help(Parsed_arguments<real>& args){
    if(args.help_flag){
        std::cout << "Usage: aprox [-i in] [-o out] [-s interval size]" << std::endl;
        std::cout << "    -i: input file, default: stdin" << std::endl;
        std::cout << "    -o: output file, default: stdout" << std::endl;
        std::cout << "    -p: read postfix notation, default: infix" << std::endl;
        std::cout << "    -b: bin_size - size of the bins in which the distributions are stored, default = 1" << std::endl;
        std::cout << "    -r: how many bins to use during result presentation, default = " << NUM_OF_RESULT_BINS_DEFAULT << std::endl;
        std::cout << "Distributions: " << std::endl;
        std::cout << "    - '~' of 'n' for normal distribution" << std::endl;
        std::cout << "    - 'u' for uniform distribution" << std::endl;
        return true;
    }
    return false;
}

/**
 * Prints the result.
 * Returns true on success, false on failure.
 */
template <typename real>
bool output(Parsed_arguments<real>& args, Expression<real>& expression){
    bool success = true;

    if(args.output_flag){
        std::ofstream out;
        out.open(args.output_file_name);

        if(out.is_open()){
            success = expression.print_result(out, args.num_of_result_bins);
            out.close();
            return success;
        }
        else{
            return false;
        }
    }
    else{
        success = expression.print_result(std::cout, args.num_of_result_bins);
        return success;
    }
}

/**
 * Computes the result from the expression.
 * Returns true on success, false on failure (division by zero for example).
 */
template <typename real>
bool compute(Parsed_arguments<real>& args, Expression<real>& expression, std::stringstream& input_buffer){
    if(args.postfix){
        if(!expression.parse_postfix_input(input_buffer)){
            std::cerr << "ERROR: PROBLEM DURING EVALUATION OCCURED - PROBABLY WHAT HAPPENED:" << std::endl;
            std::cerr << "     - DIVISION BY ZERO (BEWARE OF DISTRIBUTIONS WHICH INCLUDE ZERO)" << std::endl;
            std::cerr << "     - WRONG INPUT (ERROR IN FORMAT - NOT ENOUGH OPERANDS, TOO MANY OPERANDS, NO MATCHING PARENTHESES,.." << std::endl;
            return false;
        }
    }
    else{
        if(!expression.parse_infix_input(input_buffer)){
            std::cerr << "ERROR: PROBLEM DURING EVALUATION OCCURED - PROBABLY WHAT HAPPENED:" << std::endl;
            std::cerr << "     - DIVISION BY ZERO (BEWARE OF DISTRIBUTIONS WHICH INCLUDE ZERO)" << std::endl;
            std::cerr << "     - WRONG INPUT (ERROR IN FORMAT - NOT ENOUGH OPERANDS, TOO MANY OPERANDS, NO MATCHING PARENTHESES,.." << std::endl;
            return false;
        }
    }
    
    return true;
}

int main(int argc, char **argv){

    using real = double;

    Parsed_arguments<real> args = parse_arguments<real>(argc, argv);

    Expression<real> expression(args.bin_size, STANDARD_DEVIATION_QUOTIENT);
    std::stringstream input_buffer;
    
    if(args.error_occurred) return 1;
    if(print_help<real>(args)) return 0;
    if(!read_input<real>(args, input_buffer)) return 1;
    if(!compute<real>(args, expression, input_buffer)) return 1;
    if(!output<real>(args, expression)) return 1;

    return 0;
}