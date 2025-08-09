#include <cmath>
#include "omniscient_agent.hpp"

OmniscientAgent::OmniscientAgent(float x) : Agent(x) {
    _cachedY = _y = x;
    _w[_id] = x;
}

float OmniscientAgent::estimate() const noexcept {
    return _cachedY;
}

void OmniscientAgent::acknowledgeDeparture(OmniscientAgent* leavingAgent) {
    auto it = _w.find(leavingAgent->id());
    if(it != _w.end()) _w.erase(it);
    if(leavingAgent->value() != _y) return;
    _y = _x;
    for(auto [_, x] : _w) _y = std::max(_y, x);
}

void OmniscientAgent::interact(OmniscientAgent* other) noexcept {
    _cachedY = other->_cachedY = other->_y = _y = std::max(_y, other->_y);
    for(auto [id, x] : _w){
        other->_w[id] = x;
    }
    if(_w.size() == other->_w.size()) return;
    for(auto [id, x] : other->_w){
        _w[id] = x;
    }
}