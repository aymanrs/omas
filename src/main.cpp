#include <fstream>
#include <vector>
#include <random>
#include <optional>
#include <iostream>
#include <list>
#include <queue>
#include <future>
#ifdef DEBUG
    #include <stdexcept>
#endif
#include "utils.hpp"
#include "agent.hpp"
#include "optimal_agent.hpp"

struct interaction {
    float t;
    int id1, id2;
    interaction(float T, int ID1, int ID2) : t(T), id1(ID1), id2(ID2) {} 
    bool operator < (const interaction& o) const {
        if(t != o.t) return t > o.t;
        return id1 < o.id1 || (id1 == o.id1 && id2 < o.id2);
    }
};
float asymptote(AgentType type, int n, int threshold, int simTime, float rho, float delta) noexcept {
    int idCounter = 0;
    // arrivals will be automatically populated when creating agents
    // no modification is necessary
    std::unordered_map<int, float> arrivals;
    float startAvgTime = 1350, totalTrackingTime = 0;
    std::priority_queue<std::pair<float, int>, std::vector<std::pair<float, int>>, std::greater<std::pair<float, int>>> departures;
    std::priority_queue<interaction> interactions;
    float tauRho = rho ? -1/log(1-rho) : INFINITY;
    float tauDelta = rho ? tauRho/n : INFINITY;float lambdaDelta = 1/tauDelta;
    float tauGamma = n*(n-1)/2;
    float t = 0, nextArrivalTime = rho ? sampleExpDist(tauDelta) : INFINITY;
    int actualMax;
    float runAsymtote = 0;
    std::unordered_map<int, Agent*> agents;
    for(int i = 0;i < n;i++){
        Agent* newAgent = Agent::makeAgent(type, rng()%1000, idCounter++, threshold, &t, rho, lambdaDelta, &arrivals);
        if(rho) departures.emplace(
            sampleExpDist(tauRho),
            newAgent->_id
        );
        for(auto [id, oldAgent] : agents){
            interactions.emplace(sampleExpDist(tauGamma), newAgent->_id, id);
        }
        agents[newAgent->_id] = newAgent;
    }
    float TSE, error;
    bool reachedTrackingStart = false;
    while(t < simTime) {
        float prevT = t;
        float nextInteractionTime = interactions.empty() ? INFINITY : interactions.top().t;
        float nextDepartureTime = departures.empty() ? INFINITY : departures.top().first;
        if(nextInteractionTime < nextDepartureTime && nextInteractionTime < nextArrivalTime) {
            t = nextInteractionTime;
            auto [_, id1, id2] = interactions.top();
            interactions.pop();
            auto it1 = agents.find(id1), it2 = agents.find(id2);
            if(it1 == agents.end() || it2 == agents.end()) continue;
            if(reachedTrackingStart) {
                float error1 = actualMax-it1->second->estimate();
                float error2 = actualMax-it2->second->estimate();
                TSE -= error1*error1 + error2*error2;
            }
            it1->second->interact(it2->second);
            if(reachedTrackingStart) {
                float error1 = actualMax-it1->second->estimate();
                float error2 = actualMax-it2->second->estimate();
                TSE += error1*error1 + error2*error2;
            }
            interactions.emplace(t+sampleExpDist(tauGamma), id1, id2);
        } else if(nextDepartureTime < nextArrivalTime) {
            t = nextDepartureTime;
            int id = departures.top().second;
            departures.pop();
            // TODO: call Agent.leave()
            int oldValue = agents[id]->value();
            if(reachedTrackingStart) {
                error = actualMax-agents[id]->estimate();
                TSE -= error*error;
            }
            delete agents[id];
            agents.erase(id);
            if(reachedTrackingStart && oldValue == actualMax){
                TSE = 0;
                actualMax = std::numeric_limits<int>::min();
                for(auto [_, agent] : agents) {
                    actualMax = std::max(actualMax, agent->value());
                }
                for(auto [_, agent] : agents) {
                    error = actualMax-agent->estimate();
                    TSE += error*error;
                }
            }
        } else {
            t = nextArrivalTime;
            nextArrivalTime = t+sampleExpDist(tauDelta);
            Agent* newAgent = Agent::makeAgent(type, rng()%1000, idCounter++, threshold, &t, rho, lambdaDelta, &arrivals);
            for(auto [id, oldAgent] : agents) {
                interactions.emplace(t+sampleExpDist(tauGamma), newAgent->_id, id);
            }
            agents[newAgent->_id] = newAgent;
            departures.emplace(t+sampleExpDist(tauRho), newAgent->_id);
            if(reachedTrackingStart) {
                if(newAgent->value() > actualMax) {
                    actualMax = newAgent->value();
                    TSE = 0;
                    for(auto [id, agent] : agents) {
                        error = actualMax-agent->estimate();
                        TSE += error*error;
                    }
                } else {
                    error = actualMax-newAgent->estimate();
                    TSE += error*error;
                }
            }
        }
        if(t > startAvgTime) {
            if(!reachedTrackingStart) {
                actualMax = std::numeric_limits<int>::min();
                for(auto [id, agent] : agents) {
                    actualMax = std::max(actualMax, agent->value());
                }
                TSE = 0;
                for(auto [id, agent] : agents) {
                    error = actualMax-agent->estimate();
                    TSE += error*error;
                }
                reachedTrackingStart = true;
            }
            totalTrackingTime += t-prevT;
            runAsymtote += (TSE/agents.size())*(t-prevT);
        }
    }
    for(auto [_, agent] : agents) delete agent;
    return runAsymtote/totalTrackingTime;
}

