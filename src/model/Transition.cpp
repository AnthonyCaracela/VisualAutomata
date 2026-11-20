#include "Transition.h"

namespace fa::model {

bool Transition::operator==(const Transition& other) const {
    return from == other.from && to == other.to && symbol == other.symbol;
}

} // namespace fa::model
