#ifndef __TWO_MAX_AGENT_HPP__
#define __TWO_MAX_AGENT_HPP__

#include "agent.hpp"

class TwoMaxAgent : public Agent {
private:
    float _y, _y2;
    int _t, _t2, _threshold;
    void update();
public:
    TwoMaxAgent(int x, int threshold);
    float estimate();
    void interact(Agent* that);
};

#endif