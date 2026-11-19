#include "NFAStrategy.h"

#include "model/NFA.h"

#include <algorithm>
#include <cstddef>
#include <set>

namespace fa::sim {

SimulationResult NFAStrategy::run(const fa::model::Automaton& automaton,
                                  const std::string& input) const {
    SimulationResult result;

    const auto* nfa = dynamic_cast<const fa::model::NFA*>(&automaton);
    if (nfa == nullptr) {
        result.error = "NFAStrategy solo simula automatas de tipo NFA";
        return result;
    }

    const auto initial = nfa->initialStateId();
    if (!initial) {
        result.error = "El automata no tiene estado inicial";
        return result;
    }

    // Paso 0: clausura epsilon del estado inicial, todavia sin consumir nada.
    std::set<int> current = nfa->epsilonClosure({*initial});
    current.insert(*initial);
    result.steps.push_back({0, current, '\0', input});

    for (std::size_t i = 0; i < input.size(); ++i) {
        const char symbol = input[i];

        if (nfa->alphabet().count(symbol) == 0) {
            result.error = std::string("El simbolo '") + symbol +
                           "' no pertenece al alfabeto";
            return result;
        }

        // Union de los destinos de todos los estados activos con este simbolo.
        std::set<int> moved;
        for (int state : current) {
            const std::set<int> targets = nfa->targets(state, symbol);
            moved.insert(targets.begin(), targets.end());
        }

        current = nfa->epsilonClosure(moved);
        current.insert(moved.begin(), moved.end());

        result.steps.push_back(
            {static_cast<int>(i) + 1, current, symbol, input.substr(i + 1)});

        if (current.empty()) {
            return result;  // no quedan estados activos: rechaza
        }
    }

    const std::set<int> finals = nfa->finalStateIds();
    result.accepted = std::any_of(current.begin(), current.end(),
                                  [&finals](int id) { return finals.count(id) > 0; });
    return result;
}

}  // namespace fa::sim
