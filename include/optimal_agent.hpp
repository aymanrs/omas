#ifndef __OPTIMAL_AGENT_HPP__
#define __OPTIMAL_AGENT_HPP__

#include <unordered_map>
#include <cmath>
#include "pcg_random.hpp"
#include "agent.hpp"

static const float NaN = nanf("");

// IMPORTANT: Maintaining time and arrivals is the responsibility of the client
// This is designed like this to allow parallelism
class OptimalAgent : public Agent {
private:
    std::unordered_map<int, std::pair<int, float>> _w;
    std::unordered_map<float, int> _reach;
    mutable float _y = NaN;
    int _freeTimeZeroSlots;
    void modifyReach(float t, int change) noexcept;
    const float* _time;
    float _rho;
    float _lambdaDelta;
    std::unordered_map<int, float>* _arrivals;
public:
    OptimalAgent(float x, int id, const float* time, float rho, float lambdaDelta, std::unordered_map<int, float>* arrivals);
    static float gamma;
    float estimate() const noexcept;
    void interact(Agent* that) noexcept;
};

#endif