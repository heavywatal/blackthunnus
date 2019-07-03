/*! @file individual.hpp
    @brief Interface of Individual class
*/
#pragma once
#ifndef PBT_INDIVIDUAL_HPP_
#define PBT_INDIVIDUAL_HPP_

#include "random_fwd.hpp"

#include <cstdint>
#include <iosfwd>
#include <memory>
#include <vector>
#include <map>
#include <limits>

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////

namespace pbf {

//! @brief Parameters for Individual class (command-line)
/*! @ingroup params
*/
struct IndividualParams {
    //! @ingroup params
    //@{
    //! \f$r\f$:  used in Individual::recruitment()
    double RECRUITMENT_COEF = 2.0;
    //! \f$K\f$: carrying capacity used in Individual::recruitment()
    double CARRYING_CAPACITY = 1e+3;
    //! \f$k \in (0, \infty)\f$ for overdispersion in recruitment().
    //! Equivalent to Poisson when \f$k \to \infty\f$ (or \f$k<0\f$ for convience).
    double NEGATIVE_BINOM_K = -1.0;
    //@}
};

//! @brief Parameters for Individual class (JSON file)
/*! @ingroup params
*/
struct IndividualJson {
    //! @cond
    IndividualJson();
    void set_dependent_static();
    void read(std::istream&);
    void write(std::ostream&) const;
    //! @endcond

    //! @ingroup params
    //@{
    //! mortality due to natural causes
    std::vector<double> NATURAL_MORTALITY;
    //! mortality due to fishing activities
    std::vector<double> FISHING_MORTALITY;
    //! precalculated values
    std::vector<double> WEIGHT_FOR_AGE;
    //! transition matrix for migration
    std::vector<std::vector<std::vector<double>>> MIGRATION_MATRICES;
    //@}

    //! survival rate per quater year
    std::vector<double> SURVIVAL_RATE;
    //! discrete distributions for migration
    std::vector<std::vector<std::discrete_distribution<uint_fast16_t>>> MIGRATION_DISTRIBUTIONS;
};

/*! @brief Individual class
*/
class Individual {
  public:
    //! Alias
    using param_type = IndividualParams;
    Individual() = delete;
    //! for initial population
    explicit Individual(bool is_male): is_male_(is_male) {}
    //! for sexual reproduction
    Individual(const std::shared_ptr<Individual>& father,
               const std::shared_ptr<Individual>& mother, int_fast32_t year, bool is_male)
    : father_(father), mother_(mother),
      birth_year_(year), location_(mother ? mother->location() : 0u),
      is_male_(is_male) {}

    //! evaluate survival
    bool has_survived(const int_fast32_t year, const int_fast32_t quarter, URBG&) const;

    //! evaluate maturity
    bool is_in_breeding_place() const noexcept {
        return location_ < num_breeding_places();
    }

    //! number of juveniles
    uint_fast32_t recruitment(int_fast32_t year, double density_effect, URBG&) const noexcept;

    //! change #location_
    void migrate(const int_fast32_t year, URBG&);

    //! collect ancestoral IDs
    void trace_back(std::ostream& ost, std::map<const Individual*, size_t>* ids, int_fast32_t year) const;
    //! write all the data members in TSV
    std::ostream& write(std::ostream&) const;
    //! write all the data members in TSV with translated IDs
    std::ostream& write(std::ostream&, const std::map<const Individual*, size_t>&) const;
    friend std::ostream& operator<<(std::ostream&, const Individual&);
    //! column names for write()
    static std::vector<std::string> names();
    //! Get default parameters in json
    static std::string default_json();
    //! Read class variables from stream in json
    static void read_json(std::istream& ist) {JSON_.read(ist);}
    //! Write class variables to stream in json
    static void write_json(std::ostream& ost) {JSON_.write(ost);}
    //! Export negative_binomial_distribution to Rcpp for testing
    static std::vector<int> rnbinom(int n, double k, double mu);
    //! Set #PARAM_
    static void param(const param_type& p) {PARAM_ = p;}
    //! Get #PARAM_
    static const param_type& param() {return PARAM_;}

    //! number of locations
    static size_t num_locations() noexcept {
        return JSON_.MIGRATION_MATRICES[0u].size();
    }
    //! number of breeding places
    static constexpr size_t num_breeding_places() noexcept {return 2u;}

    //! @name Getter functions
    //@{
    //! IndividualJson.NATURAL_MORTALITY
    static const std::vector<double>&
    natural_mortality() {return JSON_.NATURAL_MORTALITY;}
    //! IndividualJson.FISHING_MORTALITY
    static const std::vector<double>&
    fishing_mortality() {return JSON_.FISHING_MORTALITY;}
    //! IndividualJson.SURVIVAL_RATE
    static const std::vector<double>&
    survival_rate() {return JSON_.SURVIVAL_RATE;}
    //! IndividualJson.WEIGHT_FOR_AGE
    static const std::vector<double>&
    weight_for_age() {return JSON_.WEIGHT_FOR_AGE;}
    //! IndividualJson.MIGRATION_MATRICES
    static const std::vector<std::vector<std::vector<double>>>&
    migration_matrices() {return JSON_.MIGRATION_MATRICES;}
    //! IndividualJson.WEIGHT_FOR_AGE
    double weight(const int_fast32_t year) const noexcept {
        return JSON_.WEIGHT_FOR_AGE[4 * (year - birth_year_)];
    }
    //! !#father_
    bool is_first_gen() const noexcept {return !father_;}
    //! @cond
    const Individual* father_get() const noexcept {return father_.get();}
    const Individual* mother_get() const noexcept {return mother_.get();}
    int_fast32_t birth_year() const noexcept {return birth_year_;}
    uint_fast16_t location() const noexcept {return location_;}
    bool is_male() const noexcept {return is_male_;}
    //! @endcond
    //@}

  private:
    //! Parameters shared among instances (command-line)
    static param_type PARAM_;
    //! Parameters shared among instances (JSON file)
    static IndividualJson JSON_;

    //! father
    const std::shared_ptr<Individual> father_ = nullptr;
    //! mother
    const std::shared_ptr<Individual> mother_ = nullptr;
    //! year of birth
    int_fast32_t birth_year_ = -4;
    //! current location
    uint_fast16_t location_ = 0u;
    //! sex
    const bool is_male_;
};

} // namespace pbf

#endif /* PBT_INDIVIDUAL_HPP_ */
