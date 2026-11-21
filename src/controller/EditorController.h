#pragma once

#include "AutomatonFactory.h"
#include "../model/Automaton.h"

#include <QObject>
#include <QString>

#include <memory>
#include <string>
#include <vector>

namespace fa::ctrl {

class EditorController final : public QObject {
    Q_OBJECT
public:
    explicit EditorController(QObject* parent = nullptr);

    void newAutomaton(AutomatonType type);
    bool onStateAdded(double x, double y);
    bool onStateMoved(int id, double x, double y);
    bool onTransitionAdded(int from, int to, char symbol);
    bool onItemDeleted(int id);
    bool setInitial(int id);
    bool setFinal(int id, bool value);
    bool onTransitionDeleted(int from, int to, char symbol);
    bool deleteItems(const std::vector<int>& stateIds,
                     const std::vector<fa::model::Transition>& transitions);
    bool setStateName(int id, const std::string& name);
    bool loadFromFile(const std::string& path);
    bool saveToFile(const std::string& path, bool jflap);

    const fa::model::Automaton& automaton() const;

signals:
    void automatonChanged();
    void stateMovedOnly();
    void modified();
    void errorOccurred(const QString& title, const QString& message);
    void statusMessage(const QString& message, int timeoutMs = 3000);

private:
    std::unique_ptr<fa::model::Automaton> m_automaton;

    bool reportFailure(const QString& title, const std::exception& exception);
};

} // namespace fa::ctrl
