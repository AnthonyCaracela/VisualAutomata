#pragma once

#include "SimulationResult.h"
#include "model/Automaton.h"

#include <string>

namespace fa::sim {

// Interfaz del patron Strategy: cada tipo de automata simula a su manera.
class ISimulationStrategy {
public:
    virtual ~ISimulationStrategy() = default;
    virtual SimulationResult run(const fa::model::Automaton& automaton,
                                 const std::string& input) const = 0;
};

}  // namespace fa::sim
