#ifndef __TWO_MAX_AGENT_HPP__
#define __TWO_MAX_AGENT_HPP__

#include "agent.hpp"

// Proposed Algorithm.
// Maintains the 2 biggest encountered values
// as well as estimates for the avg IPL and avg N.
// Explicitly computes the probability that each value
// is still present and estimates the most likely maximum
class TwoMaxAgent : public Agent {
private:
    float _y, _y2;
    int _t, _t2;
    float _avgInteractionsAs2Max, _avgInteractions;
    void update();
public:
    TwoMaxAgent(float x);
    float estimate() const noexcept;
    void interact(TwoMaxAgent* other) noexcept;
};

#endif