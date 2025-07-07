#include <fstream>
#include <iostream>
#include <queue>
#include <future>
#include "pcg_random.hpp"
#ifdef DEBUG
#include <stdexcept>
#endif
#include "utils.hpp"
#include "agent.hpp"
#include "optimal_agent.hpp"
#include "twomax_agent.hpp"
#include "rho_guess_agent.hpp"
#include "threshold_agent.hpp"

template<typename AgentType>
float asymptote(int n, int threshold, int simTime, float rho) noexcept {
    return urd(rng);
    int idCounter = 0;
    // arrivals will be automatically populated when creating agents
    // no modification is necessary
    std::unordered_map<int, float> arrivals;
    float startAvgTime = 100000, totalTrackingTime = 0;
    std::priority_queue<std::pair<float, int>, std::vector<std::pair<float, int>>, std::greater<std::pair<float, int>>> departures;
    std::priority_queue<std::pair<float, int>, std::vector<std::pair<float, int>>, std::greater<std::pair<float, int>>> interactions;
    float tauRho = rho ? -1/log(1-rho) : INFINITY;
    float tauDelta = rho ? tauRho/n : INFINITY;float lambdaDelta = 1/tauDelta;
    float tauGamma = n;
    float t = 0, nextArrivalTime = rho ? sampleExpDist(tauDelta) : INFINITY;
    float actualMax;
    float runAsymtote = 0;
    std::unordered_map<int, Agent*> agents;
    n=0;
    for(int i = 0;i < n;i++){
        Agent* newAgent = Agent::makeAgent<AgentType>(urd(rng)*1000, idCounter++, threshold, &t, rho, lambdaDelta, &arrivals);
        if(rho) departures.emplace(
            sampleExpDist(tauRho),
            newAgent->id()
        );
        interactions.emplace(sampleExpDist(tauGamma), newAgent->id());
        agents[newAgent->id()] = newAgent;
    }
    float TSE, error;
    bool reachedTrackingStart = false;
    while(t < simTime) {
        float prevT = t;
        float nextInteractionTime = interactions.empty() ? INFINITY : interactions.top().first;
        float nextDepartureTime = departures.empty() ? INFINITY : departures.top().first;
        if(nextInteractionTime < nextDepartureTime && nextInteractionTime < nextArrivalTime) {
            t = nextInteractionTime;
            int id = interactions.top().second;
            interactions.pop();
            if(agents.size() < 2) continue;
            auto it1 = agents.find(id);
            if(it1 == agents.end()) continue;
            auto it2 = std::next(agents.begin(), rng()%(agents.size()-1));
            if(it2->first == id) it2++;
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
            interactions.emplace(t+sampleExpDist(tauGamma), id);
        } else if(nextDepartureTime < nextArrivalTime) {
            t = nextDepartureTime;
            int id = departures.top().second;
            Agent* oldAgent = agents[id];
            departures.pop();
            // TODO: call Agent.leave()
            int oldValue = oldAgent->value();
            if(reachedTrackingStart) {
                error = actualMax-oldAgent->estimate();
                TSE -= error*error;
            }
            delete oldAgent;
            agents.erase(id);
            if(reachedTrackingStart && oldValue == actualMax){
                TSE = 0;
                actualMax = std::numeric_limits<float>::min();
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
            Agent* newAgent = Agent::makeAgent<AgentType>(urd(rng)*1000, idCounter++, threshold, &t, rho, lambdaDelta, &arrivals);
            agents[newAgent->id()] = newAgent;
            interactions.emplace(t+sampleExpDist(tauGamma), newAgent->id());
            departures.emplace(t+sampleExpDist(tauRho), newAgent->id());
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
                actualMax = std::numeric_limits<float>::min();
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
    for(auto [_, agent] : agents) {
        delete agent;
    }
    return runAsymtote/totalTrackingTime;
}

template<typename AgentType>
Curve generateCurve(int n, int threshold, int simTime, const std::vector<float>& IPLs, int nRuns, const std::string& label = "") {
    Curve curve;
    curve.label = label;
    curve.x = IPLs;
    curve.y.reserve(IPLs.size());
    float tauGamma = n*(n-1)/2;
    float gamma = 1-exp(-1/tauGamma);
    OptimalAgent::gamma = gamma;
    float perc = 100./IPLs.size();
    int idx = 0;
    std::future<float> future_asymptotes[IPLs.size()][nRuns];
    for(int i = 0;i < IPLs.size();i++) {
        float IPL = IPLs[i];
        float rho = 1-exp(-2./(n*IPL));
        for(int j = 0;j < nRuns;j++) future_asymptotes[i][j] = std::async(std::launch::async, asymptote<AgentType>, n, threshold, simTime, rho);
        // std::cout << idx*perc << "%\n";
        // idx++;
    }
    for(int i = 0;i < IPLs.size();i++) {
        float avgAsymptote = 0;
        for(int j = 0;j < nRuns;j++) avgAsymptote += future_asymptotes[i][j].get();
        curve.y.push_back(avgAsymptote/nRuns);continue;
        curve.y.push_back(sqrtf(std::max(0.f, avgAsymptote/nRuns)));
    }
    return curve;
}



int main() {
    const int n = 25, T_star = 11, simTime = 400000, nRuns = 60;
    std::vector<float> InteractionsPerLifetime;
    const float a = 500, b = 4000, nIPLs = 10;
    for(float IPLIdx = 0; IPLIdx < nIPLs;IPLIdx++) {
        InteractionsPerLifetime.push_back(a+(b-a)*IPLIdx/nIPLs);
    }
    // Curve optimal = generateCurve(AgentType::Optimal, n, T_star, simTime, InteractionsPerLifetime, nRuns, "optimal");
    // for(float &y_i : optimal.y) y_i /= M_SQRT2f;
    float avgRatio = 0, c = 0;
    Curve rhoguess = generateCurve<RhoGuessAgent>(n, T_star, simTime, InteractionsPerLifetime, nRuns, "guess");
    Curve twoMax = generateCurve<TwoMaxAgent>(n, T_star, simTime, InteractionsPerLifetime, nRuns, "2max");
    for(int i = 0;i < rhoguess.y.size();i++){
        if(rhoguess.y[i] <= 0) continue;
        avgRatio += twoMax.y[i]/rhoguess.y[i];
        c++;
    }
    avgRatio/=c;
    std::cout << avgRatio << '\n';
    using std::to_string;
    savePlot(
        {rhoguess, twoMax},
        "data/comparison.plot",
        "n=" + to_string(n) + ", T*=" + to_string(T_star) + ", " + to_string(nRuns) + " run(s) per ρ value",
        "Expected #interactions per agent",
        "Root of Avg MS Error"
    );
}