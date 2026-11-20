#include "JsonAdapter.h"

#include "../controller/AutomatonFactory.h"
#include "../model/Exceptions.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

#include <cmath>
#include <limits>
#include <stdexcept>

namespace fa::adapters {
namespace {

[[noreturn]] void invalidJson(const QString& message) {
    throw std::runtime_error((QStringLiteral("Invalid automaton JSON: ") + message).toStdString());
}

int requiredInt(const QJsonObject& object, const QString& key) {
    const auto value = object.value(key);
    if (!value.isDouble() || !std::isfinite(value.toDouble()) ||
        std::floor(value.toDouble()) != value.toDouble() ||
        value.toDouble() < std::numeric_limits<int>::min() ||
        value.toDouble() > std::numeric_limits<int>::max()) {
        invalidJson(QStringLiteral("'") + key + QStringLiteral("' must be an integer."));
    }
    return static_cast<int>(value.toDouble());
}

double requiredNumber(const QJsonObject& object, const QString& key) {
    const auto value = object.value(key);
    if (!value.isDouble() || !std::isfinite(value.toDouble())) {
        invalidJson(QStringLiteral("'") + key + QStringLiteral("' must be a finite number."));
    }
    return value.toDouble();
}

bool requiredBool(const QJsonObject& object, const QString& key) {
    const auto value = object.value(key);
    if (value.isBool() == false) {
        invalidJson(QStringLiteral("'") + key + QStringLiteral("' must be a boolean."));
    }
    return value.toBool();
}

QString requiredString(const QJsonObject& object, const QString& key) {
    const auto value = object.value(key);
    if (!value.isString()) {
        invalidJson(QStringLiteral("'") + key + QStringLiteral("' must be a string."));
    }
    return value.toString();
}

char requiredSymbol(const QJsonObject& object) {
    const QString symbol = requiredString(object, QStringLiteral("symbol"));
    if (symbol.isEmpty()) {
        return '\0';
    }
    const QByteArray utf8 = symbol.toUtf8();
    if (utf8.size() != 1) {
        invalidJson(QStringLiteral("'symbol' must be empty or one single-byte character."));
    }
    return utf8.at(0);
}

} // namespace

std::unique_ptr<fa::model::Automaton> JsonAdapter::importFrom(const std::string& path) const {
    QFile file(QString::fromStdString(path));
    if (!file.open(QIODevice::ReadOnly)) {
        throw std::runtime_error("Cannot open JSON file for reading: " + path +
                                 " (" + file.errorString().toStdString() + ")");
    }

    QJsonParseError parseError{};
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        invalidJson(parseError.errorString().isEmpty()
                        ? QStringLiteral("the document root must be an object.")
                        : parseError.errorString());
    }
    const QJsonObject root = document.object();
    const QString type = requiredString(root, QStringLiteral("type"));
    fa::ctrl::AutomatonType automatonType;
    if (type == QStringLiteral("DFA")) {
        automatonType = fa::ctrl::AutomatonType::DFA;
    } else if (type == QStringLiteral("NFA")) {
        automatonType = fa::ctrl::AutomatonType::NFA;
    } else {
        invalidJson(QStringLiteral("'type' must be 'DFA' or 'NFA'."));
    }

    const auto statesValue = root.value(QStringLiteral("states"));
    const auto transitionsValue = root.value(QStringLiteral("transitions"));
    if (!statesValue.isArray() || !transitionsValue.isArray()) {
        invalidJson(QStringLiteral("'states' and 'transitions' must be arrays."));
    }

    auto automaton = fa::ctrl::AutomatonFactory::create(automatonType);
    try {
        for (const auto value : statesValue.toArray()) {
            if (!value.isObject()) {
                invalidJson(QStringLiteral("each state must be an object."));
            }
            const auto state = value.toObject();
            fa::model::State parsedState{
                requiredInt(state, QStringLiteral("id")),
                requiredString(state, QStringLiteral("name")).toStdString(),
                requiredBool(state, QStringLiteral("initial")),
                requiredBool(state, QStringLiteral("final")),
                requiredNumber(state, QStringLiteral("x")),
                requiredNumber(state, QStringLiteral("y"))};
            automaton->addState(parsedState);
        }
        for (const auto value : transitionsValue.toArray()) {
            if (!value.isObject()) {
                invalidJson(QStringLiteral("each transition must be an object."));
            }
            const auto transition = value.toObject();
            automaton->addTransition({requiredInt(transition, QStringLiteral("from")),
                                      requiredInt(transition, QStringLiteral("to")),
                                      requiredSymbol(transition)});
        }
        automaton->validate();
    } catch (const std::exception& exception) {
        throw std::runtime_error(std::string("Invalid automaton JSON: ") + exception.what());
    }
    return automaton;
}

void JsonAdapter::exportTo(const fa::model::Automaton& automaton,
                           const std::string& path) const {
    automaton.validate();
    if (automaton.typeName() != "DFA" && automaton.typeName() != "NFA") {
        throw fa::model::InvalidAutomatonException("Cannot export an unknown automaton type.");
    }

    QJsonObject root;
    root.insert(QStringLiteral("type"), QString::fromStdString(automaton.typeName()));
    QJsonArray states;
    for (const auto& state : automaton.states()) {
        QJsonObject item;
        item.insert(QStringLiteral("id"), state.id);
        item.insert(QStringLiteral("name"), QString::fromStdString(state.name));
        item.insert(QStringLiteral("initial"), state.initial);
        item.insert(QStringLiteral("final"), state.final);
        item.insert(QStringLiteral("x"), state.x);
        item.insert(QStringLiteral("y"), state.y);
        states.append(item);
    }
    root.insert(QStringLiteral("states"), states);

    QJsonArray transitions;
    for (const auto& transition : automaton.transitions()) {
        QJsonObject item;
        item.insert(QStringLiteral("from"), transition.from);
        item.insert(QStringLiteral("to"), transition.to);
        item.insert(QStringLiteral("symbol"),
                    transition.symbol == '\0'
                        ? QString()
                        : QString::fromLatin1(&transition.symbol, 1));
        transitions.append(item);
    }
    root.insert(QStringLiteral("transitions"), transitions);

    QFile file(QString::fromStdString(path));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        throw std::runtime_error("Cannot open JSON file for writing: " + path +
                                 " (" + file.errorString().toStdString() + ")");
    }
    const QByteArray data = QJsonDocument(root).toJson(QJsonDocument::Indented);
    if (file.write(data) != data.size()) {
        throw std::runtime_error("Failed to write complete JSON file: " + path +
                                 " (" + file.errorString().toStdString() + ")");
    }
}

} // namespace fa::adapters
