#pragma once

#include "Alphabet.h"

#include <iosfwd>

namespace fa::model {

struct Transition {
    int from{0};
    int to{0};
    char symbol{EPSILON};  // '\0' = epsilon
};

bool operator==(const Transition& a, const Transition& b);
bool operator!=(const Transition& a, const Transition& b);

// Orden total (from, symbol, to); permite usar Transition en std::set/std::map.
bool operator<(const Transition& a, const Transition& b);

// Ejemplo de salida: 0 --a--> 1   (epsilon se imprime como "eps")
std::ostream& operator<<(std::ostream& os, const Transition& t);

}  // namespace fa::model
