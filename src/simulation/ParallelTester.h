#pragma once

#include "ISimulationStrategy.h"
#include "SimulationResult.h"
#include "model/Automaton.h"

#include <string>
#include <vector>

namespace fa::sim {

// Prueba varias cadenas en paralelo contra un mismo automata.
// Los resultados salen en el mismo orden que las cadenas de entrada.
class ParallelTester {
public:
    // maxThreads = 0 usa los hilos que tenga la maquina.
    explicit ParallelTester(unsigned maxThreads = 0);

    std::vector<SimulationResult> testAll(const fa::model::Automaton& automaton,
                                          const std::vector<std::string>& inputs,
                                          const ISimulationStrategy& strategy) const;

private:
    unsigned m_maxThreads;
};

}  // namespace fa::sim
