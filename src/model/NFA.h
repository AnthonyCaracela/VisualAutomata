#pragma once

#include "Automaton.h"

#include <set>

namespace fa::model {

// Automata Finito No determinista: admite transiciones epsilon y varias
// transiciones con el mismo (estado, simbolo).
class NFA : public Automaton {
public:
    void addTransition(const Transition& transition) override;
    void validate() const override;
    std::string typeName() const override { return "NFA"; }

    // Extras utiles para la simulacion (P2 puede usarlos o hacer los suyos).
    std::set<int> targets(int from, char symbol) const;           // destinos con ese simbolo
    std::set<int> epsilonClosure(const std::set<int>& states) const;
};

}  // namespace fa::model
