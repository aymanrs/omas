#ifndef __RHO_GUESS_AGENT_HPP__
#define __RHO_GUESS_AGENT_HPP__

#include "agent.hpp"

class RhoGuessAgent : public Agent {
public:
    float _y, _t;
    float _interactions;
    float _weight;
    float _avgInteractions;
    void update();
public:
    RhoGuessAgent(float x, int id);
    float estimate() const noexcept;
    void interact(Agent* that) noexcept;
};

#endif