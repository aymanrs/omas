#ifndef __TWO_MAX_AGENT_HPP__
#define __TWO_MAX_AGENT_HPP__

#include "agent.hpp"

class TwoMaxAgent : public Agent {
private:
    float _y, _y2;
    int _t, _t2, _threshold, _interactions;
    void update();
public:
    TwoMaxAgent(float x, int id, int threshold);
    float estimate() const noexcept;
    void interact(Agent* that) noexcept;
};

#endif