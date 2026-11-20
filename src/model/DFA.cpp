#include "DFA.h"

#include "Exceptions.h"

#include <algorithm>

namespace fa::model {

void DFA::addState(const State& state) {
    addStateBase(state);
}

void DFA::removeState(int id) {
    removeStateBase(id);
}

void DFA::addTransition(const Transition& transition) {
    if (transition.symbol == '\0') {
        throw InvalidTransitionException("A DFA cannot contain epsilon transitions.");
    }
    const auto conflict = std::find_if(
        m_transitions.begin(), m_transitions.end(),
        [&transition](const Transition& current) {
            return current.from == transition.from && current.symbol == transition.symbol;
        });
    if (conflict != m_transitions.end()) {
        throw InvalidTransitionException("A DFA must have at most one transition per symbol and state.");
    }
    addTransitionBase(transition);
}

void DFA::removeTransition(const Transition& transition) {
    removeTransitionBase(transition);
}

void DFA::validate() const {
    validateBase();
    for (std::size_t i = 0; i < m_transitions.size(); ++i) {
        if (m_transitions[i].symbol == '\0') {
            throw InvalidAutomatonException("A DFA cannot contain epsilon transitions.");
        }
        for (std::size_t j = i + 1; j < m_transitions.size(); ++j) {
            if (m_transitions[i].from == m_transitions[j].from &&
                m_transitions[i].symbol == m_transitions[j].symbol) {
                throw InvalidAutomatonException("A DFA cannot contain nondeterministic transitions.");
            }
        }
    }
}

std::string DFA::typeName() const {
    return "DFA";
}

} // namespace fa::model
