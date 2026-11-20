#include "DFAStrategy.h"

#include "../model/Exceptions.h"

#include <algorithm>

namespace fa::sim {

SimulationResult DFAStrategy::run(const fa::model::Automaton& automaton,
                                  const std::string& input) const {
    SimulationResult result;
    try {
        automaton.validate();
        if (automaton.typeName() != "DFA") {
            throw fa::model::InvalidAutomatonException("DFAStrategy requires a DFA.");
        }

        const auto initial = std::find_if(
            automaton.states().begin(), automaton.states().end(),
            [](const fa::model::State& state) { return state.initial; });
        if (initial == automaton.states().end()) {
            throw fa::model::InvalidAutomatonException("The automaton has no initial state.");
        }

        int current = initial->id;
        result.steps.push_back({0, {current}, '\0', input});
        for (std::size_t i = 0; i < input.size(); ++i) {
            const auto transition = std::find_if(
                automaton.transitions().begin(), automaton.transitions().end(),
                [current, &input, i](const fa::model::Transition& candidate) {
                    return candidate.from == current && candidate.symbol == input[i];
                });
            if (transition == automaton.transitions().end()) {
                result.error = "No transition for input symbol.";
                result.steps.push_back({static_cast<int>(i + 1), {}, input[i], input.substr(i + 1)});
                return result;
            }
            current = transition->to;
            result.steps.push_back(
                {static_cast<int>(i + 1), {current}, input[i], input.substr(i + 1)});
        }
        const auto finalState = std::find_if(
            automaton.states().begin(), automaton.states().end(),
            [current](const fa::model::State& state) { return state.id == current; });
        result.accepted = finalState != automaton.states().end() && finalState->final;
    } catch (const std::exception& exception) {
        result.error = exception.what();
    }
    return result;
}

} // namespace fa::sim
