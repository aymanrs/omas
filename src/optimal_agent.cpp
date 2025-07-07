#include "optimal_agent.hpp"
#include <list>
#include <cmath>
#include <algorithm>
#ifdef DEBUG
#include <stdexcept>
#endif
#include "utils.hpp"

float fastPow(float x, int n){
    float r = 1;
    while(n) {
        if(n&1) r *= x;
        x *= x;
        n >>= 1;
    }
    return r;
}

float OptimalAgent::gamma = 0.003;
OptimalAgent::OptimalAgent(float x, int id, const float* time, float rho, float lambdaDelta, std::unordered_map<int, float>* const arrivals)
: Agent(x, id) {
    _time = time;
    _rho = rho;
    _lambdaDelta = lambdaDelta;
    _arrivals = arrivals;
    _w[_id] = {x, *_time};
    _arrivals->insert({_id, *_time});
    _freeTimeZeroSlots = 25-(*_time == 0);
}

// Since we loop through the whole _reach map during each estimation,
// it makes sense to remove the zeroes which are useless
void OptimalAgent::modifyReach(float t, int change) noexcept {
    if(!(_reach[t] += change)) _reach.erase(t);
}

void OptimalAgent::interact(Agent* that) noexcept {
    OptimalAgent* other = reinterpret_cast<OptimalAgent*>(that);
    _y = NaN;
    other->_y = NaN;
    modifyReach(_w[_id].second, -1);
    modifyReach(*_time, +1);
    _w[_id].second = *_time;
    other->modifyReach(other->_w[other->_id].second, -1);
    other->modifyReach(*_time, +1);
    other->_w[other->_id].second = *_time;
    for(auto [id, valueAndTime] : _w){
        auto it = other->_w.find(id);
        if(it == other->_w.end()) {
            auto arit = _arrivals->find(id);
            other->modifyReach(valueAndTime.second, +1);
            other->modifyReach(arit->second, -1);
            other->_w[id] = valueAndTime;
            if(arit->second == 0) other->_freeTimeZeroSlots--;
        } else if(it->second.second < valueAndTime.second) {
            other->modifyReach(it->second.second, -1);
            other->modifyReach(valueAndTime.second, +1);
            it->second = valueAndTime;
        }
    }
    for(auto [id, valueAndTime] : other->_w){
        auto it = _w.find(id);
        if(it == _w.end()) {
            auto arit = _arrivals->find(id);
            modifyReach(valueAndTime.second, +1);
            modifyReach(arit->second, -1);
            _w[id] = valueAndTime;
            if(arit->second == 0) _freeTimeZeroSlots--;
        } else if(it->second.second < valueAndTime.second) {
            modifyReach(it->second.second, -1);
            modifyReach(valueAndTime.second, +1);
            it->second = valueAndTime;
        }
    }
}
float OptimalAgent::estimate() const noexcept {
    if(!isnanf(_y)) return _y;
    float localTime = _w.at(_id).second;
    std::mt19937 rng(_id*20000+int(localTime));
    float lnRhoBar = log(1-_rho);
    float lnGammaBar = log(1-gamma);
    float pStayedGivenUnreached = 1, pUnreachedAndStayed = 1, pUnreachedAndLeft = 0;
    int nReached = 0;
    // let x be the integer
    // if x >= 0, this pair encodes some agent's last interaction
    // and x is the value of that agent
    // if x < 0, this pair encodes some change in the number of reached
    // agents, and x+10000 is the actual change
    std::vector<std::pair<float, int>> events; events.reserve(_reach.size());
    for(auto [time, x] : _reach) if(x) events.emplace_back(time, x-10000);
    for(auto [_, valueAndTime] : _w) {
        if(valueAndTime.first > _x)
            events.emplace_back(valueAndTime.second, valueAndTime.first);
    }
    std::sort(events.begin(), events.end(), std::greater<std::pair<float, int>>());
    int availableTimeZeroSlots = _freeTimeZeroSlots;
    while(availableTimeZeroSlots--) {
        int x = urd(rng)*1000;
        if(x > _x) events.emplace_back(0, x);
    }
    events.emplace_back(0, -10000);
    float prevT = localTime, pSGUNoInteraction = 0, totalTime = 0;
    float lambdaUnreached = 0;
    int max = _x;
    std::vector<std::tuple<float, float, float, int>> t_pUAS_pUAL;t_pUAS_pUAL.reserve(events.size());
    t_pUAS_pUAL.emplace_back(localTime, pUnreachedAndStayed, pUnreachedAndLeft, 0);
    for(auto [t, x] : events) {
        if(t < prevT) {
            float deltaT = prevT-t;
            totalTime += deltaT;
            float pStayedTillNextTimeStep = pow(1-_rho, deltaT);
            float pUGSTillNextTimeStep = pow(fastPow(1-gamma, nReached), deltaT);
            float c = lnRhoBar/(lnRhoBar+nReached*lnGammaBar);
            // lambdaUnreached += (pUnreachedAndStayed + pUnreachedAndLeft - c) * (
            //                    (_rho ? (pStayedTillNextTimeStep-1)/lnRhoBar : deltaT) +
            //                    (pUGSTillNextTimeStep-1)/lnGammaBar -
            //                    (pStayedTillNextTimeStep * pUGSTillNextTimeStep - 1) / (lnRhoBar+lnGammaBar)
            //                 );
            float pUAndSTillNextTimeStep = pStayedTillNextTimeStep*pUGSTillNextTimeStep;
            float pUAndLBeforeNextTimeStep = c * (1-pUAndSTillNextTimeStep);
            pUnreachedAndStayed *= pUAndSTillNextTimeStep;
            pUnreachedAndLeft = pUAndSTillNextTimeStep*pUnreachedAndLeft + pUAndLBeforeNextTimeStep;
            float pUnreached = pUnreachedAndLeft+pUnreachedAndStayed;
            pStayedGivenUnreached = pUnreached ? pUnreachedAndStayed/pUnreached : 1;
            prevT = t;
        }
        if(x >= 0) {
            if(urd(rng) < pStayedGivenUnreached)
                max = std::max(max, x);
        } else {
            nReached += x+10000;
            t_pUAS_pUAL.emplace_back(t, pUnreachedAndStayed, pUnreachedAndLeft, nReached);
        }
    }
    // lambdaUnreached *= lambdaDelta/totalTime;
    int eventPtr = t_pUAS_pUAL.size()-1;
    float t = 0;
    while(true){
        t += sampleExpDist(1/_lambdaDelta);
        if(t > localTime) break;
        int x = urd(rng)*1000;
        if(x <= _x) continue;
        while(std::get<0>(t_pUAS_pUAL[eventPtr]) < t) eventPtr--;
        std::tie(prevT, pUnreachedAndStayed, pUnreachedAndLeft, nReached) = t_pUAS_pUAL[eventPtr];
        float c = lnRhoBar/(lnRhoBar+nReached*lnGammaBar);
        float deltaT = prevT-t;
        float pStayedTillNextTimeStep = pow(1-_rho, deltaT);
        float pUGSTillNextTimeStep = pow(fastPow(1-gamma, nReached), deltaT);
        float pUAndSTillNextTimeStep = pStayedTillNextTimeStep*pUGSTillNextTimeStep;
        float pUAndLBeforeNextTimeStep = c * (1-pUAndSTillNextTimeStep);
        pUnreachedAndStayed *= pUAndSTillNextTimeStep;
        pUnreachedAndLeft = pUAndSTillNextTimeStep*pUnreachedAndLeft + pUAndLBeforeNextTimeStep;
        float pUnreached = pUnreachedAndLeft+pUnreachedAndStayed;
        if(urd(rng) > pUnreached) continue;
        pStayedGivenUnreached = pUnreached ? pUnreachedAndStayed/pUnreached : 1;
        if(urd(rng) < pStayedGivenUnreached)
            max = std::max(max, x);
    }
    return _y = max;
}