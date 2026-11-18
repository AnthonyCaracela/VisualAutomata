#include "NFA.h"

#include <vector>

namespace fa::model {

void NFA::addTransition(const Transition& transition) {
    requireTransitionIsWellFormed(transition);
    insertTransition(transition);
}

void NFA::validate() const {
    validateCommon();
}

std::set<int> NFA::targets(int from, char symbol) const {
    std::set<int> result;
    for (const Transition& t : transitions()) {
        if (t.from == from && t.symbol == symbol) result.insert(t.to);
    }
    return result;
}

std::set<int> NFA::epsilonClosure(const std::set<int>& states) const {
    std::set<int> closure(states);
    std::vector<int> pending(states.begin(), states.end());

    while (!pending.empty()) {
        const int current = pending.back();
        pending.pop_back();
        for (int neighbor : targets(current, EPSILON)) {
            if (closure.insert(neighbor).second) pending.push_back(neighbor);
        }
    }
    return closure;
}

}  // namespace fa::model
