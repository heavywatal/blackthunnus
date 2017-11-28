/*! @file individual.hpp
    @brief Interface of Individual class
*/
#pragma once
#ifndef PBT_INDIVIDUAL_HPP_
#define PBT_INDIVIDUAL_HPP_

#include "common.hpp"

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
    bool has_survived(const uint_fast32_t year, urbg_t& g) const {
        const auto age = year - birth_year_;
        return (g.canonical() > NATURAL_MORTALITY_[age])
            && (g.canonical() > FISHING_MORTALITY_[age]);
    }

    //! evaluate maturity
    bool is_in_breeding_place() const {
        return location_ < 2u;
    }

    //! number of eggs per mating
    uint_fast32_t clutch_size(urbg_t& g) const {
        thread_local std::poisson_distribution<uint_fast32_t> poisson(MEAN_CLUTCH_SIZE_);
        return poisson(g);
    }

    //! change #location_
    void migrate(const uint_fast32_t year, urbg_t& g) {
        location_ = MIGRATION_DISTRIBUTIONS_[year - birth_year_][location_](g);
    }

    //! write
    std::ostream& write(std::ostream&) const;
    friend std::ostream& operator<<(std::ostream&, const Individual&);
    //! options description for Individual class
    static boost::program_options::options_description options_desc();
    //! set static variables to default values
    static void set_static_default();
    //! set static variables that depend on other variables
    static void set_dependent_static();
    //! set class variables from json
    static void from_json(const json::json&);
    //! encode class variables to json
    static void to_json(json::json&);
    //! unit test
    static void test();

    //! number of locations
    static size_t num_locations() {
        return MIGRATION_MATRICES_[0].size();
    }
    //! getter of #WEIGHT_FOR_AGE_
    double weight(const uint_fast32_t year) const {
        return WEIGHT_FOR_AGE_[year - birth_year_];
    }
    //! getter of #id_
    uint_fast32_t id() const {return id_;}
    //! getter of #location_
    uint_fast32_t location() const {return location_;}

  private:
    //! maximum age to consider
    constexpr static uint_fast32_t MAX_AGE_ = 80u;
    //! \f$K\f$ in weight()
    constexpr static double GROWTH_RATE_ = 0.08;
    //! \f$L\f$ in weight()
    constexpr static double MAX_WEIGHT_ = 500.0;
    //! parameter for clutch_size()
    static uint_fast32_t MEAN_CLUTCH_SIZE_;
    //! mortality due to natural causes
    static std::vector<double> NATURAL_MORTALITY_;
    //! mortality due to fishing activities
    static std::vector<double> FISHING_MORTALITY_;
    //! precalculated values
    static std::vector<double> WEIGHT_FOR_AGE_;
    //! transition matrix for migration
    static std::vector<std::vector<std::vector<double>>> MIGRATION_MATRICES_;
    //! discrete distributions for migration
    static std::vector<std::vector<std::discrete_distribution<uint_fast32_t>>> MIGRATION_DISTRIBUTIONS_;
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
