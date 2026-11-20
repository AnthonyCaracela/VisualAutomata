#pragma once

#include "SimulationResult.h"

#include "../model/Automaton.h"

namespace fa::sim {

class ISimulationStrategy {
public:
    virtual ~ISimulationStrategy() = default;
    virtual SimulationResult run(const fa::model::Automaton& automaton,
                                 const std::string& input) const = 0;
};

} // namespace fa::sim
