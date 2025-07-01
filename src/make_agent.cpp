#include "threshold_agent.hpp"
#include "announce_agent.hpp"
#include "twomax_agent.hpp"
#include "optimal_agent.hpp"
#ifdef DEBUG
#include <stdexcept>
#endif

Agent* Agent::makeAgent(AgentType type, int x, int id, int threshold, const float* time, float rho, float lambdaDelta, std::unordered_map<int, float>* arrivals) {
    switch (type){
    case AgentType::Threshold:
        return new ThresholdAgent(x, id, threshold);
    case AgentType::Announce:
        return new AnnounceAgent(x, id);
    case AgentType::TwoMax:
        return new TwoMaxAgent(x, id, threshold);
    case AgentType::Optimal:
        return new OptimalAgent(x, id, time, rho, lambdaDelta, arrivals);
    }
}