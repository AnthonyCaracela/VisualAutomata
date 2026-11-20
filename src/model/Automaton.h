#pragma once

#include "State.h"
#include "Transition.h"

#include <iosfwd>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace fa::model {

// BASE IMPLEMENTATION - P1
class Automaton {
public:
    virtual ~Automaton() = default;

    virtual void addState(const State& state) = 0;
    virtual void removeState(int id) = 0;
    virtual void addTransition(const Transition& transition) = 0;
    virtual void removeTransition(const Transition& transition) = 0;
    virtual void validate() const = 0;
    virtual std::string typeName() const = 0;

    const std::vector<State>& states() const;
    const std::vector<Transition>& transitions() const;
    const std::set<char>& alphabet() const;
    std::optional<int> initialStateId() const;
    int nextStateId() const;

    void setInitial(int id);
    void setFinal(int id, bool value);
    void setStateName(int id, const std::string& name);
    void moveState(int id, double x, double y);

    friend std::ostream& operator<<(std::ostream& os, const Automaton& automaton);

protected:
    void addStateBase(const State& state);
    void removeStateBase(int id);
    void addTransitionBase(const Transition& transition);
    void removeTransitionBase(const Transition& transition);
    void validateBase() const;
    void rebuildAlphabet();

    std::vector<State> m_states;
    std::vector<Transition> m_transitions;
    std::set<char> m_alphabet;
};

} // namespace fa::model
