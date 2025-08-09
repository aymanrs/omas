#include "simulation.hpp"
#include "twomax_agent.hpp"
#include "omniscient_agent.hpp"
#include "threshold_agent.hpp"
#include "announce_agent.hpp"

int main() {
    const int n = 40, simTimePerLifetime = 30, nRuns = 10;
    std::vector<float> InteractionsPerLifetime;
    const float a = 20, b = 4000, nIPLs = 15;
    for(float IPLIdx = 0; IPLIdx < nIPLs;IPLIdx++) {
        float pt = IPLIdx/nIPLs;
        pt *= pt;
        InteractionsPerLifetime.push_back(a+(b-a)*pt);
    }
    std::function<float(const MetricInfo&, float)> metric = [](const MetricInfo& metricInfo, float estimate) {
        float error = metricInfo.actualMax != estimate;
        return error;
    };
    std::vector<Curve> curves = {
        generateCurve<TwoMaxAgent>(n, simTimePerLifetime, InteractionsPerLifetime, nRuns, metric, "Proposed Algorithm"),
        generateCurve<OmniscientAgent>(n, simTimePerLifetime, InteractionsPerLifetime, nRuns, metric, "Lower Bound"),
    };
    using std::to_string;
    savePlot(
        curves,
        "data/comparison.plot",
        "n=" + to_string(n) + ", Simulation Window = " + to_string(simTimePerLifetime) + "×Avg Lifetime, " + to_string(nRuns) + " run(s) per L value",
        "Expected #interactions per agent (L)",
        "Asymptotic Probability of not Guessing the Correct Max"
    );
}