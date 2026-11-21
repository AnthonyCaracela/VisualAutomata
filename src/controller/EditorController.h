#pragma once

#include "AutomatonFactory.h"
#include "../model/Automaton.h"

#include <QObject>
#include <QString>

#include <memory>
#include <string>

namespace fa::ctrl {

class EditorController final : public QObject {
    Q_OBJECT
public:
    explicit EditorController(QObject* parent = nullptr);

    // Firmas exactas del acuerdo de grupo (P4)
    void newAutomaton(AutomatonType type);
    void onStateAdded(double x, double y);
    void onStateMoved(int id, double x, double y);
    void onTransitionAdded(int from, int to, char symbol);
    void onItemDeleted(int id);
    void setInitial(int id);
    void setFinal(int id, bool value);

    // Operaciones complementarias requeridas por la Vista y Adapters
    void onTransitionDeleted(int from, int to, char symbol);
    void setStateName(int id, const std::string& name);
    bool loadFromFile(const std::string& path);
    bool saveToFile(const std::string& path, bool jflap) const;

    const fa::model::Automaton& automaton() const;
    bool hasAutomaton() const;

signals:
    void automatonChanged();
    void stateMovedOnly();
    void errorOccurred(const QString& title, const QString& message);
    void statusMessage(const QString& message, int timeoutMs = 3000);

private:
    std::unique_ptr<fa::model::Automaton> m_automaton;
};

} // namespace fa::ctrl