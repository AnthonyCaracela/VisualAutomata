#pragma once

#include <string>

namespace fa::model {

struct State {
    int id{};
    std::string name;
    bool initial{};
    bool final{};
    double x{};
    double y{};

    bool operator==(const State& other) const;
};

} // namespace fa::model
