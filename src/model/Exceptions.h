#pragma once

#include <stdexcept>
#include <string>

namespace fa::model {

class InvalidTransitionException : public std::runtime_error {
public:
    explicit InvalidTransitionException(const std::string& message)
        : std::runtime_error(message) {}
};

class InvalidAutomatonException : public std::runtime_error {
public:
    explicit InvalidAutomatonException(const std::string& message)
        : std::runtime_error(message) {}
};

} // namespace fa::model
