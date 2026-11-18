#pragma once

#include "Alphabet.h"
#include "State.h"
#include "Transition.h"

#include <iosfwd>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace fa::model {

// Clase base abstracta de AFD y AFN.
// Guarda estados y transiciones; el alfabeto se deduce de las transiciones.
class Automaton {
public:
    virtual ~Automaton() = default;

    // ---- Interfaz acordada con el grupo ----
    virtual void addState(const State& state);
    virtual void removeState(int id);
    virtual void addTransition(const Transition& transition) = 0;
    virtual void removeTransition(const Transition& transition);
    virtual void validate() const = 0;

    const std::vector<State>& states() const { return m_states; }
    const std::vector<Transition>& transitions() const { return m_transitions; }
    const std::set<char>& alphabet() const { return m_alphabet; }

    // ---- Extras (solo agregan, no cambian lo acordado) ----
    virtual std::string typeName() const = 0;  // "DFA" o "NFA"

    bool hasState(int id) const;
    const State& getState(int id) const;               // lanza InvalidAutomatonException
    std::optional<int> initialStateId() const;
    std::set<int> finalStateIds() const;
    int nextStateId() const;                           // id libre para un estado nuevo

    void setInitial(int id);                           // deja ESTE como unico inicial
    void setFinal(int id, bool value);
    void moveState(int id, double x, double y);
    void clear();

protected:
    Automaton() = default;

    // Ayudas para que DFA y NFA validen sin repetir codigo.
    void requireTransitionIsWellFormed(const Transition& t) const;  // estados, simbolo, duplicada
    void insertTransition(const Transition& t);
    void validateCommon() const;                                     // vacio, 1 inicial, transiciones

private:
    State& mutableState(int id);
    void rebuildAlphabet();

    std::vector<State> m_states;
    std::vector<Transition> m_transitions;
    std::set<char> m_alphabet;
};

// Imprime tipo, alfabeto, estados y transiciones.
std::ostream& operator<<(std::ostream& os, const Automaton& automaton);

}  // namespace fa::model
