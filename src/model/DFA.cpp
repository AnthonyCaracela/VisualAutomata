#include "DFA.h"

#include "InvalidAutomatonException.h"
#include "InvalidTransitionException.h"

#include <set>
#include <utility>

namespace fa::model {

void DFA::addTransition(const Transition& transition) {
    if (isEpsilon(transition.symbol)) {
        throw InvalidTransitionException("Un AFD no admite transiciones epsilon");
    }
    requireTransitionIsWellFormed(transition);

    if (next(transition.from, transition.symbol).has_value()) {
        throw InvalidTransitionException(
            "El AFD ya tiene una transicion desde el estado " +
            std::to_string(transition.from) + " con el simbolo '" +
            std::string(1, transition.symbol) + "'");
    }
    insertTransition(transition);
}

void DFA::validate() const {
    validateCommon();

    std::set<std::pair<int, char>> seen;
    for (const Transition& t : transitions()) {
        if (isEpsilon(t.symbol)) {
            throw InvalidAutomatonException("Un AFD no puede tener transiciones epsilon");
        }
        if (!seen.insert({t.from, t.symbol}).second) {
            throw InvalidAutomatonException(
                "El AFD es no determinista en el estado " + std::to_string(t.from) +
                " con el simbolo '" + std::string(1, t.symbol) + "'");
        }
    }
}

std::optional<int> DFA::next(int from, char symbol) const {
    for (const Transition& t : transitions()) {
        if (t.from == from && t.symbol == symbol) return t.to;
    }
    return std::nullopt;
}

bool DFA::isComplete() const {
    for (const State& s : states()) {
        for (char c : alphabet()) {
            if (!next(s.id, c).has_value()) return false;
        }
    }
    return true;
}

}  // namespace fa::model
