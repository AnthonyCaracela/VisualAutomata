#include "Transition.h"

#include <ostream>
#include <tuple>

namespace fa::model {

bool operator==(const Transition& a, const Transition& b) {
    return a.from == b.from && a.to == b.to && a.symbol == b.symbol;
}

bool operator!=(const Transition& a, const Transition& b) {
    return !(a == b);
}

bool operator<(const Transition& a, const Transition& b) {
    return std::tie(a.from, a.symbol, a.to) < std::tie(b.from, b.symbol, b.to);
}

std::ostream& operator<<(std::ostream& os, const Transition& t) {
    os << t.from << " --";
    if (isEpsilon(t.symbol)) {
        os << "eps";
    } else {
        os << t.symbol;
    }
    os << "--> " << t.to;
    return os;
}

}  // namespace fa::model
