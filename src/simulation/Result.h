#pragma once

#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace fa::sim {

// Template generico: contiene un valor T o un mensaje de error.
// Evita devolver codigos magicos o lanzar excepciones en el camino normal.
template <typename T>
class Result {
public:
    static Result success(T value) {
        Result r;
        r.m_value = std::move(value);
        return r;
    }

    static Result failure(std::string error) {
        Result r;
        r.m_error = std::move(error);
        return r;
    }

    bool ok() const { return m_value.has_value(); }
    explicit operator bool() const { return ok(); }

    // Lanza std::logic_error si no hay valor.
    const T& value() const {
        if (!m_value) {
            throw std::logic_error("Result sin valor: " + m_error);
        }
        return *m_value;
    }

    T valueOr(T fallback) const { return m_value ? *m_value : std::move(fallback); }

    const std::string& error() const { return m_error; }

    // Transforma el valor con una lambda; si hay error, lo propaga tal cual.
    template <typename F>
    auto map(F&& fn) const -> Result<std::invoke_result_t<F, const T&>> {
        using U = std::invoke_result_t<F, const T&>;
        if (!ok()) {
            return Result<U>::failure(m_error);
        }
        return Result<U>::success(fn(*m_value));
    }

private:
    Result() = default;

    std::optional<T> m_value;
    std::string m_error;
};

}  // namespace fa::sim
