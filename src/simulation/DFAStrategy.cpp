#include "DFAStrategy.h"

#include "model/DFA.h"

#include <cstddef>

namespace fa::sim {

SimulationResult DFAStrategy::run(const fa::model::Automaton& automaton,
                                  const std::string& input) const {
    SimulationResult result;

    const auto* dfa = dynamic_cast<const fa::model::DFA*>(&automaton);
    if (dfa == nullptr) {
        result.error = "DFAStrategy solo simula automatas de tipo DFA";
        return result;
    }

    const auto initial = dfa->initialStateId();
    if (!initial) {
        result.error = "El automata no tiene estado inicial";
        return result;
    }

    int current = *initial;
    // Paso 0: estado inicial, todavia no se consumio nada.
    result.steps.push_back({0, {current}, '\0', input});

    for (std::size_t i = 0; i < input.size(); ++i) {
        const char symbol = input[i];

        if (dfa->alphabet().count(symbol) == 0) {
            result.error = std::string("El simbolo '") + symbol +
                           "' no pertenece al alfabeto";
            return result;
        }

        const auto next = dfa->next(current, symbol);
        if (!next) {
            return result;  // AFD parcial: sin transicion, rechaza
        }

        current = *next;
        result.steps.push_back(
            {static_cast<int>(i) + 1, {current}, symbol, input.substr(i + 1)});
    }

    result.accepted = dfa->getState(current).final;
    return result;
}

}  // namespace fa::sim
