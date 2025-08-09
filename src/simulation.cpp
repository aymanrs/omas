#include <queue>
#include <future>
#include "simulation.hpp"
#include "announce_agent.hpp"
#include "omniscient_agent.hpp"
#include "twomax_agent.hpp"
#include "threshold_agent.hpp"

template<ValidAgentType AgentType>
float asymptoticMetricValue(int n, int simTimePerLifetime, float IPL, const std::function<float(const MetricInfo&, float)>& metric, int threshold) noexcept {
    std::priority_queue<std::pair<float, int>, std::vector<std::pair<float, int>>, std::greater<std::pair<float, int>>> departures;
    std::priority_queue<std::pair<float, int>, std::vector<std::pair<float, int>>, std::greater<std::pair<float, int>>> interactions;
    float tau_d = n*IPL/2;
    float tau_a = tau_d/n;
    float tau_g = n;
    float t = 0, nextArrivalTime = sampleExpDist(tau_a);
    float metricValueSum = 0;
    std::unordered_map<int, AgentType*> agents;
    MetricInfo metricInfo{.avgN = float(n), .IPL = IPL, .actualMax = std::numeric_limits<float>::lowest()};
    for(int i = 0;i < n;i++){
        AgentType* newAgent = Agent::makeAgent<AgentType>(urd(rng)*1000, threshold);
        metricInfo.actualMax = std::max(metricInfo.actualMax, newAgent->value());
        departures.emplace(
            sampleExpDist(tau_d),
            newAgent->id()
        );
        interactions.emplace(sampleExpDist(tau_g), newAgent->id());
        agents[newAgent->id()] = newAgent;
    }
    float TSE = 0;
    float error;
    bool reachedTrackingStart = false; // Becomes true once `startTrackingTime` is reached
    float simTime = std::max(simTimePerLifetime * tau_d, 3000.f);
    float startTrackingTime = simTime/2, totalTrackingTime = 0;
    while(t < simTime) {
        float prevT = t;
        float nextInteractionTime = interactions.empty() ? INFINITY : interactions.top().first;
        float nextDepartureTime = departures.empty() ? INFINITY : departures.top().first;
        t = std::min(std::min(nextArrivalTime, nextInteractionTime), nextDepartureTime);
        if(t > startTrackingTime) {
            if(!reachedTrackingStart) {
                TSE = 0;
                for(auto [id, agent] : agents) {
                    error = metric(metricInfo, agent->estimate());
                    TSE += error;
                }
                reachedTrackingStart = true;
            }
            totalTrackingTime += t-prevT;
            // ^ Not strictly necessary to keep track of but can prevent nasty bugs
            // when some sections of time aren't accounted for/overcounted
            metricValueSum += (TSE/agents.size())*(t-prevT);
        }
        if(nextInteractionTime < nextDepartureTime && nextInteractionTime < nextArrivalTime) {
            // Interaction Step
            int id = interactions.top().second;
            interactions.pop();
            if(agents.size() < 2) continue;
            auto it1 = agents.find(id);
            // The agent initializing this interaction might have
            // left before, in which case we do nothing
            if(it1 == agents.end()) continue;

            // Pick a random different agent
            auto it2 = std::next(agents.begin(), 1+rng()%(agents.size()-1));
            if(it2 == it1) it2 = agents.begin();

            // Note that we don't update the time until the two agents
            // have been successfully picked, so that the time between
            // `prevT` and `nextInteractionTime` is accounted for during tracking
            t = nextInteractionTime;
            if(reachedTrackingStart) {
                float error1 = metric(metricInfo, it1->second->estimate());
                float error2 = metric(metricInfo, it2->second->estimate());
                TSE -= error1 + error2;
            }
            it1->second->interact(it2->second);
            if(reachedTrackingStart) {
                float error1 = metric(metricInfo, it1->second->estimate());
                float error2 = metric(metricInfo, it2->second->estimate());
                TSE += error1 + error2;
            }
            interactions.emplace(t+sampleExpDist(tau_g), id);
        } else if(nextDepartureTime < nextArrivalTime) {
            // Departure Step
            int id = departures.top().second;
            AgentType* leavingAgent = agents[id];
            departures.pop();
            float oldValue = leavingAgent->value();
            if(reachedTrackingStart) {
                error = metric(metricInfo, leavingAgent->estimate());
                TSE -= error;
            }
            agents.erase(id);
            if constexpr (std::is_same<AgentType, OmniscientAgent>::value) {
                // Omniscient agents must be informed of all depratures
                for(auto [_, agent] : agents){
                    agent->acknowledgeDeparture(leavingAgent);
                }
            }
            if constexpr (std::is_same<AgentType, AnnounceAgent>::value) {
                if(agents.size() >= 2) {
                    auto agentToInformIt = std::next(agents.begin(), rng()%(agents.size()-1)+1);
                    if(agentToInformIt->first == id) agentToInformIt = agents.begin();
                    if(reachedTrackingStart)
                        TSE -= metric(metricInfo, agentToInformIt->second->estimate());
                    agentToInformIt->second->acknowledgeDeparture(leavingAgent);
                    // We don't need to update the current max before adding the informed
                    // agent's error because this is handled later on
                    if(reachedTrackingStart)
                        TSE += metric(metricInfo, agentToInformIt->second->estimate());
                } 
            }
            delete leavingAgent;
            if(oldValue == metricInfo.actualMax){
                metricInfo.actualMax = std::numeric_limits<float>::lowest();
                for(auto [_, agent] : agents) {
                    metricInfo.actualMax = std::max(metricInfo.actualMax, agent->value());
                }
                if(reachedTrackingStart) {
                    TSE = 0;
                    for(auto [_, agent] : agents) {
                        error = metric(metricInfo, agent->estimate());
                        TSE += error;
                    }
                }
            }
        } else {
            // Arrival Step
            nextArrivalTime = t+sampleExpDist(tau_a);
            AgentType* newAgent = Agent::makeAgent<AgentType>(urd(rng)*1000, threshold);
            agents[newAgent->id()] = newAgent;
            interactions.emplace(t+sampleExpDist(tau_g), newAgent->id());
            departures.emplace(t+sampleExpDist(tau_d), newAgent->id());
            if(newAgent->value() > metricInfo.actualMax) {
                metricInfo.actualMax = newAgent->value();
                if(reachedTrackingStart) {
                    TSE = 0;
                    for(auto [id, agent] : agents) {
                        error = metric(metricInfo, agent->estimate());
                        TSE += error;
                    }
                }
            } else if (reachedTrackingStart) {
                error = metric(metricInfo, newAgent->estimate());
                TSE += error;
            }
        }
    }
    for(auto [_, agent] : agents) {
        delete agent;
    }
    return metricValueSum/totalTrackingTime;
}

