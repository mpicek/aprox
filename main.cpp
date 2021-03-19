#include <iostream>
#include <unistd.h>
#include <sstream>

// Returns true on success, false on failure.
// TODO: pridat const
// Todo: getopt na parsovani argumentu

class Expression{

};

// real is the type that represents the real number
template <typename real> class Program{

    int argc;
    char **argv;

    bool input_flag;
    char* input_file_name;
    
    bool output_flag;
    char* output_file_name;

    bool help_flag;

    real bin_size; // size of the bins in which the distributions are stored

    /**
     * 
     */
    bool save_output(){

    }

    /**
     * 
     */
    void print_output(){

    }

public:
    Program(int argc, char **argv) : argc(argc),
                                     argv(argv),
                                     bin_size(1) {}


    /**
     * Parses arguments using getopt.
     * Returns true on success, false on failure.
     */
    bool parse_arguments(){
        int c;
        char* bin_size_char = nullptr;

        // after argument : = it needs another argument
        // after argument :: = another argument is optional
        while((c = getopt(argc, argv, "i:o:b:h")) != -1){
            bool s_in_switch = false;
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
                    std::cout << "ERROR: UNABLE TO READ BIN_SIZE, SETTING BIN_SIZE TO 1 (DEFAULT)." << std::endl;
                    bin_size = 1;
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

    }

    /**
     * If print_help() printed help,
     * the method returns true and therefore the program quits.
     */
    bool print_help(){
        //Todo
        if(help_flag){
            std::cout << "Usage: aprox [-i in] [-o out] [-s interval size]" << std::endl;
            std::cout << "    -i: input file (default: stdin)" << std::endl;
            std::cout << "    -o: output file (default: stdout)" << std::endl;
            std::cout << "    -b: bin_size - size of the bins in which the distributions are stored, default = 1" << std::endl;
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