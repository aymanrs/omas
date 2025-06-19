#include "optimal_agent.hpp"
#include <list>
#include <cmath>
#include <algorithm>
#ifdef DEBUG
#include <stdexcept>
#endif
#include "utils.hpp"

float OptimalAgent::time = 0;
float OptimalAgent::rho = 0;
float OptimalAgent::gamma = 0.003;
std::unordered_map<int, float> OptimalAgent::arrivals;
OptimalAgent::OptimalAgent(int x) {
    _x = x;
    _w[_id] = {x, time};
    arrivals[_id] = time;
}

// Since we loop through the whole _reach map during each estimation,
// it makes sense to remove the zeroes which are useless
void OptimalAgent::modifyReach(float t, int change) {
    if(!(_reach[t] += change)) _reach.erase(t);
}

void OptimalAgent::interact(Agent* that) {
    OptimalAgent* other = reinterpret_cast<OptimalAgent*>(that);
    if(other == this) {
        _y = NaN;
        this->_arrivalsKnownAsOfLastInteraction = arrivals.size()-_w.size();
        return;
    }
    _y = NaN;
    other->_y = NaN;
    modifyReach(_w[_id].second, -1);
    modifyReach(time, +1);
    _w[_id].second = time;
    other->modifyReach(other->_w[other->_id].second, -1);
    other->modifyReach(time, +1);
    other->_w[other->_id].second = time;
    for(auto [id, valueAndTime] : _w){
        auto it = other->_w.find(id);
        if(it == other->_w.end()) {
            auto arit = arrivals.find(id);
            other->modifyReach(valueAndTime.second, +1);
            other->modifyReach(arit->second, -1);
            other->_w[id] = valueAndTime;
        } else if(it->second.second < valueAndTime.second) {
            other->modifyReach(it->second.second, -1);
            other->modifyReach(valueAndTime.second, +1);
            it->second = valueAndTime;
        }
    }
    for(auto [id, valueAndTime] : other->_w){
        auto it = _w.find(id);
        if(it == _w.end()) {
            auto arit = arrivals.find(id);
            modifyReach(valueAndTime.second, +1);
            modifyReach(arit->second, -1);
            _w[id] = valueAndTime;
        } else if(it->second.second < valueAndTime.second) {
            modifyReach(it->second.second, -1);
            modifyReach(valueAndTime.second, +1);
            it->second = valueAndTime;
        }
    }
    _arrivalsKnownAsOfLastInteraction = arrivals.size()-_w.size();
    other->_arrivalsKnownAsOfLastInteraction = arrivals.size()-other->_w.size();
}
float OptimalAgent::estimate() const {
    if(!isnanf(_y)) return _y;
    float localTime = _w.at(_id).second;
    float lnRhoBar = log(1-rho);
    float lnGammaBar = log(1-gamma);
    float pUnreachedGivenStayed = 1, pUnreached = 1, pStayedGivenUnreached = 1;
    int nReached = 0;
    // let x be the integer
    // if x >= 0, this pair encodes some agent's last interaction
    // and x is the value of that agent
    // if x < 0, this pair encodes some change in the number of reached
    // agents, and x+10000 is the actual change
    std::vector<std::pair<float, int>> events; events.reserve(_reach.size()+arrivals.size());
    for(auto [time, x] : _reach) if(x) events.emplace_back(time, x-10000);
    for(auto [_, valueAndTime] : _w) {
        if(valueAndTime.first > _x)
            events.emplace_back(valueAndTime.second, valueAndTime.first);
    }
    int availableSlots = _arrivalsKnownAsOfLastInteraction;
    while(availableSlots--){
        int x = rng()%1000; 
        if(x > _x)
            events.emplace_back(urd(rng)*localTime, x);
    }
    std::sort(events.begin(), events.end(), std::greater<std::pair<float, int>>());
    float prevT = localTime;
    int max = _x;
    for(auto [t, x] : events) {
        if(t > prevT) {
            float pStayed = pow(1-rho, prevT-t); // probability of staying until prevT
            float pUGS = pow(1-gamma, nReached*(prevT-t)); // probability of not being reached until prevT given that it stayed until then
            float pUandL = -lnRhoBar*(pStayed*pUGS-1)/(lnRhoBar+nReached*lnGammaBar);
            // The use of pStayed and pUGS is above just to avoid recomputing them,
            // this is not a valid formula by itself
            pUnreached = pUandL + pUnreached*pUGS*pStayed;
            pUnreachedGivenStayed *= pUGS;
            pStayedGivenUnreached = pUnreached == 0 ? 0 : pow(1-rho, localTime-t) * pUnreachedGivenStayed / pUnreached;
        }
        if(x >= 0) {
            if(urd(rng) < pStayedGivenUnreached)
                max = std::max(max, x);
        } else {
            nReached += x+10000;
        }
    }
    return _y = max;
}

void OptimalAgent::leave(Agent* agentToInform) {
    _w.clear();
    _x = rng() % 1000;
    _w[_id] = {_x, time};
    _reach.clear();
}