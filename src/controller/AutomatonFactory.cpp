#include "AutomatonFactory.h"

#include "../model/DFA.h"
#include "../model/Exceptions.h"
#include "../model/NFA.h"

namespace fa::ctrl {

std::unique_ptr<fa::model::Automaton> AutomatonFactory::create(AutomatonType type) {
    switch (type) {
    case AutomatonType::DFA:
        return std::make_unique<fa::model::DFA>();
    case AutomatonType::NFA:
        return std::make_unique<fa::model::NFA>();
    }
    throw fa::model::InvalidAutomatonException("Unknown automaton type.");
}

} // namespace fa::ctrl
