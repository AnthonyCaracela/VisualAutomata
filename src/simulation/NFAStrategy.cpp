#include "NFAStrategy.h"

#include "../model/Exceptions.h"

#include <algorithm>
#include <functional>

namespace fa::sim {
namespace {

std::set<int> epsilonClosure(const fa::model::Automaton& automaton,
                             const std::set<int>& seed) {
    std::set<int> closure = seed;
    std::vector<int> pending(seed.begin(), seed.end());
    while (!pending.empty()) {
        const int state = pending.back();
        pending.pop_back();
        for (const auto& transition : automaton.transitions()) {
            if (transition.from == state && transition.symbol == '\0' &&
                closure.insert(transition.to).second) {
                pending.push_back(transition.to);
            }
        }
    }
    return closure;
}

} // namespace

SimulationResult NFAStrategy::run(const fa::model::Automaton& automaton,
                                  const std::string& input) const {
    SimulationResult result;
    try {
        automaton.validate();
        if (automaton.typeName() != "NFA") {
            throw fa::model::InvalidAutomatonException("NFAStrategy requires an NFA.");
        }

        const auto initial = std::find_if(
            automaton.states().begin(), automaton.states().end(),
            [](const fa::model::State& state) { return state.initial; });
        if (initial == automaton.states().end()) {
            throw fa::model::InvalidAutomatonException("The automaton has no initial state.");
        }

        std::set<int> current = epsilonClosure(automaton, {initial->id});
        result.steps.push_back({0, current, '\0', input});
        for (std::size_t i = 0; i < input.size(); ++i) {
            std::set<int> next;
            for (const auto& transition : automaton.transitions()) {
                if (current.count(transition.from) != 0 && transition.symbol == input[i]) {
                    next.insert(transition.to);
                }
            }
            current = epsilonClosure(automaton, next);
            result.steps.push_back(
                {static_cast<int>(i + 1), current, input[i], input.substr(i + 1)});
        }

        result.accepted = std::any_of(
            automaton.states().begin(), automaton.states().end(),
            [&current](const fa::model::State& state) {
                return state.final && current.count(state.id) != 0;
            });
        if (current.empty() && !input.empty()) {
            result.error = "No state is reachable for the input.";
        }
    } catch (const std::exception& exception) {
        result.error = exception.what();
    }
    return result;
}

} // namespace fa::sim
