/*! @file individual.hpp
    @brief Interface of Individual class
*/
#pragma once
#ifndef PBT_INDIVIDUAL_HPP_
#define PBT_INDIVIDUAL_HPP_

#include <iosfwd>
#include <cstdint>
#include <cmath>
#include <random>

#include <boost/program_options.hpp>

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////

namespace pbt {

/*! @brief Individual class
*/
class Individual {
  public:
    //! default constructor
    Individual()
    : id_(++LAST_ID_) {}
    //! sexual reproduction
    Individual(const Individual& father, const Individual& mother, const uint_fast32_t year)
    : id_(++LAST_ID_), father_id_(father.id()), mother_id_(mother.id()),
      birth_year_(year), location_(mother.location()) {}

    //! evaluate survival
    bool has_survived() const;

    //! evaluate maturity
    bool is_matured(const uint_fast32_t year) const {
        return year > birth_year_ + AGE_OF_MATURATION_;
    }

    //! von Bertalanffy growth equation
    /*! \f[
            L = L_\infty(1 - e^{-K(t - t_0)})
        \f]
    */
    double weight(const uint_fast32_t year) const {
        return -MAX_WEIGHT_ * std::expm1(-GROWTH_RATE_ * (year - birth_year_));
    }

    //! number of matings per season
    template <class URBG>
    uint_fast32_t mating_number(URBG& g) const {
        thread_local std::poisson_distribution<uint_fast32_t> poisson(MEAN_MATING_NUMBER_);
        return poisson(g);
    }

    //! number of eggs per mating
    template <class URBG>
    uint_fast32_t clutch_size(URBG& g) const {
        thread_local std::poisson_distribution<uint_fast32_t> poisson(MEAN_CLUTCH_SIZE_);
        return poisson(g);
    }

    //! change #location_
    template <class URBG>
    void migrate(URBG& g) {
        const std::vector<double> p = {0.1, 0.2, 0.3, 0.4};
        std::discrete_distribution<uint_fast32_t> dist(p.begin(), p.end());
        location_ = dist(g);
    }

    //! write
    std::ostream& write(std::ostream&) const;
    friend std::ostream& operator<<(std::ostream&, const Individual&);
    //! options description for Individual class
    static boost::program_options::options_description options_desc();
    //! unit test
    static void test();

    //! getter of #id_
    uint_fast32_t id() const {return id_;}
    //! getter of #location_
    uint_fast32_t location() const {return location_;}

  private:
    //! by the year
    constexpr static uint_fast32_t AGE_OF_MATURATION_ = 3;
    //! \f$K\f$ in weight()
    constexpr static double GROWTH_RATE_ = 0.02;
    //! \f$L\f$ in weight()
    constexpr static double MAX_WEIGHT_ = 500.0;
    //! parameter for mating_number()
    static uint_fast32_t MEAN_MATING_NUMBER_;
    //! parameter for clutch_size()
    static uint_fast32_t MEAN_CLUTCH_SIZE_;
    //! mortality due to natural causes
    static double NATURAL_MORTALITY_;
    //! mortality due to fishing activities
    static double FISHING_MORTALITY_;
    //! ID for a new instance
    static uint_fast32_t LAST_ID_;

    //! ID
    const uint_fast32_t id_ = 0;
    //! father's ID
    const uint_fast32_t father_id_ = 0;
    //! mother's ID
    const uint_fast32_t mother_id_ = 0;
    //! year of birth
    uint_fast32_t birth_year_ = 0;
    //! current location
    uint_fast32_t location_ = 0;
};

} // namespace pbt

#endif /* PBT_INDIVIDUAL_HPP_ */
