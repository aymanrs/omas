#include "announce_agent.hpp"
#include <algorithm>

AnnounceAgent::AnnounceAgent(float x) : Agent(x) {
    _y = x;
    _k = 0;
}

float AnnounceAgent::estimate() const noexcept {
    return _y;
}

void AnnounceAgent::acknowledgeDeparture(AnnounceAgent* leavingAgent) {
    if(leavingAgent->_k >= _k) {
        _y = _x;
        _k = leavingAgent->_k+1;
    }
}

void AnnounceAgent::interact(AnnounceAgent* other) noexcept {
    if(_k > other->_k){
        _y = other->_y = std::max(_y, float(other->_x));
        other->_k = _k;
    } else if (_k < other->_k) {
        _y = other->_y = std::max(float(_x), other->_y);
        _k = other->_k;
    } else {
        _y = other->_y = std::max(_y, other->_y);
    }
}