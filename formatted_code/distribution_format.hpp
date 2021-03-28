#ifndef DISTRIBUTION_HPP_
#define DISTRIBUTION_HPP_

#include <boost/math/distributions/normal.hpp>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unistd.h>

#define DIVISION_ERROR 100
#define PRINT_BLOCK_PER_PROBABILITY 0.003

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

template <typename real> class Distribution {

  std::map<real, real> distribution;
  char type;
  real from;
  real to;
  real standard_deviation_quotient;
  real bin_size;

public:
  bool error_occurred;

  Distribution() {}

  Distribution(char type, real bin_size)
      : type(type), bin_size(bin_size), error_occurred(false) {}

  /**
   * Creates distribution. If error occurred, it is saved in error_occurred.
   */
  Distribution(char type, real from_param, real to_param, real bin_size,
               real standard_deviation_quotient)
      : type(type), bin_size(bin_size), error_occurred(false) {

    from = nearest_bin(from_param);
    to = nearest_bin(to_param);

    // when from > to, the distribution is not correct
    if (from > to) {
      error_occurred = true;
      return;
    } else
      error_occurred = false;

    // if distribution is just a single value
    if (from == to) {
      distribution[from] = 1;
      return;
    }

    switch (type) {
    case '~': // normal distribution
      create_normal_distribution(standard_deviation_quotient);
      break;
    case 'n': // also normal distribution
      create_normal_distribution(standard_deviation_quotient);
      break;
    case 'u': // uniform distribution
      create_uniform_distribution();
      break;
    default:
      break;
    }
  }

  /**
   * COPY CONSTRUCTOR
   */
  Distribution(const Distribution &second) {
    DEBUG("Copy constructor called.");
    distribution = second.distribution;

    from = second.from;
    to = second.to;
    bin_size = second.bin_size;
    error_occurred = second.error_occurred;
  }

  /**
   * COPY ASSIGNMENT
   */
  Distribution &operator=(const Distribution &second) {
    DEBUG("Assignment called");
    if (&second == this)
      return *this;

    distribution = second.distribution;

    from = second.from;
    to = second.to;
    bin_size = second.bin_size;
    error_occurred = second.error_occurred;

    return *this;
  }

  /**
   * Creates normal distribution with standard_deviation_quotient defined
   * in arguments. Mean is computed from variables from and to.
   */
  void create_normal_distribution(real standard_deviation_quotient) {

    real mean = from + ((to - from) / 2);
    real standard_deviation = (mean - from) / standard_deviation_quotient;
    real sum = 0;

    auto dist =
        boost::math::normal_distribution<real>(mean, standard_deviation);
    for (real i = from; i <= to; i += bin_size) {
      distribution[i] = boost::math::pdf(dist, i); // sampling from pdf
      sum += distribution[i];
    }

    normalize(); // normalizing the distribution
  }

  /**
   * Creates uniform distribution.
   */
  void create_uniform_distribution() {
    real uniform_value = (real)1 / return_num_of_bins();

    for (real i = from; i <= to; i += bin_size) {
      distribution[i] = uniform_value;
    }
  }

  /**
   * Finds nearest bin into which the number should go.
   */
  real nearest_bin(real number) {
    int multiple = round((number - from) / bin_size);
    return multiple * bin_size + from;
  }

  /**
   * Finds nearest bin into which the number should go, but the another_bin_size
   * is different from the bin_size of the distribution.
   */
  real nearest_bin(real number, real another_bin_size) {
    int multiple = round((number - from) / another_bin_size);
    return multiple * another_bin_size + from;
  }

  /**
   * Normalizes distribution so that the sum equals 1.
   */
  void normalize() {
    real sum = 0;
    for (auto &&element : distribution) {
      sum += element.second;
    }
    for (auto &&element : distribution) {
      element.second /= sum;
    }
  }

  /**
   * Returns the number of bins of the distribution.
   */
  unsigned int return_num_of_bins() {
    return (to - from + bin_size) / bin_size;
  }

  Distribution operator+(const Distribution &second) {
    Distribution<real> new_dist = Distribution<real>('m', bin_size);
    if (error_occurred || second.error_occurred) {
      new_dist.error_occurred = true;
      return new_dist;
    }
    new_dist.error_occurred = false;

    // sum of two distributions (each element with each)
    for (auto &&element1 : distribution) {
      for (auto &&element2 : second.distribution) {
        // already in new_dist
        if (new_dist.distribution.find(element1.first + element2.first) !=
            new_dist.distribution.end()) {
          new_dist.distribution[element1.first + element2.first] +=
              (element1.second + element2.second);
        } else { // new element in new_dist
          new_dist.distribution[element1.first + element2.first] =
              (element1.second + element2.second);
        }
      }
    }

    new_dist.from = new_dist.distribution.begin()->first;
    new_dist.to = new_dist.distribution.rbegin()->first;

    new_dist.normalize();
    return new_dist;
  }

  Distribution operator+(const real scalar) {
    Distribution<real> new_dist = Distribution<real>(*this);
    if (error_occurred) {
      new_dist.error_occurred = true;
      return new_dist;
    }
    new_dist.error_occurred = false;

    std::map<real, real> new_map;
    for (auto &&element : distribution) {
      new_map[element.first + scalar] = element.second;
    }
    new_dist.from = from + scalar;
    new_dist.to = to + scalar;
    new_dist.distribution = new_map;

    return new_dist;
  }

  Distribution operator-(const Distribution &second) {
    Distribution<real> new_dist = Distribution<real>('m', bin_size);
    if (error_occurred || second.error_occurred) {
      new_dist.error_occurred = true;
      return new_dist;
    }
    new_dist.error_occurred = false;

    // difference of two distributions (each element with each)
    for (auto &&element1 : distribution) {
      for (auto &&element2 : second.distribution) {
        // already in new_dist
        if (new_dist.distribution.find(element1.first - element2.first) !=
            new_dist.distribution.end()) {
          new_dist.distribution[element1.first - element2.first] +=
              (element1.second - element2.second);
        } else { // new element in new_dist
          new_dist.distribution[element1.first - element2.first] =
              (element1.second - element2.second);
        }
      }
    }

    new_dist.from = new_dist.distribution.begin()->first;
    new_dist.to = new_dist.distribution.rbegin()->first;

    new_dist.normalize();
    return new_dist;
  }

  Distribution operator-(const real scalar) {
    Distribution<real> new_dist = Distribution<real>(*this);
    if (error_occurred) {
      new_dist.error_occurred = true;
      return new_dist;
    }
    new_dist.error_occurred = false;

    std::map<real, real> new_map;
    for (auto &&element : distribution) {
      new_map[element.first - scalar] = element.second;
    }
    new_dist.from = from - scalar;
    new_dist.to = to - scalar;
    new_dist.distribution = new_map;

    return new_dist;
  }

  Distribution operator*(const Distribution &second) {
    Distribution<real> new_dist = Distribution<real>('m', bin_size);
    if (error_occurred || second.error_occurred) {
      new_dist.error_occurred = true;
      return new_dist;
    }
    new_dist.error_occurred = false;

    // product of two distributions (each element with each)
    for (auto &&element1 : distribution) {
      for (auto &&element2 : second.distribution) {
        // already in new_dist
        if (new_dist.distribution.find(
                nearest_bin(element1.first * element2.first)) !=
            new_dist.distribution.end()) {
          new_dist.distribution[nearest_bin(element1.first * element2.first)] +=
              (element1.second + element2.second);
        } else { // new element in new_dist
          new_dist.distribution[nearest_bin(element1.first * element2.first)] =
              (element1.second + element2.second);
        }
      }
    }

    new_dist.from = new_dist.distribution.begin()->first;
    new_dist.to = new_dist.distribution.rbegin()->first;

    new_dist.normalize();

    return new_dist;
  }

  Distribution operator*(const real scalar) {
    Distribution<real> new_dist = Distribution<real>(*this);
    if (error_occurred) {
      new_dist.error_occurred = true;
      return new_dist;
    }
    new_dist.error_occurred = false;

    std::map<real, real> new_map;
    for (auto &&element : distribution) {
      new_map[nearest_bin(element.first * scalar)] = element.second;
    }
    new_dist.from = nearest_bin(from * scalar);
    new_dist.to = nearest_bin(to * scalar);
    new_dist.distribution = new_map;
    return new_dist;
  }

  /**
   * We just transfer division to product with the help of the function
   * divide_scalar_numerator (each element becomes (1/element)). Than we
   * multiply this new distribution with the first one.
   */
  Distribution operator/(Distribution &second) {
    Distribution<real> prepared_for_division =
        second.divide_scalar_numerator(1);
    if (prepared_for_division.error_occurred || error_occurred) {
      prepared_for_division.error_occurred = true;
      return prepared_for_division;
    }
    prepared_for_division.error_occurred = false;

    return prepared_for_division * *this; // product
  }

  Distribution operator/(const real scalar) {
    Distribution<real> new_dist = Distribution<real>(*this);
    if (error_occurred || scalar == 0) {
      new_dist.error_occurred = true;
      return new_dist;
    }
    new_dist.error_occurred = false;

    std::map<real, real> new_map;
    for (auto &&element : distribution) {
      new_map[nearest_bin(element.first / scalar)] = element.second;
    }
    new_dist.from = nearest_bin(from / scalar);
    new_dist.to = nearest_bin(to / scalar);
    new_dist.distribution = new_map;

    new_dist.normalize();
    return new_dist;
  }

  /**
   * Rounding of the number so that we avoid errors (mainly in indexing).
   */
  real error_rounding(real number) {
    return round(number * DIVISION_ERROR) / DIVISION_ERROR;
  }

  /**
   * Prints the distribution.
   * If num_of_result_bins = -1 then print every bin we have in the distribution
   */
  void print(std::ostream &ostr, int num_of_result_bins) {
    if (error_occurred) {
      std::cerr << "ERROR OCCURRED DURING COMPUTATION." << std::endl;
      return;
    }

    ostr << "RESULT = " << from << " ~ " << to << std::endl;
    ostr << std::endl;
    real sum = 0;

    real new_bin_size; // bin size might differ depending on the number of bins
    if (num_of_result_bins == -1) {
      new_bin_size = bin_size;
    } else {
      new_bin_size = (to - from) / (num_of_result_bins - 1);
    }

    std::map<real, real>
        tmp; // temporary map in which we will sum the distribution

    // For each element in the distribution we find the bin into which
    // it sould go in the new shortened distribution. At first we just
    // initialize it.
    for (real i = 0; i < num_of_result_bins; i++)
      tmp[error_rounding(nearest_bin(i * new_bin_size + from, new_bin_size))] =
          0;

    // Now we sum it in the tmp distribution.
    for (auto &&element : distribution) {
      tmp[error_rounding(nearest_bin(element.first, new_bin_size))] +=
          element.second;
    }

    // Printing
    for (auto element = tmp.rbegin(); element != tmp.rend(); ++element) {
      sum += element->second;
      int hvezd = element->second / PRINT_BLOCK_PER_PROBABILITY;
      ostr << std::right << std::setw(9) << element->first << "  ";
      for (int h = 0; h <= hvezd; h++)
        ostr << "*";
      ostr << std::endl;
    }
  }

  /**
   * Makes operation: scalar / distribution.
   */
  Distribution divide_scalar_numerator(real scalar) {
    Distribution<real> new_dist = Distribution<real>('m', bin_size);
    if (error_occurred) {
      new_dist.error_occurred = true;
      return new_dist;
    }
    new_dist.error_occurred = false;

    // division by zero
    if (from <= 0 && to >= 0) {
      new_dist.error_occurred = true;
      return new_dist;
    }

    for (auto &&element : distribution) {
      new_dist.distribution[nearest_bin(scalar / element.first)] =
          element.second;
      DEBUG(nearest_bin(scalar / element.first));
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
Distribution<real> operator+(const real scalar, Distribution<real> dist) {
  return dist + scalar;
}

/**
 * Commutative arithmetic operation.
 */
template <typename real>
Distribution<real> operator-(const real scalar, Distribution<real> dist) {
  return dist - scalar;
}

/**
 * Commutative arithmetic operation.
 */
template <typename real>
Distribution<real> operator*(const real scalar, Distribution<real> dist) {
  return dist * scalar;
}

/**
 * Divison - not commutative
 */
template <typename real>
Distribution<real> operator/(const real scalar, Distribution<real> dist) {
  return dist.divide_scalar_numerator(scalar);
}

#endif