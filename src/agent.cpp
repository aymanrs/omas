#include "agent.hpp"
#include "threshold_agent.hpp"
#include "announce_agent.hpp"
#include "twomax_agent.hpp"
#include "omniscient_agent.hpp"

thread_local int Agent::_idCounter = 0;

Agent::Agent(float x) : _id(_idCounter++), _x(x) {}

float Agent::value() const {
    return _x;
}

int Agent::id() const {
    return _id;
}

Agent::~Agent() {}


template<>
AnnounceAgent* Agent::makeAgent<AnnounceAgent>(float x, int threshold) {
    return new AnnounceAgent(x);
}

template<>
TwoMaxAgent* Agent::makeAgent<TwoMaxAgent>(float x, int threshold) {
    return new TwoMaxAgent(x);
}

template<>
OmniscientAgent* Agent::makeAgent<OmniscientAgent>(float x, int threshold) {
    return new OmniscientAgent(x);
}
template<>
ThresholdAgent* Agent::makeAgent<ThresholdAgent>(float x, int threshold) {
    return new ThresholdAgent(x, threshold);
}