#pragma once

#include "SimulationStep.h"

#include <string>
#include <vector>

namespace fa::sim {

struct SimulationResult {
    bool accepted{};
    std::vector<SimulationStep> steps;
    std::string error;
};

} // namespace fa::sim
