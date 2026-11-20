#pragma once

#include "ISimulationStrategy.h"

#include <string>
#include <vector>

namespace fa::sim {

class ParallelTester {
public:
    explicit ParallelTester(unsigned maxThreads = 0);

    std::vector<SimulationResult> testAll(
        const fa::model::Automaton& automaton,
        const std::vector<std::string>& inputs,
        const ISimulationStrategy& strategy) const;

private:
    unsigned m_maxThreads;
};

} // namespace fa::sim
