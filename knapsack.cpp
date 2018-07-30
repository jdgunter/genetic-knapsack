#include <vector>
#include <iostream>
#include <random>
#include <algorithm>

struct Item {
    const int value;
    const int weight;
    Item(int v, int w) : value(v), weight(w) {}
};

std::ostream& operator<<(std::ostream& stream, Item item) {
    stream << "{value: " << item.value << ", weight: " << item.weight << "}";
    return stream;
}

using Individual = std::vector<bool>;
using Population = std::vector<Individual>;

struct KnapsackSolver {
    const std::vector<Item> items;
    const int capacity;
    const int popSize;
    const int maxIterations;
    Population generation;
    
    std::mt19937 rng;
    std::uniform_int_distribution<int> breedingDistribution;
    std::uniform_int_distribution<int> populationDistribution;

    KnapsackSolver(const std::vector<Item>& its, int cap, int pop, int maxit) 
    : items(its), 
      capacity(cap),
      popSize(pop),
      maxIterations(maxit),
      rng((std::random_device())()),  
      breedingDistribution(0, items.size() - 1),
      populationDistribution(0, pop-1) 
      { }

    int fitness(const Individual& ind) const;
    Individual mutate(const Individual& parent);
    Individual crossover(const Individual& parent1, const Individual& parent2);

    Population generateInitialPop();
    Individual generateIndividual();
    Population naturalSelection(const Population& pop) const;
    Population breed(const Population& pop);
    Individual solve();
};

/**
 * Computes the fitness of a given individual
 */
int KnapsackSolver::fitness(const Individual& ind) const {
    int fit = 0;
    int weight = 0;
    for (int i = 0, end = items.size(); i < end; ++i) {
        fit += items[i].value * ind[i];
        weight += items[i].weight * ind[i];
    }
    if (weight > capacity) {
        fit = 0;
    }
    return fit;
}

/**
 * mutates an individual, creating a new individual with a random bit flipped
 */
Individual KnapsackSolver::mutate(const Individual& parent) {
    Individual child(parent);
    int mutatingIndex = breedingDistribution(rng);
    child[mutatingIndex] = !child[mutatingIndex];
    return child;
}

/**
 * breeds two individuals together by choosing a random crossover index c, taking all values
 * before c from the first parent and all values after c from the second parent
 */
Individual KnapsackSolver::crossover(const Individual& parent1, const Individual& parent2) {
    Individual child(parent1.size());
    int crossoverIndex = breedingDistribution(rng);
    for (int i = 0; i < crossoverIndex; ++i) {
        child[i] = parent1[i];
    }
    for (std::size_t i = crossoverIndex; i < parent2.size(); ++i) {
        child[i] = parent2[i];
    }
    return child;
}

/**
 * generates a random individual, for use in initializing the population
 */
Individual KnapsackSolver::generateIndividual() {
    int weight = 0;
    int len = items.size();
    Individual ind(len, false);
    for (int i = 0; i < len; ++i) {
        bool bit = breedingDistribution(rng) % 2; // ~50% chance to be on or off
        if (bit) {
            weight += items[i].weight;
            if (weight > capacity) {
                break; // quit early if we go over capacity
            }
            ind[i] = true;
        }
    }
    return ind;
}

/**
 * generates an initial random population
 */
Population KnapsackSolver::generateInitialPop() {
    Population pop;
    for (int i = 0; i < popSize; ++i) {
        pop.push_back(generateIndividual());
    }
    return pop;
}

/**
 * takes a population and culls all unfit individuals, preserving the most
 * fit for future breeding
 */
Population KnapsackSolver::naturalSelection(const Population& pop) const {
    Population newPop(pop);
    // sort population in descending order by fitness
    std::sort(newPop.begin(), newPop.end(), [this](const auto& lhs, const auto& rhs) {
        return (this->fitness(lhs) > this->fitness(rhs));
    });
    return Population(newPop.begin(), newPop.begin() + popSize);
}

/**
 * breeds a new generation by mutating every individual and breeding each individual
 * with another random individual, appending these to the original population
 */
Population KnapsackSolver::breed(const Population& pop) {
    Population newPop(pop);
    newPop.resize(popSize * 3);
    for (auto& individual : pop) {
        newPop.push_back(mutate(individual));
        newPop.push_back(crossover(individual, pop[populationDistribution(rng)]));
    }
    return newPop;
}

/**
 * run the genetic algorithm, outputting the fittest individual found
 */
Individual KnapsackSolver::solve() {
    Population nextGen;
    generation = generateInitialPop();
    for (int i = 0; i < maxIterations; ++i) {
        nextGen = breed(generation);
        generation = naturalSelection(nextGen);
    } 
    return generation[0];
}

/**
 * generate a random sample problem using a given seed
 */
std::vector<Item> generateKnapsackProblem(int size, int seed) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(1,100);
    std::vector<Item> items;
    for (int i = 0; i < size; ++i) {
        items.push_back(Item(dist(rng), dist(rng)));
    }
    return items;
}

int main() {
    std::vector<Item> items = generateKnapsackProblem(50, 57);
    KnapsackSolver solver(items, 500, 30, 50000);
    Individual solution = solver.solve();
    
    int totalValue = 0, totalWeight = 0;
    std::cout << "Initial item set:\n";
    for (auto item : items) {
        std::cout << item << "\n";
    }
    std::cout << "\nThe items chosen are:\n";
    for (std::size_t i = 0; i < solution.size(); ++i) {
        if (solution[i]) {
            std::cout << items[i] << "\n";
            totalValue += items[i].value;
            totalWeight += items[i].weight;
        }
    } 
    std::cout << "For a total value of " << totalValue << " and a total weight of " << totalWeight << "\n";
    return 0;
}