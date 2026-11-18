#pragma once

#include <cctype>
#include <set>
#include <string>

namespace fa::model {

// Simbolo reservado para la transicion epsilon (vacia).
inline constexpr char EPSILON = '\0';

// El alfabeto es el conjunto de simbolos usados en las transiciones
// (sin contar epsilon). Se mantiene como std::set<char> para que
// Automaton::alphabet() cumpla la firma acordada con el grupo.
using Alphabet = std::set<char>;

inline bool isEpsilon(char symbol) {
    return symbol == EPSILON;
}

// Un simbolo normal debe ser un caracter imprimible y no un espacio.
inline bool isValidSymbol(char symbol) {
    return std::isgraph(static_cast<unsigned char>(symbol)) != 0;
}

// Devuelve el alfabeto con formato "{a, b, c}".
inline std::string alphabetToString(const Alphabet& alphabet) {
    std::string out = "{";
    bool first = true;
    for (char c : alphabet) {
        if (!first) out += ", ";
        out += c;
        first = false;
    }
    out += "}";
    return out;
}

}  // namespace fa::model
