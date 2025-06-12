#include <list>
#include <cmath>
#include "utils.hpp"
#include "optimal_agent.hpp"

int OptimalAgent::time = 0;
float OptimalAgent::rho = 0;
int OptimalAgent::n = 0;

OptimalAgent::OptimalAgent(int x) {
    _x = x;
    _w[this] = {x, time};
}

void OptimalAgent::interact(Agent* that) {
    OptimalAgent* other = reinterpret_cast<OptimalAgent*>(that);
    _y = NaN;
    other->_y = NaN;
    _w[this].second = time;
    other->_w[other].second = time;
    std::list<OptimalAgent*> keysToRemove;
    for(auto [agent, valueAndTime] : _w){
        auto it = other->_w.find(agent);
        if(it == other->_w.end()) other->_w[agent] = valueAndTime;
        else if(it->second.second < valueAndTime.second) {
            it->second = valueAndTime;
        }
        if((time-valueAndTime.second)*rho >= 0.98) keysToRemove.push_back(agent); 
    }
    for(auto [agent, valueAndTime] : other->_w){
        auto it = _w.find(agent);
        if(it == _w.end()) _w[agent] = valueAndTime;
        else if(it->second.second < valueAndTime.second) {
            it->second = valueAndTime;
        }
        if((time-valueAndTime.second)*rho >= 0.98) keysToRemove.push_back(agent); 
    }
    for(OptimalAgent* keyToRemove : keysToRemove){
        auto it = _w.find(keyToRemove);
        if(it != _w.end()) _w.erase(it);
        it = other->_w.find(keyToRemove);
        if(it != other->_w.end()) other->_w.erase(it);
    }
}

float OptimalAgent::estimate() {
    if(_y != NaN) return _y;
    float max = _x;
    for(auto [agent, valueAndTime] : _w){
        if(agent == this) continue;
        auto [value, t] = valueAndTime;
        t = time-t;
        if(urd(rng) < pow(1-rho, t)) {
            max = std::max(max, float(value));
        } else {
            max = std::max(max, float(rng()%1000));
        }
    }
    for(int _ = 0;_ < n-_w.size();_++) max = std::max(max, float(rng()%1000));
    _y = max;
    return _y;
}

void OptimalAgent::leave(Agent* agentToInform) {
    _w.clear();
    _x = rng() % 1000;
    _w[this] = {_x, time};
}