#ifndef __ANNOUNCE_AGENT_HPP__
#define __ANNOUNCE_AGENT_HPP__

#include "agent.hpp"

class AnnounceAgent : public Agent {
private:
    float _y;
    int _k;
public:
    AnnounceAgent(float x, int id);
    void interact(Agent* that) noexcept;
    float estimate() const noexcept;
    void leave(Agent* agentToInform = nullptr);
};

#endif