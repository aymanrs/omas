#include <cmath>
#include "rho_guess_agent.hpp"
#ifdef DEBUG
#include <stdexcept>
#endif

RhoGuessAgent::RhoGuessAgent(float x, int id) : Agent(x, id) {
    _y = x;
    _t = 0;
    _interactions = 0;
    _avgInteractions = 0;
    _weight = 1;
}

float RhoGuessAgent::estimate() const noexcept {
    float pMyYStayed = _t ? pow(_avgInteractions/(_avgInteractions+1), _t) : 1;
    return pMyYStayed * _y + (1-pMyYStayed)*_x;
}

void RhoGuessAgent::update() {
    _t++;
    _interactions++;
    if(_y <= _x) {
        _y = _x;
        _t = 0;
    }
}

void RhoGuessAgent::interact(Agent* that) noexcept {
    RhoGuessAgent* other = reinterpret_cast<RhoGuessAgent*>(that);
#ifdef DEBUG
    if(other == nullptr) {
        throw std::runtime_error("Invalid agent type during interaction (expected RhoGuessAgent)");
    }
#endif
    update();
    other->update();
    _avgInteractions = (_avgInteractions*_weight+1+other->_interactions)/(_weight+1);
    _weight++;
    other->_avgInteractions = (other->_avgInteractions*other->_weight+1+_interactions)/(other->_weight+1);
    other->_weight++;
    if(_y == other->_y) {
        _t = other->_t = std::min(_t, other->_t);
    }
    if(_x >= other->_y) {
        other->_y = _x;
        other->_t = 0;
    } else if(other->_x >= _y){
        _y = other->_x;
        _t = 0;
    }
    float avgInteractions = (_avgInteractions*_weight+other->_avgInteractions*_weight)/(_weight+other->_weight);
    float pMyYStayed = pow(avgInteractions/(avgInteractions+1), _t), myY = _y;
    float pOtherYStayed = pow(avgInteractions/(avgInteractions+1), other->_t), otherY = other->_y;
    if(pMyYStayed*myY+(1-pMyYStayed)*other->_x > pOtherYStayed*otherY+(1-pOtherYStayed)*other->_x) {
        other->_y = _y;
        other->_t = _t;
    }
    if(pMyYStayed*myY+(1-pMyYStayed)*_x < pOtherYStayed*otherY+(1-pOtherYStayed)*_x) {
        _y = other->_y;
        _t = other->_t;
    }
}