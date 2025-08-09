#ifndef __THRESHOLD_AGENT_HPP__
#define __THRESHOLD_AGENT_HPP__

#include "agent.hpp"

// Algorithm 2 in paper [1]
class ThresholdAgent : public Agent {
private:
    float _y;
    int _t, _threshold;
    void update();
public:
    ThresholdAgent(float x, int threshold);
    float estimate() const noexcept;
    void interact(ThresholdAgent* other) noexcept;
};

#endif