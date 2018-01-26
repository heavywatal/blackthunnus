/*! @file population.cpp
    @brief Implementation of Population class
*/
#include "population.hpp"
#include "individual.hpp"

#include <wtl/debug.hpp>
#include <wtl/iostr.hpp>
#include <wtl/random.hpp>
#include <sfmt.hpp>

namespace pbt {

Population::Population(const size_t initial_size)
: engine_(std::make_unique<URBG>(wtl::random_device_64{}())) {HERE;
    Individual::set_default_values();
    const size_t half = initial_size / 2UL;
    const size_t rest = initial_size - half;
    males_.reserve(half);
    females_.reserve(rest);
    auto founder = std::make_shared<Individual>();
    for (size_t i=0; i<half; ++i) {males_.emplace_back(new Individual(founder, founder, 0));}
    for (size_t i=0; i<rest; ++i) {females_.emplace_back(new Individual(founder, founder, 0));}
}

Population::~Population() {} // to allow forward declaration of Individual

void Population::run(const uint_fast32_t simulating_duration,
                     const double sample_rate,
                     const uint_fast32_t recording_duration) {HERE;
    auto recording_start = simulating_duration - recording_duration;
    for (year_ = 4u; year_ < simulating_duration; ++year_) {
        DCERR(year_ << ": " << sizes() << std::endl);
        reproduce();
        survive(0u);
        survive(1u);
        survive(2u);
        survive(3u);
        if (year_ >= recording_start) {
            sample(sample_rate);
        }
        migrate();
    }
    DCERR(year_ << ": " << sizes() << std::endl);
}

void Population::reproduce() {
    std::vector<std::shared_ptr<Individual>> boys;
    std::vector<std::shared_ptr<Individual>> girls;
    std::vector<std::vector<std::shared_ptr<Individual>>> males_located(Individual::num_locations());
    for (const auto& p: males_) {
        males_located[p->location()].emplace_back(p);
    }
    for (const auto& mother: females_) {
        if (!mother->is_in_breeding_place()) continue;
        const auto& potential_fathers = males_located[mother->location()];
        if (potential_fathers.empty()) continue;
        const uint_fast32_t num_juveniles = mother->recruitment(year_, *engine_);
        const std::shared_ptr<Individual> father = *wtl::choice(potential_fathers.begin(), potential_fathers.end(), *engine_);
        // TODO: multiple fathers
        for (uint_fast32_t i=0; i<num_juveniles; ++i) {
            if (wtl::generate_canonical(*engine_) < 0.5) {
                boys.emplace_back(new Individual(father, mother, year_));
            } else {
                girls.emplace_back(new Individual(father, mother, year_));
            }
        }
    }
    std::copy(boys.begin(), boys.end(), std::back_inserter(males_));
    std::copy(girls.begin(), girls.end(), std::back_inserter(females_));
}

void Population::survive(const uint_fast32_t quarter) {
    auto impl = [quarter,this](const decltype(males_)& v) {
        decltype(males_) survivors;
        survivors.reserve(v.size());
        std::copy_if(v.begin(), v.end(), std::back_inserter(survivors),
                     [&](const auto& p) {return p->has_survived(year_, quarter, *engine_);});
        return survivors;
    };
    males_ = impl(males_);
    females_ = impl(females_);
}

void Population::migrate() {
    for (auto& p: males_) {p->migrate(year_, *engine_);}
    for (auto& p: females_) {p->migrate(year_, *engine_);}
}

void Population::sample(const double rate) {
    auto impl = [rate,this](decltype(males_)* individuals) {
        decltype(males_) survivors;
        survivors.reserve(individuals->size());
        std::vector<std::vector<size_t>> adults(Individual::num_breeding_places());
        std::vector<std::vector<size_t>> juveniles(Individual::num_breeding_places());
        size_t num_adults = 0u;
        for (size_t i=0; i<individuals->size(); ++i) {
            auto& p = individuals->at(i);
            if (p->is_in_breeding_place()) {
                if (p->birth_year() == year_) {
                    juveniles[p->location()].emplace_back(i);
                } else {
                    adults[p->location()].emplace_back(i);
                    ++num_adults;
                }
            } else {
                survivors.emplace_back(p);
            }
        }
        decltype(males_) samples;
        samples.reserve(static_cast<size_t>(std::round(3.0 * rate * num_adults)));
        auto sort = [&](const std::vector<size_t>& indices, const size_t k) {
            const auto chosen = wtl::sample(indices.size(), k, *engine_);
            for (size_t i=0; i<indices.size(); ++i) {
                if (chosen.find(i) == chosen.end()) {
                    survivors.emplace_back(individuals->at(indices[i]));
                } else {
                    samples.emplace_back(individuals->at(indices[i]));
                }
            }
        };
        for (uint_fast32_t loc=0; loc<Individual::num_breeding_places(); ++loc) {
            const size_t n_adults = adults[loc].size();
            const size_t n_adult_samples = static_cast<size_t>(std::round(rate * n_adults));
            const size_t n_juvenile_samples = 2u * n_adult_samples;
            sort(adults[loc], n_adult_samples);
            sort(juveniles[loc], n_juvenile_samples);
        }
        individuals->swap(survivors);
        return samples;
    };
    auto m_samples = impl(&males_);
    auto f_samples = impl(&females_);
    auto& samples = year_samples_[year_];
    samples.reserve(m_samples.size() + f_samples.size());
    std::copy(m_samples.begin(), m_samples.end(), std::back_inserter(samples));
    std::copy(f_samples.begin(), f_samples.end(), std::back_inserter(samples));
}

std::ostream& Population::write_sample_header(std::ostream& ost) const {
    if (year_samples_.empty()) return ost;
    return wtl::join(Individual::names(), ost, "\t") << "\tcapture_year\n";
}

std::ostream& Population::write_sample(std::ostream& ost) const {
    write_sample_header(ost);
    for (const auto& ys: year_samples_) {
        for (const auto& p: ys.second) {
            ost << *p << "\t" << ys.first << "\n";
        }
    }
    return ost;
}

std::ostream& Population::write_sample_family(std::ostream& ost) const {
    std::unordered_map<uint_fast32_t, uint_fast32_t> id_year;
    std::map<uint_fast32_t, Individual*> nodes;
    for (const auto& ys: year_samples_) {
        for (const auto& p: ys.second) {
            p->trace_back(&nodes);
            id_year.emplace(p->id(), ys.first);
        }
    }
    write_sample_header(ost);
    for (const auto& p: nodes) {
        ost << *p.second << "\t";
        auto it = id_year.find(p.first);
        if (it != id_year.end()) {
            ost << it->second;
        }
        ost << "\n";
    }
    return ost;
}

std::vector<size_t> Population::sizes() const {
    std::vector<size_t> counter(Individual::num_locations(), 0u);
    for (const auto& p: males_) {++counter[p->location()];}
    for (const auto& p: females_) {++counter[p->location()];}
    return counter;
}

std::ostream& Population::write(std::ostream& ost) const {
    for (const auto& p: males_) {ost << *p << "\n";}
    for (const auto& p: females_) {ost << *p << "\n";}
    return ost;
}

//! shortcut Population::write(ost)
std::ostream& operator<<(std::ostream& ost, const Population& pop) {
    return pop.write(ost);
}

} // namespace pbt
