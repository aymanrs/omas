#ifndef __AGENT_HPP__
#define __AGENT_HPP__

#include <string>

enum class AgentType {
    Announce,
    Threshold,
    TwoMax,
    Optimal
};

class Agent {
protected:
    int _x;
    Agent() {
        _id = idCounter++;
    }
public:
    int _id;
    static int idCounter;
    static Agent* makeAgent(AgentType type, int x, int threshold = 0);
    virtual void interact(Agent* that) = 0;
    virtual void leave(Agent* agentToInform = nullptr) {};
    virtual float estimate() const = 0;
    int value() const {
        return _x;
    }
    virtual ~Agent(){};
};

#endif