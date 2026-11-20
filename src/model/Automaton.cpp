#include "Automaton.h"

#include "Exceptions.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <ostream>

namespace fa::model {

const std::vector<State>& Automaton::states() const {
    return m_states;
}

const std::vector<Transition>& Automaton::transitions() const {
    return m_transitions;
}

const std::set<char>& Automaton::alphabet() const {
    return m_alphabet;
}

std::optional<int> Automaton::initialStateId() const {
    const auto initial = std::find_if(
        m_states.begin(), m_states.end(),
        [](const State& state) { return state.initial; });
    if (initial == m_states.end()) {
        return std::nullopt;
    }
    return initial->id;
}

int Automaton::nextStateId() const {
    int candidate = 0;
    while (std::any_of(m_states.begin(), m_states.end(),
                       [candidate](const State& state) { return state.id == candidate; })) {
        if (candidate == std::numeric_limits<int>::max()) {
            throw InvalidAutomatonException("No free nonnegative state id remains.");
        }
        ++candidate;
    }
    return candidate;
}

void Automaton::setInitial(int id) {
    const auto selected = std::find_if(
        m_states.begin(), m_states.end(),
        [id](const State& state) { return state.id == id; });
    if (selected == m_states.end()) {
        throw InvalidAutomatonException("Cannot set an initial state that does not exist.");
    }
    for (auto& state : m_states) {
        state.initial = state.id == id;
    }
}

void Automaton::setFinal(int id, bool value) {
    const auto selected = std::find_if(
        m_states.begin(), m_states.end(),
        [id](const State& state) { return state.id == id; });
    if (selected == m_states.end()) {
        throw InvalidAutomatonException("Cannot set a final state that does not exist.");
    }
    selected->final = value;
}

void Automaton::setStateName(int id, const std::string& name) {
    if (name.empty()) {
        throw InvalidAutomatonException("A state name cannot be empty.");
    }
    const auto selected = std::find_if(
        m_states.begin(), m_states.end(),
        [id](const State& state) { return state.id == id; });
    if (selected == m_states.end()) {
        throw InvalidAutomatonException("Cannot rename a state that does not exist.");
    }
    selected->name = name;
}

void Automaton::moveState(int id, double x, double y) {
    if (!std::isfinite(x) || !std::isfinite(y)) {
        throw InvalidAutomatonException("State coordinates must be finite.");
    }
    const auto selected = std::find_if(
        m_states.begin(), m_states.end(),
        [id](const State& state) { return state.id == id; });
    if (selected == m_states.end()) {
        throw InvalidAutomatonException("Cannot move a state that does not exist.");
    }
    selected->x = x;
    selected->y = y;
}

void Automaton::addStateBase(const State& state) {
    if (std::any_of(m_states.begin(), m_states.end(),
                    [&state](const State& current) { return current.id == state.id; })) {
        throw InvalidAutomatonException("A state with this id already exists.");
    }
    if (!std::isfinite(state.x) || !std::isfinite(state.y)) {
        throw InvalidAutomatonException("State coordinates must be finite.");
    }
    m_states.push_back(state);
}

void Automaton::removeStateBase(int id) {
    const auto state = std::find_if(m_states.begin(), m_states.end(),
                                    [id](const State& current) { return current.id == id; });
    if (state == m_states.end()) {
        throw InvalidAutomatonException("Cannot remove a state that does not exist.");
    }

    m_states.erase(state);
    m_transitions.erase(
        std::remove_if(m_transitions.begin(), m_transitions.end(),
                       [id](const Transition& transition) {
                           return transition.from == id || transition.to == id;
                       }),
        m_transitions.end());
    rebuildAlphabet();
}

void Automaton::addTransitionBase(const Transition& transition) {
    const auto containsState = [this](int id) {
        return std::any_of(m_states.begin(), m_states.end(),
                           [id](const State& state) { return state.id == id; });
    };
    if (!containsState(transition.from) || !containsState(transition.to)) {
        throw InvalidTransitionException("Transition endpoints must refer to existing states.");
    }
    if (std::find(m_transitions.begin(), m_transitions.end(), transition) != m_transitions.end()) {
        throw InvalidTransitionException("This transition already exists.");
    }
    m_transitions.push_back(transition);
    if (transition.symbol != '\0') {
        m_alphabet.insert(transition.symbol);
    }
}

void Automaton::removeTransitionBase(const Transition& transition) {
    const auto found = std::find(m_transitions.begin(), m_transitions.end(), transition);
    if (found == m_transitions.end()) {
        throw InvalidTransitionException("Cannot remove a transition that does not exist.");
    }
    m_transitions.erase(found);
    rebuildAlphabet();
}

void Automaton::validateBase() const {
    if (m_states.empty()) {
        throw InvalidAutomatonException("An automaton must contain at least one state.");
    }

    const auto initialCount = std::count_if(
        m_states.begin(), m_states.end(), [](const State& state) { return state.initial; });
    if (initialCount != 1) {
        throw InvalidAutomatonException("An automaton must contain exactly one initial state.");
    }

    for (const auto& transition : m_transitions) {
        const auto containsState = [this](int id) {
            return std::any_of(m_states.begin(), m_states.end(),
                               [id](const State& state) { return state.id == id; });
        };
        if (!containsState(transition.from) || !containsState(transition.to)) {
            throw InvalidAutomatonException("Transition endpoints must refer to existing states.");
        }
    }
}

void Automaton::rebuildAlphabet() {
    m_alphabet.clear();
    for (const auto& transition : m_transitions) {
        if (transition.symbol != '\0') {
            m_alphabet.insert(transition.symbol);
        }
    }
}

std::ostream& operator<<(std::ostream& os, const Automaton& automaton) {
    os << automaton.typeName() << " { states: " << automaton.m_states.size()
       << ", transitions: " << automaton.m_transitions.size() << " }";
    return os;
}

} // namespace fa::model
