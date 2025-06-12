#ifndef __THRESHOLD_AGENT_HPP__
#define __THRESHOLD_AGENT_HPP__

#include "agent.hpp"

class ThresholdAgent : public Agent {
private:
    float _y;
    int _t, _threshold;
    void update();
public:
    ThresholdAgent(int x, int threshold);
    float estimate();
    void interact(Agent* that);
};

#endif