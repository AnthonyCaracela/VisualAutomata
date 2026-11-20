#include "JffAdapter.h"

#include "../controller/AutomatonFactory.h"
#include "../model/Exceptions.h"

#include <QFile>
#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

namespace fa::adapters {
namespace {

struct ParsedTransition {
    int from{};
    int to{};
    char symbol{};
};

[[noreturn]] void invalidJff(const QString& message) {
    throw std::runtime_error((QStringLiteral("Invalid JFLAP file: ") + message).toStdString());
}

int parseInt(const QString& value, const QString& field) {
    bool ok = false;
    const int parsed = value.toInt(&ok);
    if (!ok) {
        invalidJff(QStringLiteral("'") + field + QStringLiteral("' must be an integer."));
    }
    return parsed;
}

double parseDouble(const QString& value, const QString& field) {
    bool ok = false;
    const double parsed = value.toDouble(&ok);
    if (!ok || !std::isfinite(parsed)) {
        invalidJff(QStringLiteral("'") + field + QStringLiteral("' must be a finite number."));
    }
    return parsed;
}

char parseSymbol(const QString& value) {
    if (value.isEmpty()) {
        return '\0';
    }
    const QByteArray utf8 = value.toUtf8();
    if (utf8.size() != 1) {
        invalidJff(QStringLiteral("transition symbols must be empty or one single-byte character."));
    }
    return utf8.at(0);
}

} // namespace

std::unique_ptr<fa::model::Automaton> JffAdapter::importFrom(const std::string& path) const {
    QFile file(QString::fromStdString(path));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error("Cannot open JFLAP file for reading: " + path +
                                 " (" + file.errorString().toStdString() + ")");
    }

    QXmlStreamReader xml(&file);
    std::vector<fa::model::State> states;
    std::vector<ParsedTransition> transitions;
    bool sawStructure = false;
    bool sawAutomaton = false;
    bool sawType = false;

    while (!xml.atEnd()) {
        xml.readNext();
        if (!xml.isStartElement()) {
            continue;
        }
        if (xml.name() == QStringLiteral("structure")) {
            if (sawStructure) invalidJff(QStringLiteral("duplicate <structure> element."));
            sawStructure = true;
        } else if (xml.name() == QStringLiteral("type")) {
            if (sawType) invalidJff(QStringLiteral("duplicate <type> element."));
            sawType = true;
            if (xml.readElementText().trimmed() != QStringLiteral("fa")) {
                invalidJff(QStringLiteral("only finite automata (<type>fa</type>) are supported."));
            }
        } else if (xml.name() == QStringLiteral("automaton")) {
            if (sawAutomaton) invalidJff(QStringLiteral("duplicate <automaton> element."));
            sawAutomaton = true;
        } else if (xml.name() == QStringLiteral("state")) {
            if (!sawAutomaton) invalidJff(QStringLiteral("<state> must be inside <automaton>."));
            const auto attributes = xml.attributes();
            if (!attributes.hasAttribute(QStringLiteral("id")) ||
                !attributes.hasAttribute(QStringLiteral("name"))) {
                invalidJff(QStringLiteral("each <state> requires id and name attributes."));
            }
            fa::model::State state{};
            state.id = parseInt(attributes.value(QStringLiteral("id")).toString(),
                                QStringLiteral("state id"));
            state.name = attributes.value(QStringLiteral("name")).toString().toStdString();
            bool hasX = false;
            bool hasY = false;
            while (xml.readNextStartElement()) {
                const QString element = xml.name().toString();
                if (element == QStringLiteral("x")) {
                    if (hasX) invalidJff(QStringLiteral("duplicate state <x> element."));
                    state.x = parseDouble(xml.readElementText().trimmed(), QStringLiteral("x"));
                    hasX = true;
                } else if (element == QStringLiteral("y")) {
                    if (hasY) invalidJff(QStringLiteral("duplicate state <y> element."));
                    state.y = parseDouble(xml.readElementText().trimmed(), QStringLiteral("y"));
                    hasY = true;
                } else if (element == QStringLiteral("initial")) {
                    if (state.initial) invalidJff(QStringLiteral("duplicate <initial> marker."));
                    state.initial = true;
                    xml.skipCurrentElement();
                } else if (element == QStringLiteral("final")) {
                    if (state.final) invalidJff(QStringLiteral("duplicate <final> marker."));
                    state.final = true;
                    xml.skipCurrentElement();
                } else {
                    xml.skipCurrentElement();
                }
            }
            if (!hasX || !hasY) {
                invalidJff(QStringLiteral("each state requires finite <x> and <y> coordinates."));
            }
            states.push_back(std::move(state));
        } else if (xml.name() == QStringLiteral("transition")) {
            if (!sawAutomaton) invalidJff(QStringLiteral("<transition> must be inside <automaton>."));
            ParsedTransition transition{};
            bool hasFrom = false;
            bool hasTo = false;
            bool hasRead = false;
            while (xml.readNextStartElement()) {
                const QString element = xml.name().toString();
                if (element == QStringLiteral("from")) {
                    if (hasFrom) invalidJff(QStringLiteral("duplicate transition <from> element."));
                    transition.from = parseInt(xml.readElementText().trimmed(),
                                               QStringLiteral("from"));
                    hasFrom = true;
                } else if (element == QStringLiteral("to")) {
                    if (hasTo) invalidJff(QStringLiteral("duplicate transition <to> element."));
                    transition.to = parseInt(xml.readElementText().trimmed(),
                                             QStringLiteral("to"));
                    hasTo = true;
                } else if (element == QStringLiteral("read")) {
                    if (hasRead) invalidJff(QStringLiteral("duplicate transition <read> element."));
                    transition.symbol = parseSymbol(xml.readElementText());
                    hasRead = true;
                } else {
                    xml.skipCurrentElement();
                }
            }
            if (!hasFrom || !hasTo || !hasRead) {
                invalidJff(QStringLiteral("each transition requires <from>, <to>, and <read>."));
            }
            transitions.push_back(transition);
        }
    }
    if (xml.hasError()) {
        invalidJff(xml.errorString());
    }
    if (!sawStructure || !sawType || !sawAutomaton) {
        invalidJff(QStringLiteral("expected <structure>, <type>fa</type>, and <automaton>."));
    }

