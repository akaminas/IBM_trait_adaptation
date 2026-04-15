
```cpp
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

struct Individual {
    double trait = 0.0;
    double fitness = 1.0;
};

class Simulation {
public:
    Simulation(
        std::size_t population_size,
        std::size_t generations,
        double sigma_environment,
        double sigma_mutation,
        double environment_shift,
        std::size_t burn_in,
        unsigned int seed = std::random_device{}())
        : N(population_size),
          T(generations),
          sigma_e(sigma_environment),
          sigma_l(sigma_mutation),
          env_shift(environment_shift),
          burn_in_time(burn_in),
          rng(seed),
          uniform01(0.0, 1.0),
          normal01(0.0, 1.0) {
        if (N == 0 || T == 0) {
            throw std::invalid_argument("Population size and generations must be positive.");
        }
        population.resize(N);
    }

    void run(const std::string& output_path) {
        initialise_population();

        std::ofstream csv(output_path);
        if (!csv) {
            throw std::runtime_error("Failed to open output file: " + output_path);
        }

        csv << "time,avg_trait,avg_fitness,environment\n";
        std::cout << "time,avg_trait,avg_fitness,environment\n";

        for (std::size_t t = 0; t < T; ++t) {
            update_environment(t);
            evaluate_fitness();

            auto [avg_trait, avg_fitness] = compute_summary();

            csv << t << ','
                << avg_trait << ','
                << avg_fitness << ','
                << environment << '\n';

            std::cout << t << ','
                      << avg_trait << ','
                      << avg_fitness << ','
                      << environment << '\n';

            reproduce_next_generation();
        }
    }

private:
    const std::size_t N;
    const std::size_t T;
    const double sigma_e;
    const double sigma_l;
    const double env_shift;
    const std::size_t burn_in_time;

    double environment = 0.0;
    std::vector<Individual> population;

    std::mt19937 rng;
    std::uniform_real_distribution<double> uniform01;
    std::normal_distribution<double> normal01;

    static constexpr double trait_min = -5.0;
    static constexpr double trait_max = 5.0;

    double rand_uniform() {
        return uniform01(rng);
    }

    double rand_normal(double mean, double sd) {
        return mean + sd * normal01(rng);
    }

    void initialise_population() {
        for (auto& ind : population) {
            ind.trait = rand_uniform();
            ind.fitness = 1.0;
        }
    }

    void update_environment(std::size_t t) {
        if (t >= burn_in_time) {
            environment += env_shift + rand_normal(0.0, sigma_e);
        }

        if (!std::isfinite(environment)) {
            environment = 0.0;
        }
    }

    static double compute_fitness(double trait, double env) {
        const double diff = trait - env;
        const double squared = diff * diff;

        if (squared > 100.0) {
            return 0.0;
        }

        return std::exp(-squared);
    }

    void evaluate_fitness() {
        double total_fitness = 0.0;

        for (auto& ind : population) {
            ind.fitness = compute_fitness(ind.trait, environment);
            total_fitness += ind.fitness;
        }

        if (total_fitness <= 0.0 || !std::isfinite(total_fitness)) {
            for (auto& ind : population) {
                ind.fitness = 1.0;
            }
        }
    }

    std::pair<double, double> compute_summary() const {
        const double total_trait = std::accumulate(
            population.begin(), population.end(), 0.0,
            [](double sum, const Individual& ind) {
                return sum + ind.trait;
            });

        const double total_fitness = std::accumulate(
            population.begin(), population.end(), 0.0,
            [](double sum, const Individual& ind) {
                return sum + ind.fitness;
            });

        return {total_trait / static_cast<double>(N),
                total_fitness / static_cast<double>(N)};
    }

    std::vector<double> build_cdf() const {
        std::vector<double> cdf(N);
        cdf[0] = population[0].fitness;

        for (std::size_t i = 1; i < N; ++i) {
            cdf[i] = cdf[i - 1] + population[i].fitness;
        }

        return cdf;
    }

    std::size_t choose_parent(const std::vector<double>& cdf) {
        const double total_fitness = cdf.back();
        const double draw = rand_uniform() * total_fitness;
        const auto it = std::lower_bound(cdf.begin(), cdf.end(), draw);
        return static_cast<std::size_t>(std::distance(cdf.begin(), it));
    }

    Individual reproduce(const Individual& parent) {
        Individual child;
        child.trait = parent.trait + rand_normal(0.0, sigma_l);
        child.trait = std::clamp(child.trait, trait_min, trait_max);

        if (!std::isfinite(child.trait)) {
            child.trait = 0.0;
        }

        child.fitness = 1.0;
        return child;
    }

    void reproduce_next_generation() {
        const auto cdf = build_cdf();
        std::vector<Individual> next_population(N);

        for (auto& ind : next_population) {
            const std::size_t parent_index = choose_parent(cdf);
            ind = reproduce(population[parent_index]);
        }

        population = std::move(next_population);
    }
};

int main() {
    try {
        const std::size_t population_size = 10000;
        const std::size_t generations = 1000;
        const double sigma_environment = 0.1;
        const double sigma_mutation = 0.05;
        const double environment_shift = 0.001;
        const std::size_t burn_in = 100;

        Simulation sim(
            population_size,
            generations,
            sigma_environment,
            sigma_mutation,
            environment_shift,
            burn_in
        );

        sim.run("data/trait_output.csv");
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
