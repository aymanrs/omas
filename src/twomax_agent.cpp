#include "twomax_agent.hpp"
#include <limits>
#include <cmath>
#include <algorithm>

// An approximation of the probability that a given
// agent is still active given that their last confirmed
// presence was `tn` interaction rounds ago
// `IPL` is (an approximation of) the average number of interactions over an agent's lifetime
// `N` is (an approximation of) the number of active agents
static inline double probStayedGivenT(double tn, double IPL, double N){
    // This function makes the following simplifying assumptions:
    // - N is big enough to model the spread of information as a logistic regression.
    //   So if there are no arrivals/departures, the probability that some information
    //   that started from a given agent reaches another given agent within T "seconds" 
    //   is 1/(1+(N-1)e^(-2T/(N-1)))
    // - The time taken by the `tn` interaction rounds is equal to the average
    //   time taken by `tn` interaction rounds
    // - No agent other than the agent from whom the information originated may leave
    // - No new agent arrives
    double expMinusTwoOverNMinus1T = exp(-tn);
    double pS = pow(1-1/IPL, tn);
    double logRhoBar = -2/(N*IPL);
    double pRgS = (N/(1+(N-1)*expMinusTwoOverNMinus1T)-1)/(N-1);
    double pRaL = -logRhoBar/(1+(N-1)*expMinusTwoOverNMinus1T) * (
        (pS-1)/logRhoBar +
        (1-pS*expMinusTwoOverNMinus1T)/(logRhoBar-2/(N-1))
    );
    return pS*(1-pRgS)/(1-pS*pRgS-pRaL);
}

TwoMaxAgent::TwoMaxAgent(float x) : Agent(x) {
    _y = x;
    _t = 0;
    _y2 = std::numeric_limits<float>::lowest();
    _t2 = 0;
    _avgInteractions = 0;
    _avgInteractionsAs2Max = 0;
}

float TwoMaxAgent::estimate() const noexcept {
    if(!_t || _y == _x || _y2 == std::numeric_limits<float>::lowest()) return _y;
    double N = _avgInteractionsAs2Max ? std::max(_avgInteractions/_avgInteractionsAs2Max, 2.f) : 2;
    double yIsMax = probStayedGivenT(_t, _avgInteractions, N);
    double y2IsMax = probStayedGivenT(_t2, _avgInteractions, N)*(1-yIsMax);
    return yIsMax > y2IsMax ? _y : _y2;
}

void TwoMaxAgent::update() {
    _t++;
    _t2++;
    _avgInteractions++;
    if(_y <= _x){
        _y = _x;
        _t = 0;
        _y2 = std::numeric_limits<float>::lowest();
        _t2 = 0;
        return;
    }
    if (_y2 <= _x){
        _y2 = _x;
        _t2 = 0;
        return;
    }
    double N = _avgInteractionsAs2Max ? std::max(_avgInteractions/_avgInteractionsAs2Max, 2.f) : 2;
    double yIsMax = probStayedGivenT(_t,_avgInteractions, N);
    double y2IsMax = probStayedGivenT(_t2,_avgInteractions, N)*(1-yIsMax);
    if(y2IsMax < 1-yIsMax-y2IsMax){
        _y2 = _x;
        _t2 = 0;
        y2IsMax = 0;
    } else if(yIsMax < 1-yIsMax-y2IsMax) {
        _y = _y2;
        _t = _t2;
        _y2 = _y2 <= _x ? std::numeric_limits<float>::lowest() : _x;
        _t2 = 0;
    }
}

void TwoMaxAgent::interact(TwoMaxAgent* other) noexcept {
    update();
    other->update();
    _avgInteractions = other->_avgInteractions = (_avgInteractions+other->_avgInteractions)/2;
    if(_y > other->_y) {
        other->_y2 = other->_y;
        other->_t2 = other->_t;
        other->_y = _y;
        other->_t = _t;
    } else if (_y < other->_y){
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
    } else if (_y2 < other->_y2){
        _y2 = other->_y2;
        _t2 = other->_t2;
    } else {
        _t2 = other->_t2 = std::min(_t2, other->_t2);
    }
    if(_y <= _x || _y2 <= _x) _avgInteractionsAs2Max += 0.5;
    if(_y <= other->_x || _y2 <= other->_x) other->_avgInteractionsAs2Max += 0.5;
    _avgInteractionsAs2Max = other->_avgInteractionsAs2Max = (_avgInteractionsAs2Max + other->_avgInteractionsAs2Max)/2;
}