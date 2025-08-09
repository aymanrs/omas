#include <algorithm>
#include "threshold_agent.hpp"

ThresholdAgent::ThresholdAgent(float x, int threshold) : Agent(x) {
    _y = x;
    _t = 0;
    _threshold = threshold;
}

float ThresholdAgent::estimate() const noexcept {
    return _y;
}

void ThresholdAgent::update() {
    if(_t > _threshold) {
        _t = 0;
        _y = _x;
    }
    if(_y <= _x){
        _y = _x;
        _t = 0;
    }
}

void ThresholdAgent::interact(ThresholdAgent* other) noexcept {
    if(_y > other->_y) {
        other->_y = _y;
        other->_t = ++_t;
    } else if (_y < other->_y) {
        _y = other->_y;
        _t = ++other->_t;
    } else {
        _t = other->_t = std::min(_t, other->_t)+1;
    }
    update();
    other->update();
}