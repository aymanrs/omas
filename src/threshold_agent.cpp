#include "threshold_agent.hpp"
#ifdef DEBUG
#include <stdexcept>
#endif

ThresholdAgent::ThresholdAgent(float x, int id, int threshold) : Agent(x, id) {
    _y = x;
    _t = 0;
    _threshold = threshold;
}

float ThresholdAgent::estimate() const noexcept {
    return _y;
}

void ThresholdAgent::update() {
    _t++;
    if(_t >= _threshold) {
        _t = 0;
        _y = _x;
    }
    if(_y <= _x){
        _y = _x;
        _t = 0;
    }
}

void ThresholdAgent::interact(Agent* that) noexcept {
    ThresholdAgent* other = reinterpret_cast<ThresholdAgent*>(that);
#ifdef DEBUG
    if(other == nullptr) {
        throw std::runtime_error("Invalid agent type during interaction (expected ThresholdAgent)");
    }
#endif
    update();
    other->update();
    if(_y > other->_y) {
        other->_y = _y;
        other->_t = _t;
    } else if (_y < other->_y) {
        _y = other->_y;
        _t = other->_t;
    } else {
        _t = other->_t = std::min(_t, other->_t);
    }
}