#include <fstream>
#include <vector>
#include <random>
#include <optional>
#include <iostream>
#include <list>
#include <queue>
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
int Agent::idCounter = 0;
float asymptote(AgentType type, int n, int threshold, int simTime, float rho, float delta) noexcept {
    Agent::idCounter = 0;
    OptimalAgent::time = 0;
    OptimalAgent::arrivals.clear();
    float startAvgTime = 1000, totalTrackingTime = 0;
    bool isOptimal = type==AgentType::Optimal;
    std::priority_queue<std::pair<float, int>, std::vector<std::pair<float, int>>, std::greater<std::pair<float, int>>> departures;
    std::priority_queue<interaction> interactions;
    float tauRho = rho ? -1/log(1-rho) : INFINITY;
    float tauDelta = rho ? tauRho/n : INFINITY;OptimalAgent::lambdaDelta = 1/tauDelta;
    float tauGamma = n*(n-1)/2;
    float gamma = 1-exp(-1/tauGamma);
    OptimalAgent::gamma = gamma;
    float t = 0, nextArrivalTime = rho ? sampleExpDist(tauDelta) : INFINITY;
    int actualMax = std::numeric_limits<int>::min();
    float runAsymtote = 0;
    std::unordered_map<int, Agent*> agents;
    for(int i = 0;i < n;i++){
        Agent* newAgent = Agent::makeAgent(type, rng()%1000, threshold);
        actualMax = std::max(actualMax, newAgent->value());
        if(rho) departures.emplace(
            sampleExpDist(tauRho),
            newAgent->_id
        );
        for(auto [id, oldAgent] : agents){
            interactions.emplace(sampleExpDist(tauGamma), newAgent->_id, id);
        }
        agents[newAgent->_id] = newAgent;
    }
    float TSE = 0, error;
    for(auto [id, agent] : agents) {
        error = actualMax-agent->estimate();
        TSE += error*error;
    }
    while(t < simTime) {
        float prevT = t;
        float nextIneractionTime = interactions.empty() ? INFINITY : interactions.top().t;
        float nextDepartureTime = departures.empty() ? INFINITY : departures.top().first;
        if(nextIneractionTime < nextDepartureTime && nextIneractionTime < nextArrivalTime) {
            t = nextIneractionTime;OptimalAgent::time = t;
            auto [_, id1, id2] = interactions.top();
            interactions.pop();
            auto it1 = agents.find(id1), it2 = agents.find(id2);
            if(it1 == agents.end() || it2 == agents.end()) continue;
            float error1 = actualMax-it1->second->estimate();
            float error2 = actualMax-it2->second->estimate();
            TSE -= error1*error1 + error2*error2;
            it1->second->interact(it2->second);
            error1 = actualMax-it1->second->estimate();
            error2 = actualMax-it2->second->estimate();
            TSE += error1*error1 + error2*error2;
            interactions.emplace(t+sampleExpDist(tauGamma), id1, id2);
        } else if(nextDepartureTime < nextArrivalTime) {
            t = nextDepartureTime;OptimalAgent::time = t;
            int id = departures.top().second;
            departures.pop();
            // TODO: call Agent.leave()
            int oldValue = agents[id]->value();
            error = actualMax-agents[id]->estimate();
            TSE -= error*error;
            delete agents[id];
            agents.erase(id);
            if(oldValue == actualMax){
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
            t = nextArrivalTime;OptimalAgent::time = t;
            nextArrivalTime = t+sampleExpDist(tauDelta);
            Agent* newAgent = Agent::makeAgent(type, rng()%1000, threshold);
            if(isOptimal){
                newAgent->interact(newAgent);
            }
            for(auto [id, oldAgent] : agents) {
                interactions.emplace(t+sampleExpDist(tauGamma), newAgent->_id, id);
            }
            agents[newAgent->_id] = newAgent;
            departures.emplace(t+sampleExpDist(tauRho), newAgent->_id);
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
        if(t > startAvgTime) {
            totalTrackingTime += t-prevT;
            runAsymtote += sqrt(std::max(0.f, TSE/agents.size()))*(t-prevT);
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
    for(float rho : rhos) {
        OptimalAgent::rho = rho;
        float avgAsymptote = 0;
        for(int _ = 0;_ < nRuns;_++) avgAsymptote += asymptote(type, n, threshold, simTime, rho, n*rho);
        curve.y.push_back(avgAsymptote/nRuns);
    }
    return curve;
}

int main() {
    const int n = 25, T_star = 15, simTime = 1500, nRuns = 50;
    std::vector<float> rhos;
    const float a = 0, b = 0.015, nRhos = 2;
    for(float rhoIdx = 0; rhoIdx < nRhos;rhoIdx++) rhos.push_back(a+(b-a)*rhoIdx/nRhos);
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
        "Avg RMS Error"
    );
}
