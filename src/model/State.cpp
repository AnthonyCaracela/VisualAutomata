#include "State.h"

namespace fa::model {

bool State::operator==(const State& other) const {
    return id == other.id;
}

} // namespace fa::model
