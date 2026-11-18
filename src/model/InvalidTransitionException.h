#pragma once

#include <stdexcept>
#include <string>

namespace fa::model {

// Se lanza cuando una transicion es invalida: estados inexistentes,
// simbolo no permitido, duplicada o que rompe el determinismo de un AFD.
class InvalidTransitionException : public std::runtime_error {
public:
    explicit InvalidTransitionException(const std::string& message)
        : std::runtime_error(message) {}
};

}  // namespace fa::model
