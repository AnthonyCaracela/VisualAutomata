#pragma once

#include <stdexcept>
#include <string>

namespace fa::model {

// Se lanza cuando el automata completo es invalido: sin estados,
// sin estado inicial, con mas de un inicial, ids repetidos, etc.
class InvalidAutomatonException : public std::runtime_error {
public:
    explicit InvalidAutomatonException(const std::string& message)
        : std::runtime_error(message) {}
};

}  // namespace fa::model
