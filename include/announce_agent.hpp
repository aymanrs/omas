#ifndef __ANNOUNCE_AGENT_HPP__
#define __ANNOUNCE_AGENT_HPP__

#include "agent.hpp"

class AnnounceAgent : public Agent {
private:
    float _y;
    int _k;
public:
    AnnounceAgent(int x);
    void interact(Agent* that);
    float estimate() const;
    void leave(Agent* agentToInform = nullptr);
};

#endif