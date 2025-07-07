#include "threshold_agent.hpp"
#include "announce_agent.hpp"
#include "twomax_agent.hpp"
#include "optimal_agent.hpp"
#include "rho_guess_agent.hpp"
#ifdef DEBUG
#include <stdexcept>
#endif

template<>
Agent* Agent::makeAgent<TwoMaxAgent>(float x, int id, int threshold, const float* time, float rho, float lambdaDelta, std::unordered_map<int, float>* arrivals) {
    return new TwoMaxAgent(x, id, threshold);
}

template<>
Agent* Agent::makeAgent<RhoGuessAgent>(float x, int id, int threshold, const float* time, float rho, float lambdaDelta, std::unordered_map<int, float>* arrivals) {
    return new RhoGuessAgent(x, id);
}
template<>
Agent* Agent::makeAgent<ThresholdAgent>(float x, int id, int threshold, const float* time, float rho, float lambdaDelta, std::unordered_map<int, float>* arrivals) {
    return new ThresholdAgent(x, id, threshold);
}
template<>
Agent* Agent::makeAgent<OptimalAgent>(float x, int id, int threshold, const float* time, float rho, float lambdaDelta, std::unordered_map<int, float>* arrivals) {
    return new OptimalAgent(x, id, time, rho, lambdaDelta, arrivals);
}