    bool isNfa = false;
    for (std::size_t i = 0; i < transitions.size(); ++i) {
        isNfa = isNfa || transitions[i].symbol == '\0';
        for (std::size_t j = i + 1; j < transitions.size(); ++j) {
            if (transitions[i].from == transitions[j].from &&
                transitions[i].symbol == transitions[j].symbol) {
                isNfa = true;
            }
        }
    }

    auto automaton = fa::ctrl::AutomatonFactory::create(
        isNfa ? fa::ctrl::AutomatonType::NFA : fa::ctrl::AutomatonType::DFA);
    try {
        for (const auto& state : states) {
            automaton->addState(state);
        }
        for (const auto& transition : transitions) {
            automaton->addTransition({transition.from, transition.to, transition.symbol});
        }
        automaton->validate();
    } catch (const std::exception& exception) {
        throw std::runtime_error(std::string("Invalid JFLAP automaton: ") + exception.what());
    }
    return automaton;
}

void JffAdapter::exportTo(const fa::model::Automaton& automaton,
                          const std::string& path) const {
    automaton.validate();
    if (automaton.typeName() != "DFA" && automaton.typeName() != "NFA") {
        throw fa::model::InvalidAutomatonException("Cannot export an unknown automaton type.");
    }

    QFile file(QString::fromStdString(path));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        throw std::runtime_error("Cannot open JFLAP file for writing: " + path +
                                 " (" + file.errorString().toStdString() + ")");
    }
    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument(QStringLiteral("1.0"));
    xml.writeStartElement(QStringLiteral("structure"));
    xml.writeTextElement(QStringLiteral("type"), QStringLiteral("fa"));
    xml.writeStartElement(QStringLiteral("automaton"));
    for (const auto& state : automaton.states()) {
        xml.writeStartElement(QStringLiteral("state"));
        xml.writeAttribute(QStringLiteral("id"), QString::number(state.id));
        xml.writeAttribute(QStringLiteral("name"), QString::fromStdString(state.name));
        xml.writeTextElement(QStringLiteral("x"), QString::number(state.x, 'g', 17));
        xml.writeTextElement(QStringLiteral("y"), QString::number(state.y, 'g', 17));
        if (state.initial) xml.writeEmptyElement(QStringLiteral("initial"));
        if (state.final) xml.writeEmptyElement(QStringLiteral("final"));
        xml.writeEndElement();
    }
    for (const auto& transition : automaton.transitions()) {
        xml.writeStartElement(QStringLiteral("transition"));
        xml.writeTextElement(QStringLiteral("from"), QString::number(transition.from));
        xml.writeTextElement(QStringLiteral("to"), QString::number(transition.to));
        if (transition.symbol == '\0') {
            xml.writeEmptyElement(QStringLiteral("read"));
        } else {
            xml.writeTextElement(QStringLiteral("read"),
                                 QString::fromLatin1(&transition.symbol, 1));
        }
        xml.writeEndElement();
    }
    xml.writeEndElement();
    xml.writeEndElement();
    xml.writeEndDocument();
    if (!file.flush() || file.error() != QFile::NoError) {
        throw std::runtime_error("Failed to write complete JFLAP file: " + path);
    }
}

} // namespace fa::adapters
