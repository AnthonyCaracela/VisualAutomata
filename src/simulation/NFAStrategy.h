#pragma once

#include "ISimulationStrategy.h"

namespace fa::sim {

// Strategy concreta: simula un AFN (varios estados activos y transiciones epsilon).
class NFAStrategy : public ISimulationStrategy {
public:
    SimulationResult run(const fa::model::Automaton& automaton,
                         const std::string& input) const override;
};

}  // namespace fa::sim
