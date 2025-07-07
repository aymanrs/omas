#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <utility>
#include <random>
#include "pcg_random.hpp"

extern std::uniform_real_distribution<float> urd;
extern thread_local pcg32_fast rng;
std::pair<int, int> randomPair(int n);
int randomDifferentIndex(int n, int i);
struct Curve {
    std::vector<float> x, y;
    std::string label; // empty string indicates no label
};
void savePlot(const std::vector<Curve>& curves, const std::string& filename, const std::string& title = "", const std::string& xlabel = "", const std::string& ylabel = "");
float sampleExpDist(float tau);

#endif