#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <random>
#include "pcg_random.hpp"

// Uniform real distribution in the range [0, 1[
extern std::uniform_real_distribution<float> urd;

// Threadsafe fast 32-bit random number generator
extern thread_local pcg32_fast rng;

// Given an integer n such that n > 2, returns
// a pair of distinct integers both between 0 and n-1
// inclusive. This pair is chosen uniformly at random,
// using the provided `rng` for random number generation.
std::pair<int, int> randomPair(int n);

// Given an integer n such that n > 2
// and an integer i such that 0 <= i <= n-1,
// returns an integer between 0 and n-1 inclusive
// that is not equal to i, chosen uniformly at random
// using the provided `rng` for random number generation
int randomDifferentIndex(int n, int i);

// Represents a plotting curve
struct Curve {
    std::vector<float> x, y;
    std::string label; // empty string indicates no label
};

// Saves a plot consisting of at least one curve to
// the file indicated by `filename`. Running the "plot.py"
// script with that file as an argument displays the plot
void savePlot(const std::vector<Curve>& curves, const std::string& filename, const std::string& title = "", const std::string& xlabel = "", const std::string& ylabel = "");

// Samples from an exponential distribution with rate 1/tau
// and returns the result (if tau is 0, always returns 0).
// In particular, the mean is tau.
// the provided `rng` is used for random number generation
float sampleExpDist(float tau);

#endif