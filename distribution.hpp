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
    char type;
    real from;
    real to;
    real mean;
    real standard_deviation;
    real standard_deviation_quotient;
    real bin_size;
    bool wrong_distribution;

public:

    Distribution(){}

    Distribution(char type, real bin_size) : type(type), bin_size(bin_size), wrong_distribution(false) {}

    //TODO nastavitelnej standard_deviation_quotient (tzn. nastavitelny variance)
    Distribution(char type, real from_param, real to_param, real bin_size) : type(type), mean(0), 
                                                              standard_deviation(0),
                                                              standard_deviation_quotient(2),
                                                              bin_size(bin_size),
                                                              wrong_distribution(false) {

        DEBUG2(from_param, to_param);
        DEBUG(bin_size);

        //TODO wrong!!! from and to have to be counted differently (round to closest multiple of bin_size!!)
        from = round((float)from_param / bin_size);
        to = round((float)to_param/bin_size);

        DEBUG2(from, to);

        // when from > to, the distribution is not correct
        if(from > to){
            wrong_distribution = true;
            return;
        }
        else wrong_distribution = false;

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
        
        mean = from + ((to-from) / 2);
        std::cout << mean << " " << from << " " << to << " " << std::endl;
        DEBUG2((mean-from), standard_deviation_quotient);
        standard_deviation = (mean-from) / standard_deviation_quotient;
        float sum = 0;

        std::cout << "::::::::::::::::::::::::::::" << standard_deviation << std::endl;
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
        real uniform_value = 1 / return_num_of_bins();

        for(real i = from; i <= to; i += bin_size){
            distribution[i] = uniform_value;
        }
    }

    void create_binomial_distribution(){
        
    }

    void normalize(){
        float sum = 0;
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
        float sum = 0;

        float divide_by = (float)return_num_of_bins();
        for (auto&& element1 : distribution){
            for (auto&& element2 : second.distribution){
                // already in new_dist
                if(new_dist.distribution.find(element1.first + element2.first) != new_dist.distribution.end()){
                    new_dist.distribution[element1.first + element2.first] += (element1.second + element2.second);
                }
                else{ // new element in new_dist
                    new_dist.distribution[element1.first + element2.first] = (element1.second + element2.second);
                }
                sum += (element1.second + element2.second);
            }
        }

        new_dist.from = new_dist.distribution.begin()->first;
        new_dist.to = new_dist.distribution.rbegin()->first; //TODO opravdu spravne?

        DEBUG2("SUM = ", sum);

        new_dist.normalize();
        return new_dist;
    }
    Distribution operator+(const int scalar){}

    Distribution operator-(const Distribution &second){}
    Distribution operator-(const int scalar){}

    Distribution operator*(const Distribution &second){}
    Distribution operator*(const int scalar){}

    Distribution operator/(const Distribution &second){}
    Distribution operator/(const int scalar){}

    void print(){
        std::cout << std::endl;
        float sum = 0;

        for (float i = from; i <= to; i += bin_size){

            if(distribution.find(i) != distribution.end()){
                sum += distribution[i];
                int hvezd = distribution[i] / 0.01;
                std::cout << std::right << std::setw(9) << i << " "; // TODO zlepsit mezeru
                for(int h = 0; h <= hvezd; h++) std::cout << "*";
            }
            std::cout << std::endl;
        }

        DEBUG2("SUM = ", sum);
    }
};

#endif