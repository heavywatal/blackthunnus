/*! @file population.cpp
    @brief Implementation of Population class
*/
#include "population.hpp"
#include "individual.hpp"

#include <wtl/debug.hpp>
#include <wtl/iostr.hpp>
#include <wtl/prandom.hpp>
#include <sfmt.hpp>

#include <iostream>

namespace pbt {

Population::Population(const size_t initial_size) {HERE;
    const size_t half = initial_size / 2UL;
    males_.resize(half);
    females_.resize(half);
}

void Population::run(const uint_fast32_t years) {HERE;
    for (uint_fast32_t i=0; i<years; ++i) {
        ++year_;
        survive();
        survive();
        survive();
        survive();
        reproduce();
        std::cerr << year_ << ": " << males_.size() + females_.size() << std::endl;
    }
}

void Population::reproduce() {
    std::vector<Individual> boys;
    std::vector<Individual> girls;
    for (const auto& mother: females_) {
        if (!mother.is_matured(year_)) continue;
        auto mating_number = mother.mating_number(wtl::sfmt());
        while (mating_number > 0U) {
            const Individual& father = *wtl::choice(males_.begin(), males_.end(), wtl::sfmt());
            // TODO: weighted sampling
            if (!father.is_matured(year_)) continue;
            --mating_number;
            const unsigned int num_eggs = mother.clutch_size(wtl::sfmt());
            for (unsigned int i=0; i<num_eggs; ++i) {
                if (wtl::sfmt().canonical() < 0.5) {
                    boys.emplace_back(father, mother, year_);
                } else {
                    girls.emplace_back(father, mother, year_);
                }
            }
        }
    }
    std::copy(boys.begin(), boys.end(), std::back_inserter(males_));
    std::copy(girls.begin(), girls.end(), std::back_inserter(females_));
}

void Population::survive() {
   auto impl = [](const decltype(males_)& v) {
       decltype(males_) survivors;
       survivors.reserve(v.size());
       std::copy_if(v.begin(), v.end(), std::back_inserter(survivors),
                    [](const Individual& x) {return x.has_survived();});
       return survivors;
   };
   males_ = impl(males_);
   females_ = impl(females_);
}

std::ostream& Population::write(std::ostream& ost) const {HERE;
    auto impl = [&ost](const decltype(males_)& v) {
        for (const auto& x: v) {
            ost << x << "\n";
        }
    };
    impl(males_);
    impl(females_);
    return ost;
}

//! shortcut Population::write(ost)
std::ostream& operator<<(std::ostream& ost, const Population& pop) {
    return pop.write(ost);
}

void Population::test() {HERE;
    Population x(12);
    std::cout << x << std::endl;
    x.run(15);
    std::cout << x << std::endl;
}

} // namespace pbt
