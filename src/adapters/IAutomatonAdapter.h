#pragma once

#include "../model/Automaton.h"

#include <memory>
#include <string>

namespace fa::adapters {

class IAutomatonAdapter {
public:
    virtual ~IAutomatonAdapter() = default;
    virtual std::unique_ptr<fa::model::Automaton> importFrom(const std::string& path) const = 0;
    virtual void exportTo(const fa::model::Automaton& automaton,
                          const std::string& path) const = 0;
};

} // namespace fa::adapters
