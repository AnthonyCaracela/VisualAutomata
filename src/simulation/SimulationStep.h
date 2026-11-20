#pragma once

#include <set>
#include <string>

namespace fa::sim {

struct SimulationStep {
    int index{};
    std::set<int> currentStates;
    char consumed{};
    std::string remaining;
};

} // namespace fa::sim
