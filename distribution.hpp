#ifndef DISTRIBUTION_HPP_
#define DISTRIBUTION_HPP_

//TODO promazat nepotrebny knihovny
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <map>
#include <cmath>
#include <fstream>
#include <string>
#include <boost/math/distributions/normal.hpp>

#define DIVISION_ERROR 10
#define PRINT_BLOCK_PER_PROBABILITY 0.003

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

template <typename real>
class Distribution{

    std::map<real, real> distribution;
    char type;
    real from;
    real to;
    real standard_deviation_quotient;
    real bin_size; // TODO bin_size has to be compatible
    bool error_occured;

public:

    Distribution(){}

    Distribution(char type, real bin_size) : type(type), 
                                                                     bin_size(bin_size), 
                                                                     error_occured(false) {}

    //TODO nastavitelnej standard_deviation_quotient (tzn. nastavitelny variance)
    Distribution(char type, real from_param, real to_param, real bin_size, 
                 real standard_deviation_quotient) : 
                                                              type(type),
                                                              bin_size(bin_size),
                                                              error_occured(false) {


        from = nearest_bin(from_param);
        to = nearest_bin(to_param);

        // when from > to, the distribution is not correct
        if(from >= to){
            error_occured = true;
            return;
        }
        else error_occured = false;

        switch (type){
            case '~': // normal distribution
                create_normal_distribution(standard_deviation_quotient);
                break;
            case 'n': // also normal distribution
                create_normal_distribution(standard_deviation_quotient);
                break;
            case 'u': // uniform distribution
                create_uniform_distribution();
                break;
            case 'b': // binomial distribution
                create_binomial_distribution();
                break;
        default:
            break;
        }
    }

    /**
     * COPY CONSTRUCTOR
     */
    Distribution(const Distribution& second){
        DEBUG("Copy constructor called.");
        distribution = second.distribution;

        from = second.from;
        to = second.to;
        bin_size = second.bin_size;
        error_occured = second.error_occured;
    }

    /**
     * COPY ASSIGNMENT
     */
    Distribution& operator=(const Distribution& second){
        DEBUG("Assignment called");
        if(&second == this)
            return *this;

        distribution = second.distribution;

        from = second.from;
        to = second.to;
        bin_size = second.bin_size;
        error_occured = second.error_occured;
        
        return *this;
    }

    void create_normal_distribution(real standard_deviation_quotient){
        
        real mean = from + ((to-from) / 2);
        real standard_deviation = (mean-from) / standard_deviation_quotient;
        real sum = 0;

        auto dist = boost::math::normal_distribution<real>(mean, standard_deviation);
        for(real i = from; i <= to; i += bin_size){
            distribution[i] = boost::math::pdf(dist, i);
            sum += distribution[i];
        }

        // so that the sum equals 1
        distribution[from] += (1-sum)/2;
        distribution[to] += (1-sum)/2;
    }

    void create_uniform_distribution(){
        real uniform_value = 1 / return_num_of_bins();

        for(real i = from; i <= to; i += bin_size){
            distribution[i] = uniform_value;
        }
    }

    void create_binomial_distribution(){}

    real nearest_bin(real number){
        int multiple = round((number-from) / bin_size);
        return multiple * bin_size + from;
    }

    real nearest_bin(real number, real another_bin_size){
        int multiple = round((number-from) / another_bin_size);
        return multiple * another_bin_size + from;
    }

    void normalize(){
        real sum = 0;
        for(auto&& element : distribution){
            sum += element.second;
        }
        for(auto&& element : distribution){
            element.second /= sum;
        }
    }

    unsigned int return_num_of_bins(){
        return (to - from + bin_size) / bin_size;
    }

    Distribution operator+(const Distribution &second){
        Distribution<real> new_dist = Distribution<real>('m', bin_size); // m stands for mixed TODO
        if(error_occured || second.error_occured){
            new_dist.error_occured = true;
            return new_dist;
        }
        new_dist.error_occured = false;
        

        for (auto&& element1 : distribution){
            for (auto&& element2 : second.distribution){
                // already in new_dist
                if(new_dist.distribution.find(element1.first + element2.first) != new_dist.distribution.end()){
                    new_dist.distribution[element1.first + element2.first] += (element1.second + element2.second);
                }
                else{ // new element in new_dist
                    new_dist.distribution[element1.first + element2.first] = (element1.second + element2.second);
                }
            }
        }

        new_dist.from = new_dist.distribution.begin()->first;
        new_dist.to = new_dist.distribution.rbegin()->first; //TODO opravdu spravne?

        new_dist.normalize();
        return new_dist;
    }

