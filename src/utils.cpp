#include <fstream>
#ifdef DEBUG
#include <stdexcept>
#endif
#include "utils.hpp"
#include "agent.hpp"

pcg32_fast rng(pcg_extras::seed_seq_from<std::random_device>{});
std::uniform_real_distribution<float> urd(0, 1);

std::pair<int, int> randomPair(int n) {
#ifdef DEBUG
    if(n < 2) {
        throw std::domain_error("To generate a random pair, n has to be >= 2");
    }
#endif
    int i = rng()%n;
    int j = rng()%(n-1);
    if(j >= i) j++;
#ifdef DEBUG
    if(i == j) {
        throw std::runtime_error("The generated indices aren't distinct");
    }
#endif
    return std::make_pair(i, j);

}
int randomDifferentIndex(int n, int i) {
#ifdef DEBUG
    if(n < 2) {
        throw std::domain_error("To generate a random pair, n has to be >= 2");
    }
    if(i >= n){
        throw std::runtime_error("Invalid index");
    }
#endif
    int j = rng()%(n-1);
    if(j >= i) j++;
    return j;
}


void savePlot(const std::vector<Curve>& curves, const std::string& filename, const std::string& title, const std::string& xlabel, const std::string& ylabel) {
    std::ofstream outputFile(filename);
    outputFile << title << '\n';
    outputFile << xlabel << '\n';
    outputFile << ylabel << '\n';
    outputFile << curves.size() << '\n';
    for(const Curve& curve : curves){
#ifdef DEBUG
        if(curve.x.size() != curve.y.size()) {
            throw std::runtime_error("Each x value should correspond to a y value");
        }
#endif
        outputFile << curve.label << '\n';
        for(int i = 0;i < curve.x.size();i++) outputFile << (i?" " : "") << curve.x[i];
        outputFile << '\n';
        for(int i = 0;i < curve.y.size();i++) outputFile << (i?" " : "") << curve.y[i];
        outputFile << '\n';
    }
}

float sampleExpDist(float tau){
    return -log(1-urd(rng))*tau;
}