/*! @file individual.cpp
    @brief Implementation of Individual class
*/
#include "individual.hpp"
#include "individual.hpp"

#include <wtl/debug.hpp>
#include <wtl/iostr.hpp>
#include <wtl/prandom.hpp>

namespace pbt {

uint_fast32_t Individual::MEAN_CLUTCH_SIZE_ = 4;
std::vector<double> Individual::NATURAL_MORTALITY_(MAX_AGE_, 0.1);
std::vector<double> Individual::FISHING_MORTALITY_(MAX_AGE_, 0.1);
std::vector<double> Individual::WEIGHT_FOR_AGE_(MAX_AGE_, 0.0);
uint_fast32_t Individual::LAST_ID_ = 0;

//! Program options
/*! @ingroup params
    @return Program options description

    Command line option | Symbol         | Variable
    ------------------- | -------------- | -------------------------------
    `-c,--clutch`       | -              | Individual::MEAN_CLUTCH_SIZE_
*/
boost::program_options::options_description Individual::options_desc() {
    namespace po = boost::program_options;
    po::options_description desc{"Individual"};
    desc.add_options()
        ("clutch,c", po::value(&MEAN_CLUTCH_SIZE_)->default_value(MEAN_CLUTCH_SIZE_))
    ;
    return desc;
}

void Individual::set_static() {HERE;
    for (uint_fast32_t i=0; i<MAX_AGE_; ++i) {
        WEIGHT_FOR_AGE_[i] = -MAX_WEIGHT_ * std::expm1(-GROWTH_RATE_ * i);
    }
}

void Individual::from_json(const json::json& obj) {HERE;
    NATURAL_MORTALITY_ = obj.at("natural_mortality").get<decltype(NATURAL_MORTALITY_)>();
    FISHING_MORTALITY_ = obj.at("fishing_mortality").get<decltype(FISHING_MORTALITY_)>();
    WEIGHT_FOR_AGE_ = obj.at("weight_for_age").get<decltype(WEIGHT_FOR_AGE_)>();
}

void Individual::to_json(json::json& obj) {HERE;
    obj["natural_mortality"] = NATURAL_MORTALITY_;
    obj["fishing_mortality"] = FISHING_MORTALITY_;
    obj["weight_for_age"] = WEIGHT_FOR_AGE_;
}

std::ostream& Individual::write(std::ostream& ost) const {
    return ost << id_ << ":"
               << father_id_ << ":" << mother_id_ << ":"
               << birth_year_;
}

//! shortcut Individual::write(ost)
std::ostream& operator<<(std::ostream& ost, const Individual& x) {
    return x.write(ost);
}

void Individual::test() {HERE;
    Individual x;
    std::cout << x << std::endl;
}

} // namespace pbt