template<ValidAgentType AgentType>
Curve generateCurve(int n, int simTimePerLifetime, const std::vector<float>& IPLs, int nRuns, const std::function<float(const MetricInfo&, float)>& metric, const std::string& label, int threshold) noexcept {
    Curve curve;
    curve.label = label;
    curve.x = IPLs;
    curve.y.reserve(IPLs.size());
    std::future<float> future_asymptotes[IPLs.size()][nRuns];
    for(size_t i = 0;i < IPLs.size();i++) {
        for(int j = 0;j < nRuns;j++) future_asymptotes[i][j] = std::async(std::launch::async, asymptoticMetricValue<AgentType>, n, simTimePerLifetime, IPLs[i], metric, threshold);
    }
    for(size_t i = 0;i < IPLs.size();i++) {
        float avgAsymptote = 0;
        for(int j = 0;j < nRuns;j++) avgAsymptote += future_asymptotes[i][j].get();
        curve.y.push_back(avgAsymptote/nRuns);
    }
    return curve;
}

template Curve generateCurve<TwoMaxAgent>(int n, int simTimePerLifetime, const std::vector<float>& IPLs, int nRuns, const std::function<float(const MetricInfo&, float)>& metric, const std::string& label, int threshold);
template Curve generateCurve<OmniscientAgent>(int n, int simTimePerLifetime, const std::vector<float>& IPLs, int nRuns, const std::function<float(const MetricInfo&, float)>& metric, const std::string& label, int threshold);
template Curve generateCurve<ThresholdAgent>(int n, int simTimePerLifetime, const std::vector<float>& IPLs, int nRuns, const std::function<float(const MetricInfo&, float)>& metric, const std::string& label, int threshold);
template Curve generateCurve<AnnounceAgent>(int n, int simTimePerLifetime, const std::vector<float>& IPLs, int nRuns, const std::function<float(const MetricInfo&, float)>& metric, const std::string& label, int threshold);