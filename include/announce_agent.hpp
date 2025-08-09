#ifndef __ANNOUNCE_AGENT_HPP__
#define __ANNOUNCE_AGENT_HPP__

#include "agent.hpp"

// Represents an agent that may announce its departure
// to other active agents. It is the client's responsibility to inform
// other active agents (possibly none or all) about any departure.
// Note: after an agent announces its departure, making it interact
// with other agents is undefined behavior
class AnnounceAgent : public Agent {
private:
    float _y;
    int _k;
public:
    AnnounceAgent(float x);
    void interact(AnnounceAgent* other) noexcept;
    float estimate() const noexcept;
    // Informs this agent that `leavingAgent` is about to leave.
    // This may immediately affect the result of `estimate()`
    void acknowledgeDeparture(AnnounceAgent* leavingAgent);
};

#endif