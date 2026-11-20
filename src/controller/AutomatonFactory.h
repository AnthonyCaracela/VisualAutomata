#pragma once

#include "../model/Automaton.h"

#include <memory>

namespace fa::ctrl {

enum class AutomatonType { DFA, NFA };

// BASE IMPLEMENTATION - P4
class AutomatonFactory {
public:
    static std::unique_ptr<fa::model::Automaton> create(AutomatonType type);
};

} // namespace fa::ctrl
