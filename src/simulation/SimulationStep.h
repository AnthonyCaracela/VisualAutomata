#pragma once

#include <set>
#include <string>

namespace fa::sim {

// Un paso de la ejecucion: estados activos despues de consumir un simbolo.
struct SimulationStep {
    int index{0};
    std::set<int> currentStates;
    char consumed{'\0'};
    std::string remaining;
};

}  // namespace fa::sim
