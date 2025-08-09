#ifndef __OMNISCIENT_AGENT_HPP__
#define __OMNISCIENT_AGENT_HPP__

#include <unordered_map>
#include "agent.hpp"

// Represents an agent that, at interaction time, is aware of all
// departures that have happened since the last interaction and estimates
// the most likely (given its knowledge) maximum value over all active agents.
// Whenever an agent leaves, it is the client's responsibility
// to tell all other active agents about the departure.
// After an agent announces its departure, making it interact
// with other agents is undefined behavior
class OmniscientAgent : public Agent {
private:
    float _y;
    std::unordered_map<int, float> _w;
    float _cachedY;
public:
    // Informs this agent that `leavingAgent` is about to leave.
    // Note that this agent won't incorporate this information
    // into its estimates until its next interaction.
    void acknowledgeDeparture(OmniscientAgent* leavingAgent);
    OmniscientAgent(float x);
    float estimate() const noexcept;
    void interact(OmniscientAgent* other) noexcept;
};

#endif