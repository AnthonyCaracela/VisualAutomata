#pragma once

#include "../model/Automaton.h"
#include "../simulation/ISimulationStrategy.h"
#include "../simulation/SimulationResult.h"
#include "../simulation/SimulationStep.h"

#include <QObject>
#include <QString>
#include <QStringList>

#include <cstddef>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace fa::ctrl {

class SimulationController final : public QObject {
    Q_OBJECT
public:
    explicit SimulationController(const fa::model::Automaton* automaton = nullptr,
                                  QObject* parent = nullptr);

    void setAutomaton(const fa::model::Automaton* automaton);
    void setPendingInput(const std::string& input);
    void run(const std::string& input);
    void nextStep();
    void reset();
    void runAll(const std::vector<std::string>& inputs);

signals:
    void stepReady(const fa::sim::SimulationStep& step);
    void resultReady(const fa::sim::SimulationResult& result);
    void batchResultsReady(const QStringList& inputs,
                           const std::vector<fa::sim::SimulationResult>& results);
    void highlightStatesRequested(const std::set<int>& stateIds);
    void clearHighlightRequested();
    void resetPanelRequested();
    void resetTraceRequested();
    void errorOccurred(const QString& title, const QString& message);

private:
    const fa::model::Automaton* m_automaton{nullptr};
    std::optional<fa::sim::SimulationResult> m_lastSimulation;
    std::string m_lastInput;
    std::string m_pendingInput;
    std::size_t m_nextStep{0};

    std::unique_ptr<fa::sim::ISimulationStrategy> createStrategy() const;
    void notifyNoStates();
};

} // namespace fa::ctrl
