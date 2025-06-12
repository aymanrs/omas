#ifndef __OPTIMAL_AGENT_HPP__
#define __OPTIMAL_AGENT_HPP__

#include <unordered_map>
#include <cmath>
#include "agent.hpp"

static float NaN = nanf("");

class OptimalAgent : public Agent {
private:
    std::unordered_map<OptimalAgent*, std::pair<int, int>> _w;
    float _y = NaN;
public:
    static int n;
    static int time;
    static float rho;
    OptimalAgent(int x);
    float estimate();
    void interact(Agent* that);
    void leave(Agent* agentToInform = nullptr);
};

#endif