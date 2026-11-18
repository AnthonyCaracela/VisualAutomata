#include "Automaton.h"

#include "InvalidAutomatonException.h"
#include "InvalidTransitionException.h"

#include <algorithm>
#include <ostream>

namespace fa::model {

// ---------------------------------------------------------------- estados

void Automaton::addState(const State& state) {
    if (hasState(state.id)) {
        throw InvalidAutomatonException("Ya existe un estado con id " +
                                        std::to_string(state.id));
    }
    if (state.initial && initialStateId().has_value()) {
        throw InvalidAutomatonException(
            "El automata ya tiene un estado inicial (id " +
            std::to_string(*initialStateId()) + ")");
    }
    m_states.push_back(state);
}

void Automaton::removeState(int id) {
    auto it = std::find_if(m_states.begin(), m_states.end(),
                           [id](const State& s) { return s.id == id; });
    if (it == m_states.end()) {
        throw InvalidAutomatonException("No existe el estado con id " +
                                        std::to_string(id));
    }
    m_states.erase(it);

    // Las transiciones que tocan al estado eliminado ya no tienen sentido.
    m_transitions.erase(
        std::remove_if(m_transitions.begin(), m_transitions.end(),
                       [id](const Transition& t) { return t.from == id || t.to == id; }),
        m_transitions.end());
    rebuildAlphabet();
}

bool Automaton::hasState(int id) const {
    return std::any_of(m_states.begin(), m_states.end(),
                       [id](const State& s) { return s.id == id; });
}

const State& Automaton::getState(int id) const {
    auto it = std::find_if(m_states.begin(), m_states.end(),
                           [id](const State& s) { return s.id == id; });
    if (it == m_states.end()) {
        throw InvalidAutomatonException("No existe el estado con id " +
                                        std::to_string(id));
    }
    return *it;
}

State& Automaton::mutableState(int id) {
    return const_cast<State&>(getState(id));
}

std::optional<int> Automaton::initialStateId() const {
    for (const State& s : m_states) {
        if (s.initial) return s.id;
    }
    return std::nullopt;
}

std::set<int> Automaton::finalStateIds() const {
    std::set<int> ids;
    for (const State& s : m_states) {
        if (s.final) ids.insert(s.id);
    }
    return ids;
}

int Automaton::nextStateId() const {
    int next = 0;
    for (const State& s : m_states) {
        next = std::max(next, s.id + 1);
    }
    return next;
}

void Automaton::setInitial(int id) {
    State& target = mutableState(id);  // valida que exista
    for (State& s : m_states) s.initial = false;
    target.initial = true;
}

void Automaton::setFinal(int id, bool value) {
    mutableState(id).final = value;
}

void Automaton::moveState(int id, double x, double y) {
    State& s = mutableState(id);
    s.x = x;
    s.y = y;
}

void Automaton::clear() {
    m_states.clear();
    m_transitions.clear();
    m_alphabet.clear();
}

// ----------------------------------------------------------- transiciones

void Automaton::removeTransition(const Transition& transition) {
    auto it = std::find(m_transitions.begin(), m_transitions.end(), transition);
    if (it == m_transitions.end()) {
        throw InvalidTransitionException("La transicion no existe");
    }
    m_transitions.erase(it);
    rebuildAlphabet();
}

void Automaton::requireTransitionIsWellFormed(const Transition& t) const {
    if (!hasState(t.from)) {
        throw InvalidTransitionException("El estado origen " + std::to_string(t.from) +
                                         " no existe");
    }
    if (!hasState(t.to)) {
        throw InvalidTransitionException("El estado destino " + std::to_string(t.to) +
                                         " no existe");
    }
    if (!isEpsilon(t.symbol) && !isValidSymbol(t.symbol)) {
        throw InvalidTransitionException("Simbolo no valido en la transicion");
    }
    if (std::find(m_transitions.begin(), m_transitions.end(), t) != m_transitions.end()) {
        throw InvalidTransitionException("La transicion ya existe");
    }
}

void Automaton::insertTransition(const Transition& t) {
    m_transitions.push_back(t);
    if (!isEpsilon(t.symbol)) m_alphabet.insert(t.symbol);
}

void Automaton::rebuildAlphabet() {
    m_alphabet.clear();
    for (const Transition& t : m_transitions) {
        if (!isEpsilon(t.symbol)) m_alphabet.insert(t.symbol);
    }
}

// ------------------------------------------------------------- validacion

void Automaton::validateCommon() const {
    if (m_states.empty()) {
        throw InvalidAutomatonException("El automata no tiene estados");
    }

    const auto initials = std::count_if(m_states.begin(), m_states.end(),
                                        [](const State& s) { return s.initial; });
    if (initials == 0) {
        throw InvalidAutomatonException("El automata no tiene estado inicial");
    }
    if (initials > 1) {
        throw InvalidAutomatonException("El automata tiene mas de un estado inicial");
    }

    std::set<int> ids;
    for (const State& s : m_states) {
        if (!ids.insert(s.id).second) {
            throw InvalidAutomatonException("Id de estado repetido: " +
                                            std::to_string(s.id));
        }
    }

    std::set<Transition> seen;
    for (const Transition& t : m_transitions) {
        if (!ids.count(t.from) || !ids.count(t.to)) {
            throw InvalidTransitionException("Transicion con estado inexistente");
        }
        if (!isEpsilon(t.symbol) && !isValidSymbol(t.symbol)) {
            throw InvalidTransitionException("Transicion con simbolo no valido");
        }
        if (!seen.insert(t).second) {
            throw InvalidTransitionException("Transicion repetida");
        }
    }
}

// ----------------------------------------------------------------- salida

std::ostream& operator<<(std::ostream& os, const Automaton& a) {
    os << a.typeName() << " (" << a.states().size() << " estados, "
       << a.transitions().size() << " transiciones)\n";
    os << "  Alfabeto: " << alphabetToString(a.alphabet()) << '\n';
    os << "  Estados:\n";
    for (const State& s : a.states()) os << "    " << s << '\n';
    os << "  Transiciones:\n";
    for (const Transition& t : a.transitions()) os << "    " << t << '\n';
    return os;
}

}  // namespace fa::model
