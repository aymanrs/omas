#ifndef __AGENT_HPP__
#define __AGENT_HPP__

#include <string>
#include <unordered_map>

class Agent {
protected:
    float _x;
    Agent(float x, int id) : _x(x), _id(id) {}
    const int _id;
public:
    template<typename AgentType>
    static Agent* makeAgent(float x, int id, int threshold = 0, const float* time=nullptr, float rho=0, float lamdaDelta=0, std::unordered_map<int, float>* arrivals=nullptr);
    virtual void interact(Agent* that) noexcept = 0;
    virtual void leave(Agent* agentToInform = nullptr) {};
    virtual float estimate() const noexcept = 0;
    float value() const {
        return _x;
    }
    int id() const {
        return _id;
    }
    virtual ~Agent(){};
};

#endif