    Distribution operator+(const real scalar){
        // TODO opravit (to *this)
        Distribution<real> new_dist = Distribution<real>(*this); // m stands for mixed TODO
        if(error_occured){
            new_dist.error_occured = true;
            return new_dist;
        }
        new_dist.error_occured = false;

        std::map<real, real> new_map;
        for(auto&& element : distribution){
            new_map[element.first + scalar] = element.second;
        }
        new_dist.from = from + scalar;
        new_dist.to = to + scalar;
        new_dist.distribution = new_map;

        return new_dist;
    }

    Distribution operator-(const Distribution &second){
        Distribution<real> new_dist = Distribution<real>('m', bin_size); // m stands for mixed TODO
        if(error_occured || second.error_occured){
            new_dist.error_occured = true;
            return new_dist;
        }
        new_dist.error_occured = false;

        for (auto&& element1 : distribution){
            for (auto&& element2 : second.distribution){
                // already in new_dist
                if(new_dist.distribution.find(element1.first - element2.first) != new_dist.distribution.end()){
                    new_dist.distribution[element1.first - element2.first] += (element1.second - element2.second);
                }
                else{ // new element in new_dist
                    new_dist.distribution[element1.first - element2.first] = (element1.second - element2.second);
                }
            }
        }

        new_dist.from = new_dist.distribution.begin()->first;
        new_dist.to = new_dist.distribution.rbegin()->first; //TODO opravdu spravne?

        new_dist.normalize();
        return new_dist;
    }

    Distribution operator-(const real scalar){
        Distribution<real> new_dist = Distribution<real>(*this); // m stands for mixed TODO
        if(error_occured){
            new_dist.error_occured = true;
            return new_dist;
        }
        new_dist.error_occured = false;

        std::map<real, real> new_map;
        for(auto&& element : distribution){
            new_map[element.first - scalar] = element.second;
        }
        new_dist.from = from - scalar;
        new_dist.to = to - scalar;
        new_dist.distribution = new_map;

        return new_dist;
    }

    Distribution operator*(const Distribution &second){
        Distribution<real> new_dist = Distribution<real>('m', bin_size); // m stands for mixed TODO
        if(error_occured || second.error_occured){
            new_dist.error_occured = true;
            return new_dist;
        }
        new_dist.error_occured = false;

        for (auto&& element1 : distribution){
            for (auto&& element2 : second.distribution){
                // already in new_dist
                if(new_dist.distribution.find(nearest_bin(element1.first * element2.first)) != new_dist.distribution.end()){
                    new_dist.distribution[nearest_bin(element1.first * element2.first)] += (element1.second + element2.second);
                }
                else{ // new element in new_dist
                    new_dist.distribution[nearest_bin(element1.first * element2.first)] = (element1.second + element2.second);
                }
            }
        }

        new_dist.from = new_dist.distribution.begin()->first;
        new_dist.to = new_dist.distribution.rbegin()->first; //TODO opravdu spravne?

        new_dist.normalize();

        return new_dist;
    }

    Distribution operator*(const real scalar){
        Distribution<real> new_dist = Distribution<real>(*this); // m stands for mixed TODO
        if(error_occured){
            new_dist.error_occured = true;
            return new_dist;
        }
        new_dist.error_occured = false;

        std::map<real, real> new_map;
        for(auto&& element : distribution){
            new_map[nearest_bin(element.first * scalar)] = element.second;
        }
        new_dist.from = nearest_bin(from * scalar);
        new_dist.to = nearest_bin(to * scalar);
        new_dist.distribution = new_map;
        return new_dist;
    }

    Distribution operator/(const Distribution &second){
        Distribution<real> prepared_for_division = divide_scalar_numerator(1);
        if(prepared_for_division.error_occured || second.error_occured || (second.from < 0 && second.to > 0)){
            prepared_for_division.error_occured = true;
            return prepared_for_division;
        }
        prepared_for_division.error_occured = false;

        return prepared_for_division * second;
    }

