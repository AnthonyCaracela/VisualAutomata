#include "NFA.h"

namespace fa::model {

void NFA::addState(const State& state) {
    addStateBase(state);
}

void NFA::removeState(int id) {
    removeStateBase(id);
}

void NFA::addTransition(const Transition& transition) {
    addTransitionBase(transition);
}

void NFA::removeTransition(const Transition& transition) {
    removeTransitionBase(transition);
}

void NFA::validate() const {
    validateBase();
}

std::string NFA::typeName() const {
    return "NFA";
}

} // namespace fa::model
