#ifndef __OPTIMAL_AGENT_HPP__
#define __OPTIMAL_AGENT_HPP__

#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <bitset>
#include "agent.hpp"

static const float NaN = nanf("");
static const int NaNi = -1;

class OptimalAgent : public Agent {
private:
    std::unordered_map<int, std::pair<int, float>> _w;
    std::unordered_map<float, int> _reach;
    mutable float _y = NaN;
    int _arrivalsKnownAsOfLastInteraction;
    void modifyReach(float t, int change);
    public:
    OptimalAgent(int x);
    static float time;
    static float rho;
    static float gamma;
    static std::unordered_map<int, float> arrivals;
    float estimate() const;
    void interact(Agent* that);
    void leave(Agent* agentToInform = nullptr);
};

#endif