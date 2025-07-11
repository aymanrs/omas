#include "announce_agent.hpp"
#ifdef DEBUG
#include <stdexcept>
#endif

AnnounceAgent::AnnounceAgent(float x, int id) : Agent(x, id) {
    _y = x;
    _k = 0;
}

float AnnounceAgent::estimate() const noexcept {
    return _y;
}

void AnnounceAgent::leave(Agent* agentToInform) {
    AnnounceAgent* other = reinterpret_cast<AnnounceAgent*>(agentToInform);
#ifdef DEBUG
    if(other == nullptr) {
        throw std::runtime_error("Invalid agent type during interaction (expected ThresholdAgent)");
    }
#endif
    if(_k >= other->_k) {
        other->_y = other->_x;
        other->_k = _k+1;
    }
}

void AnnounceAgent::interact(Agent* that) noexcept {
    AnnounceAgent* other = reinterpret_cast<AnnounceAgent*>(that);
#ifdef DEBUG
    if(other == nullptr) {
        throw std::runtime_error("Invalid agent type during interaction (expected ThresholdAgent)");
    }
#endif
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