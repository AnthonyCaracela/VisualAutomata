#pragma once

#include "Automaton.h"

namespace fa::model {

// BASE IMPLEMENTATION - P1
class DFA final : public Automaton {
public:
    void addState(const State& state) override;
    void removeState(int id) override;
    void addTransition(const Transition& transition) override;
    void removeTransition(const Transition& transition) override;
    void validate() const override;
    std::string typeName() const override;
};

} // namespace fa::model
