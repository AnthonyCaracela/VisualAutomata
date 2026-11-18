#include "State.h"

#include <ostream>

namespace fa::model {

bool operator==(const State& a, const State& b) {
    return a.id == b.id && a.name == b.name && a.initial == b.initial &&
           a.final == b.final && a.x == b.x && a.y == b.y;
}

bool operator!=(const State& a, const State& b) {
    return !(a == b);
}

std::ostream& operator<<(std::ostream& os, const State& state) {
    if (state.name.empty()) {
        os << 'q' << state.id;
    } else {
        os << state.name;
    }
    os << " (id=" << state.id << ")";

    if (state.initial || state.final) {
        os << " [";
        if (state.initial) os << "inicial";
        if (state.initial && state.final) os << ", ";
        if (state.final) os << "final";
        os << "]";
    }
    os << " pos=(" << state.x << ", " << state.y << ")";
    return os;
}

}  // namespace fa::model
