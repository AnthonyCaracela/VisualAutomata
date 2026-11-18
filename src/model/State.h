#pragma once

#include <iosfwd>
#include <string>

namespace fa::model {

struct State {
    int id{0};
    std::string name;
    bool initial{false};
    bool final{false};
    double x{0.0};
    double y{0.0};
};

bool operator==(const State& a, const State& b);
bool operator!=(const State& a, const State& b);

// Ejemplo de salida: q0 (id=0) [inicial, final] pos=(10, 20)
std::ostream& operator<<(std::ostream& os, const State& state);

}  // namespace fa::model
