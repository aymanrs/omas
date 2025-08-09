#ifndef __AGENT_HPP__
#define __AGENT_HPP__

#include <concepts>


// Base class representing a generic agent
// Any valid agent class should satisfy the requirements
// of the `ValidAgentType` concept
class Agent {
private:
    static thread_local int _idCounter;
protected:
    const int _id;
    const float _x;
    Agent(float x);
public:
    // Generic function for creating new agents.
    // Supports: AnnounceAgent, TwoMaxAgent, OptimalAgent, ThresholdAgent, and OmniscientAgent
    template<typename AgentType>
    static AgentType* makeAgent(float x, int threshold = 10);

    // Returns the internal value of this agent
    float value() const;

    // Returns this agent's estimate for the desired quantity
    // The result of a call to `estimate()` may only change
    // after a call to a non-const method of the agent
    virtual float estimate() const noexcept = 0;
    
    // Returns the ID of this agent
    // All agent instances within the same thread have distinct IDs
    // IDs are automatically generated and are threadsafe
    int id() const;
    
    virtual ~Agent();
};

template<typename AgentType>
concept ValidAgentType = requires(AgentType* a, AgentType* b) {
    requires std::derived_from<AgentType, Agent>;
    { a->interact(b) } -> std::same_as<void>;
};

#endif