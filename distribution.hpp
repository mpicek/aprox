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
    real from;
    real to;
    real mean;
    real standard_deviation;
    real standard_deviation_quotient;
    real bin_size;
    bool wrong_distribution;
    unsigned int num_of_bins;

public:

    Distribution(){}

    //TODO nastavitelnej standard_deviation_quotient (tzn. nastavitelny variance)
    Distribution(char type, real from_param, real to_param, int bin_size) : mean(0), 
                                                              standard_deviation(0),
                                                              standard_deviation_quotient(2),
                                                              bin_size(bin_size) {

        DEBUG2(from_param, to_param);
        DEBUG(bin_size);
        from = round((float)from_param / bin_size);
        to = round((float)to_param/bin_size);

        DEBUG2(from, to);

        // when from > to, the distribution is not correct
        if(from > to){
            wrong_distribution = true;
            return;
        }
        else wrong_distribution = false;

        num_of_bins = (to - from + bin_size) / bin_size;
        DEBUG2("num of bints: ", num_of_bins);

        switch (type){
            case '~': // normal distribution
                create_normal_distribution();
                break;
            case 'n': // also normal distribution
                create_normal_distribution();
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

    void create_normal_distribution(){
        mean = (to-from) / 2;
        standard_deviation = (mean-from) / standard_deviation_quotient;
        float sum = 0;

        auto dist = boost::math::normal_distribution<real>(mean, standard_deviation);
        for(real i = from; i <= to; i += bin_size){
            distribution[i] = boost::math::pdf(dist, i);
            DEBUG2(i, distribution[i]);
            sum += distribution[i];
        }

        // so that the sum equals 1
        distribution[from] += (1-sum)/2;
        distribution[to] += (1-sum)/2;
    }

    void create_uniform_distribution(){
        real uniform_value = 1 / num_of_bins;

        for(real i = from; i <= to; i += bin_size){
            distribution[i] = uniform_value;
        }
    }

    void create_binomial_distribution(){
        
    }

    Distribution operator+(const Distribution &second){}
    Distribution operator+(const int scalar){}

    Distribution operator-(const Distribution &second){}
    Distribution operator-(const int scalar){}

    Distribution operator*(const Distribution &second){}
    Distribution operator*(const int scalar){}

    Distribution operator/(const Distribution &second){}
    Distribution operator/(const int scalar){}

    void print(){
        std::cout << std::endl;

        for (auto&& x : distribution){
            int hvezd = x.second / 0.01;
            for(int h = 0; h <= hvezd; h++) std::cout << "*";
            std::cout << std::endl;
        }
    }
};

#endif