Curve generateCurve(AgentType type, int n, int threshold, int simTime, const std::vector<float>& rhos, int nRuns, const std::string& label = "") {
    Curve curve;
    curve.label = label;
    curve.x = rhos;
    curve.y.reserve(rhos.size());
    std::vector<std::future<float>> future_mmse;future_mmse.reserve(rhos.size());
    int idx = 0;
    float tauGamma = n*(n-1)/2;
    float gamma = 1-exp(-1/tauGamma);
    OptimalAgent::gamma = gamma;
    float perc = 100./rhos.size();
    for(float rho : rhos) {
        future_mmse.push_back(std::async(std::launch::async, [&type, &nRuns, &n, &threshold, &simTime, &perc, rho, idx](){
            float avgAsymptote = 0;
            for(int _ = 0;_ < nRuns;_++) avgAsymptote += asymptote(type, n, threshold, simTime, rho, n*rho);
            std::cout << idx*perc << "%" << std::endl;
            return sqrtf(std::max(0.f, avgAsymptote/nRuns));
        }));
        idx++;
    }
    for(std::future<float>& f_mmse : future_mmse)
        curve.y.push_back(f_mmse.get());
    return curve;
}

int main() {
    const int n = 25, T_star = 15, simTime = 1800, nRuns = 150;
    std::vector<float> rhos;
    const float a = 0, b = 0.005, nRhos = 30;
    for(float rhoIdx = 0; rhoIdx < nRhos;rhoIdx++) {
        rhos.push_back(a+(b-a)*rhoIdx/nRhos);
    }
    Curve optimal = generateCurve(AgentType::Optimal, n, T_star, simTime, rhos, nRuns, "optimal");
    for(float &y_i : optimal.y) y_i /= M_SQRT2f;
    Curve threshold = generateCurve(AgentType::Threshold, n, T_star, simTime, rhos, nRuns, "dumb");
    Curve twoMax = generateCurve(AgentType::TwoMax, n, T_star, simTime, rhos, nRuns, "2max");
    using std::to_string;
    savePlot(
        {threshold, twoMax, optimal},
        "data/comparison.plot",
        "n=" + to_string(n) + ", T*=" + to_string(T_star) + ", " + to_string(nRuns) + " run(s) per ρ value",
        "ρ",
        "Root of Avg MS Error"
    );
}
