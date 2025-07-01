#ifndef __THRESHOLD_AGENT_HPP__
#define __THRESHOLD_AGENT_HPP__

#include "agent.hpp"

class ThresholdAgent : public Agent {
private:
    float _y;
    int _t, _threshold;
    void update();
public:
    ThresholdAgent(int x, int id, int threshold);
    float estimate() const noexcept;
    void interact(Agent* that) noexcept;
};

#endif