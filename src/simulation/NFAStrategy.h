#pragma once

#include "ISimulationStrategy.h"

namespace fa::sim {

class NFAStrategy final : public ISimulationStrategy {
public:
    SimulationResult run(const fa::model::Automaton& automaton,
                         const std::string& input) const override;
};

} // namespace fa::sim
