#pragma once

namespace fa::model {

struct Transition {
    int from{};
    int to{};
    char symbol{};

    bool operator==(const Transition& other) const;
};

} // namespace fa::model
