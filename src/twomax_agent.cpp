#include "twomax_agent.hpp"
#include <limits>
#ifdef DEBUG
#include <stdexcept>
#endif

TwoMaxAgent::TwoMaxAgent(int x, int threshold) {
    _x = x;
    _y = x;
    _t = 0;
    _y2 = std::numeric_limits<float>::min();
    _t2 = 0;
    _threshold = threshold;
}

float TwoMaxAgent::estimate() {
    return _y;
}

void TwoMaxAgent::update() {
    _t++;
    _t2++;
    if(_t >= _threshold) {
        _t = _t2/2;
        _y = _y2;
        _y2 = std::numeric_limits<float>::min();
        _t2 = 0;
    }
    if(_y <= _x){
        _y = _x;
        _t = 0;
    }
    if(_t2 >= _threshold*2) {
        _y2 = std::numeric_limits<float>::min();
        _t2 = 0;
    }
}

void TwoMaxAgent::interact(Agent* that) {
    TwoMaxAgent* other = reinterpret_cast<TwoMaxAgent*>(that);
#ifdef DEBUG
    if(other == nullptr) {
        throw std::runtime_error("Invalid agent type during interaction (expected TwoMaxAgent)");
    }
#endif
    update();
    other->update();
    if(_y > other->_y) {
        other->_y2 = other->_y;
        other->_t2 = other->_t;
        other->_y = _y;
        other->_t = _t;
    } else if (_y < other->_y) {
        _y2 = _y;
        _t2 = _t;
        _y = other->_y;
        _t = other->_t;
    } else {
        _t = other->_t = std::min(_t, other->_t);
    }
    if(_y2 > other->_y2) {
        other->_y2 = _y2;
        other->_t2 = _t2;
    } else if(_y2 < other->_y2) {
        _y2 = other->_y2;
        _t2 = other->_t2;
    } else {
        _t2 = other->_t2 = std::min(_t2, other->_t2);
    }
}