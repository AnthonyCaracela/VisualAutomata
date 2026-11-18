#pragma once

#include "Automaton.h"

#include <optional>

namespace fa::model {

// Automata Finito Determinista: sin epsilon y a lo mucho una transicion
// por cada par (estado, simbolo). Puede ser parcial (sin transicion = rechaza).
class DFA : public Automaton {
public:
    void addTransition(const Transition& transition) override;
    void validate() const override;
    std::string typeName() const override { return "DFA"; }

    // Extras utiles para la simulacion y para ensenar.
    std::optional<int> next(int from, char symbol) const;  // estado siguiente, si existe
    bool isComplete() const;  // todos los estados tienen transicion para todo el alfabeto
};

}  // namespace fa::model
