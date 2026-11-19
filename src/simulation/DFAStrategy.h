#pragma once

#include "ISimulationStrategy.h"

namespace fa::sim {

// Strategy concreta: simula un AFD (un solo estado activo en cada paso).
class DFAStrategy : public ISimulationStrategy {
public:
    SimulationResult run(const fa::model::Automaton& automaton,
                         const std::string& input) const override;
};

}  // namespace fa::sim
