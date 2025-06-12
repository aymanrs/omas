#include "threshold_agent.hpp"
#include "announce_agent.hpp"
#include "twomax_agent.hpp"
#include "optimal_agent.hpp"
#ifdef DEBUG
#include <stdexcept>
#endif

Agent* Agent::makeAgent(AgentType type, int x, int threshold) {
    switch (type){
    case AgentType::Threshold:
        return new ThresholdAgent(x, threshold);
    case AgentType::Announce:
        return new AnnounceAgent(x);
    case AgentType::TwoMax:
        return new TwoMaxAgent(x, threshold);
    case AgentType::Optimal:
        return new OptimalAgent(x);
    }
}