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

// Returns true on success, false on failure.
// TODO: pridat const

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

// real is the type that represents the real number
template <typename real>
class Program{

    int argc;
    char **argv;

    real bin_size; // size of the bins in which the distributions are stored
    int num_of_result_bins;

    bool postfix;

    bool help_flag;

    bool output_flag;
    char* output_file_name;

    bool input_flag;
    char* input_file_name;

    Distribution<real> result; // probably unnecessary

    

    /**
     * 
     */
    bool save_output(){

        return true;
    }

    /**
     * 
     */
    void print_output(){
        result.print();
    }

    bool parse_postfix_input(std::stringstream& buffer){

        return true;
    }


    bool parse_infix_input(std::stringstream& buffer){

        float from, to;
        buffer >> from >> to;
        result = Distribution<real>('n', from, to, bin_size);

        return true;
    }


public:
    Program(int argc, char **argv) : argc(argc),
                                     argv(argv),
                                     bin_size(1),
                                     num_of_result_bins(NUM_OF_RESULT_BINS_DEFAULT),
                                     postfix(false),
                                     help_flag(false),
                                     output_flag(false),
                                     input_flag(false) {}


    /**
     * Parses arguments using getopt.
     * Returns true on success, false on failure.
     */
    bool parse_arguments(){
        int c;
        char* bin_size_char = nullptr;
        char* result_bins_char = nullptr;

        // after argument : = it needs another argument
        // after argument :: = another argument is optional
        while((c = getopt(argc, argv, "i:o:b:h")) != -1){
            bool s_in_switch = false;
            bool r_in_switch = false;
            switch (c){
                case 'i': // input will be loaded from a file
                    input_flag = true;
                    input_file_name = optarg; // filename has to be specified
                    break;
                case 'o': // output will be printed to a file
                    output_flag = true;
                    output_file_name = optarg; // filename has to be specified
                    break;
                case 'b': // setting the bin_size
                    bin_size_char = optarg; // size of the bin has to follow
                    s_in_switch = true;
                    break;
                case 'p':
                    postfix = true;
                    break;
                case 'r': // how many bins to use during result presentation
                    result_bins_char = optarg;
                    r_in_switch = true;
                    break;
                case 'h': // print help
                    help_flag = true;
                    // don't read other options, just print help and quit
                    return true;
                    break;
                default:
                    std::cerr << "ERROR: WRONG ARGUMENT." << std::endl;
                    return false;
                    break;
            }

            // In switch you are unable to create new variables TODO why?? ... this is the solution
            if(s_in_switch){
                std::stringstream tmp(bin_size_char);
                if(!(tmp >> bin_size)){
                    std::cout << "ERROR: UNABLE TO READ BIN SIZE, SETTING BIN_SIZE TO 1 (DEFAULT)." << std::endl;
                    bin_size = 1;
                }
            }

            // In switch you are unable to create new variables TODO why?? ... this is the solution
            if(r_in_switch){
                std::stringstream tmp(result_bins_char);
                if(!(tmp >> num_of_result_bins)){
                    std::cout << "ERROR: UNABLE TO READ NUMBER OF BINS IN RESULT, SETTING NUM_OF_RUSULT_BINS TO 100 (DEFAULT)." << std::endl;
                    num_of_result_bins = 1;
                }
            }
        }

        // if there are other arguments
        if(optind < argc){
            std::cerr << "ERROR: WRONG ARGUMENT." << std::endl;
            return false;
        }

        return true;
    }

    /**
     * 
     * Returns true on success, false on failure.
     */
    bool read_input(){
        std::stringstream buffer; // We put the whole file into the buffer

        if(input_flag){
            std::ifstream input_file(input_file_name);

            // Check whether the file is correctly opened or not
            if(input_file.is_open()){
                buffer << input_file.rdbuf(); // Load the file into the buffer
                input_file.close();
            }
            else{
                std::cerr
                << "ERROR: UNABLE TO OPEN THE INPUT FILE "
                << input_file_name << std::endl;
                return false;
            }
        }
        else{ // If no file specified, read the input from stdin
            std::string input;
            std::getline(std::cin, input);
            buffer << input << std::endl;
        }

        if(postfix) return parse_postfix_input(buffer);
        else return parse_infix_input(buffer);
    }

    /**
     * If print_help() printed help,
     * the method returns true and therefore the program quits.
     */
    bool print_help(){
        //Todo
        if(help_flag){
            std::cout << "Usage: aprox [-i in] [-o out] [-s interval size]" << std::endl;
            std::cout << "    -i: input file, default: stdin" << std::endl;
            std::cout << "    -o: output file, default: stdout" << std::endl;
            std::cout << "    -p: read postfix notation, default: infix" << std::endl;
            std::cout << "    -b: bin_size - size of the bins in which the distributions are stored, default = 1" << std::endl;
            std::cout << "    -r: how many bins to use during result presentation, default = " << NUM_OF_RESULT_BINS_DEFAULT << std::endl;
            return true;
        }
        return false;
    }

    

    /**
     * 
     * Returns true on success, false on failure.
     */
    bool output(){
        if(output_flag){
            if(save_output()) return true;
            else return false;
        }
        else{
            print_output();
            return true;
        }
    }

    /**
     * 
     * Returns true on success, false on failure (division by zero for example).
     */
    bool compute(){
        return true;
    }
};

int main(int argc, char **argv){

    Program<double> program(argc, argv);
    if(!program.parse_arguments()) return 1;
    if(program.print_help()) return 0;
    if(!program.read_input()) return 1;
    if(!program.compute()) return 1;
    if(!program.output()) return 1;

    return 0;
}