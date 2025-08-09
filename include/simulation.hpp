#ifndef __SIMULATION_HPP__
#define __SIMULATION_HPP__

#include <vector>
#include <functional>
#include <string>
#include "agent.hpp"
#include "utils.hpp"


// Holds simulation quantities the metric function might make use of
struct MetricInfo {
    float avgN;       // average number of active agents, lambda_a/lambda_d 
    float IPL;        // average interactions per lifetime, 2lambda_g/lambda_d
    float actualMax;  // max value over all currently active agents
};

// Returns the asymptotic average result of the specified metric
// for a single run. The arguments given to a call to `metric`
// are (in this order), the current values for the quantities
// held by the MetricInfo type, and the estimate of some agent.
// If the agent type is `ThresholdAgent`, the threshold
// the agents will use is `threshold`. Initially there will be `n` agents.
// The agents internal values will be drawn from a uniform real distribution in [0, 1000[
// The simulation parameters will be the following:
// lambda_d = `2/(n*IPL)`
// lamda_a = `n*lambda_d`
// lambda_g = `1/n`
// simulation window = `max(simTimePerLifetime/lambda_d, 3000)`
template<ValidAgentType AgentType>
float asymptoticMetricValue(int n, int simTimePerLifetime, float IPL, const std::function<float(const MetricInfo&, float)>& metric, int threshold = 10) noexcept;

// For each IPL value in the given `IPLs` list, runs `nRuns` simulations
// and generates a plot where the x values correspond to IPLs and y values
// are the average asymptotic metric values over all the runs
// If the agent type is `ThresholdAgent`, the threshold
// the agents will use is `threshold`. Initially there will be `n` agents.
// The agents internal values will be drawn from a uniform real distribution in [0, 1000[
// The simulation parameters will be the following:
// lambda_d = `2/(n*IPL)`
// lamda_a = `n*lambda_d`
// lambda_g = `1/n`
// simulation window = `max(simTimePerLifetime/lambda_d, 3000)`
template<ValidAgentType AgentType>
Curve generateCurve(int n, int simTimePerLifetime, const std::vector<float>& IPLs, int nRuns, const std::function<float(const MetricInfo&, float)>& metric, const std::string& label = "", int threshold = 10) noexcept;

#endif