    Distribution operator/(const real scalar){
        // TODO opravit
        Distribution<real> new_dist = Distribution<real>(*this); // m stands for mixed TODO
        if(error_occured || scalar == 0){
            new_dist.error_occured = true;
            return new_dist;
        }
        new_dist.error_occured = false;

        std::map<real, real> new_map;
        for(auto&& element : distribution){
            new_map[nearest_bin(element.first / scalar)] = element.second;
        }
        new_dist.from = nearest_bin(from / scalar);
        new_dist.to = nearest_bin(to / scalar);
        new_dist.distribution = new_map;
        return new_dist;
    }

    real error_rounding(real number){
        return round(number * DIVISION_ERROR) / DIVISION_ERROR;
    }

    /**
     * num_of_result_bins ... -1 (print every bin)
     */
    void print(std::ostream& ostr, int num_of_result_bins){
        if(error_occured){
            std::cerr << "ERROR OCCURED DURING COMPUTATION (PROBABLY DIVISION BY 0)." << std::endl;
            return;
        }
        ostr << "RESULT = " << from << " ~ " << to << std::endl;
        ostr << std::endl;
        real sum = 0;

        // print return_num_of_bins() bins
        if (num_of_result_bins == -1 || (int)return_num_of_bins() < num_of_result_bins){
            for (real i = to; i >= from; i -= bin_size){
                ostr << std::right << std::setw(9) << i << "  "; // TODO zlepsit mezeru
                if(distribution.find(i) != distribution.end()){
                    sum += distribution[i];
                    int hvezd = distribution[i] / PRINT_BLOCK_PER_PROBABILITY;
                    for(int h = 0; h <= hvezd; h++) ostr << "*";
                }
                ostr << std::endl;
            }
        }
        else{ // otherwise print num_of_result_bins bins
            real new_bin_size = (to - from) / (num_of_result_bins - 1);
            std::map<real, real> tmp;

            for(real i = 0; i < num_of_result_bins; i++) tmp[error_rounding(nearest_bin(i*new_bin_size + from, new_bin_size))] = 0;

            for(auto&& element : distribution){
                tmp[error_rounding(nearest_bin(element.first, new_bin_size))] += element.second;
            }

            for(auto element = tmp.rbegin(); element != tmp.rend(); ++element){
                sum += element->second;
                int hvezd = element->second / PRINT_BLOCK_PER_PROBABILITY;
                ostr << std::right << std::setw(9) << element->first << "  "; // TODO zlepsit mezeru
                for(int h = 0; h <= hvezd; h++) ostr << "*";
                ostr << std::endl;
            }
        }

        DEBUG2("SUM = ", sum);
    }

    Distribution divide_scalar_numerator(real scalar){
        Distribution<real> new_dist = Distribution<real>('m', bin_size); // m stands for mixed TODO
        if(error_occured){
            new_dist.error_occured = true;
            return new_dist;
        }
        new_dist.error_occured = false;

        // TODO: vyresit deleni nulou
        for (auto&& element : distribution){
            // TODO NEAREST BIN NEBUDE FUNGOVAT, PROTOZE TAM NEPLATI TA PROMENNA FROM
            new_dist.distribution[nearest_bin(scalar/element.first)] = element.second;
            DEBUG(nearest_bin(scalar/element.first));
        }

        new_dist.from = new_dist.distribution.begin()->first;
        new_dist.to = new_dist.distribution.rbegin()->first;

        return new_dist;
    }
};

/**
 * Commutative arithmetic operation.
 */
template <typename real>
Distribution<real> operator+(const real scalar, Distribution<real> dist){
    return dist + scalar;
}

/**
 * Commutative arithmetic operation.
 */
template <typename real>
Distribution<real> operator-(const real scalar, Distribution<real> dist){
    return dist - scalar;
}

/**
 * Commutative arithmetic operation.
 */
template <typename real>
Distribution<real> operator*(const real scalar, Distribution<real> dist){
    return dist * scalar;
}


template <typename real>
Distribution<real> operator/(const real scalar, Distribution<real> dist){
    return dist.divide_scalar_numerator(scalar);
}

#endif