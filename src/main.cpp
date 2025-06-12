#include <fstream>
#include <vector>
#include <random>
#include <optional>
#include <iostream>
#ifdef DEBUG
    #include <stdexcept>
#endif
#include "utils.hpp"
#include "agent.hpp"
#include "optimal_agent.hpp"

float asymptote(AgentType type, int n, int threshold, int simTime, float rho) {
    OptimalAgent::rho = rho;
    OptimalAgent::time = 0;
    OptimalAgent::n = n;
    Agent* agents[n];
    int startAvgTime = 1000;
    int actualMax = std::numeric_limits<int>::min();
    float runAsymptote = 0;
    float totalSquareError = 0;
    for(int i = 0;i < n;i++) {
        agents[i] = Agent::makeAgent(type, rng()%1000, threshold);
        if(agents[i]->value() > actualMax) actualMax = agents[i]->value();
    }
    for(int i = 0;i < n;i++) {
        float error = actualMax-agents[i]->estimate();
        totalSquareError += error*error;
    }
    for(int t = 0;t < simTime;t++){
        OptimalAgent::time = t;
        for(int i = 0;i < n;i++) if(urd(rng) < rho) {
            int oldVal = agents[i]->value();
            float oldEst = agents[i]->estimate();
            Agent* agentToInform = agents[randomDifferentIndex(n, i)];
            float error = actualMax-agentToInform->estimate();
            totalSquareError -= error*error;
            agents[i]->leave(agentToInform);
            error = actualMax-agentToInform->estimate();
            totalSquareError += error*error;
            if(type != AgentType::Optimal){
                delete agents[i];
                agents[i] = Agent::makeAgent(type, rng()%1000, threshold);
            }
            if(oldVal == actualMax || agents[i]->value() > actualMax) {
                actualMax = std::numeric_limits<int>::min();
                for(int i = 0;i < n;i++) if(agents[i]->value() > actualMax) actualMax = agents[i]->value();
                totalSquareError = 0;
                for(int i = 0;i < n;i++) {
                    error = actualMax-agents[i]->estimate();
                    totalSquareError += error*error;
                }
            } else {
                float error = actualMax-oldEst;
                totalSquareError -= error*error;
                error = actualMax-agents[i]->estimate();
                totalSquareError += error*error;
            }
        }
        auto [i,j] = randomPair(n);
        float iError = actualMax-agents[i]->estimate();
        float jError = actualMax-agents[j]->estimate();
        totalSquareError -= iError*iError + jError*jError;
        agents[i]->interact(agents[j]);
        iError = actualMax-agents[i]->estimate();
        jError = actualMax-agents[j]->estimate();
        totalSquareError += iError*iError + jError*jError;
        if(t >= startAvgTime) runAsymptote += sqrt(std::max(0.f, totalSquareError/n));
    }
    for(int i = 0;i < n;i++) delete agents[i];
    runAsymptote /= (simTime-startAvgTime);
    return runAsymptote;
}

Curve generateCurve(AgentType type, int n, int threshold, int simTime, const std::vector<float>& rhos, int nRuns, const std::string& label = "") {
    Curve curve;
    curve.label = label;
    curve.x = rhos;
    curve.y.reserve(rhos.size());
    for(float rho : rhos) {
        float avgAsymptote = 0;
        for(int _ = 0;_ < nRuns;_++) avgAsymptote += asymptote(type, n, threshold, simTime, rho);
        curve.y.push_back(avgAsymptote/nRuns);
    }
    return curve;
}

Curve run(AgentType type, int n, int threshold, int simTime, float rho){
    OptimalAgent::n = n;
    OptimalAgent:rho = rho;
    OptimalAgent::time = 0;
    Curve curve;
    Agent* agents[n];
    for(int i = 0;i < n;i++) agents[i] = Agent::makeAgent(type, rng()%1000, threshold);
    for(int t = 0;t < simTime;t++){
        OptimalAgent::time = t;
        for(int i = 0;i < n;i++) if(urd(rng) < rho) {
            agents[i]->leave(agents[randomDifferentIndex(n, i)]);
            if(type != AgentType::Optimal){
                delete agents[i];
                agents[i] = Agent::makeAgent(type, rng()%1000, threshold);
            }
        }
        auto [i,j] = randomPair(n);
        agents[i]->interact(agents[j]);
        curve.x.push_back(t);
        int mx = agents[0]->value();for(int i = 1;i < n;i++) mx = std::max(mx, agents[i]->value());
        float se = 0;for(int i = 0;i < n;i++) {
            float error = mx-agents[i]->estimate();
            se += error*error;
        }
        curve.y.push_back(sqrt(se/n));
    }
    for(int i = 0;i < n;i++) delete agents[i];
    return curve;
}

int main() {
    const int n = 25, T_star = 11, simTime = 2000, nRuns = 250;
    std::vector<float> rhos;
    const float a = 0, b = 0.001, nRhos = 40;
    for(float rhoIdx = 0; rhoIdx < nRhos;rhoIdx++) rhos.push_back(a+(b-a)*rhoIdx/nRhos);
    Curve optimal = generateCurve(AgentType::Optimal, n, T_star, simTime, rhos, nRuns, "optimal");
    Curve threshold = generateCurve(AgentType::Threshold, n, T_star, simTime, rhos, nRuns, "threshold");
    Curve twoMax = generateCurve(AgentType::TwoMax, n, T_star, simTime, rhos, nRuns, "2max");
    for(float& y_i : optimal.y) y_i /= M_SQRT2f;
    using std::to_string;
    savePlot(
        {threshold, twoMax, optimal},
        "data/comparison.plot",
        "n=" + to_string(n) + ", T*=" + to_string(T_star) + ", " + to_string(nRuns) + " run(s) per ρ value",
        "ρ",
        "Avg RMS Error"
    );
}