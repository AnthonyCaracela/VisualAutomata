#pragma once

#include "IAutomatonAdapter.h"

namespace fa::adapters {

class JffAdapter final : public IAutomatonAdapter {
public:
    std::unique_ptr<fa::model::Automaton> importFrom(const std::string& path) const override;
    void exportTo(const fa::model::Automaton& automaton,
                  const std::string& path) const override;
};

} // namespace fa::adapters
