#include "SimulationController.h"

#include "../simulation/DFAStrategy.h"
#include "../simulation/NFAStrategy.h"
#include "../simulation/ParallelTester.h"

#include <exception>
#include <stdexcept>

namespace fa::ctrl {

SimulationController::SimulationController(const fa::model::Automaton* automaton,
                                           QObject* parent)
    : QObject(parent),
      m_automaton(automaton) {}

void SimulationController::setAutomaton(const fa::model::Automaton* automaton) {
    m_automaton = automaton;
    reset();
}

void SimulationController::setPendingInput(const std::string& input) {
    m_pendingInput = input;
}

std::unique_ptr<fa::sim::ISimulationStrategy>
SimulationController::createStrategy() const {
    if (m_automaton == nullptr) {
        throw std::logic_error("No automaton is available for simulation.");
    }
    if (m_automaton->typeName() == "DFA") {
        return std::make_unique<fa::sim::DFAStrategy>();
    }
    if (m_automaton->typeName() == "NFA") {
        return std::make_unique<fa::sim::NFAStrategy>();
    }
    throw std::logic_error("Unsupported automaton type for simulation.");
}

void SimulationController::notifyNoStates() {
    fa::sim::SimulationResult result;
    result.error = "Agrega al menos un estado al lienzo; el primero será inicial.";
    emit resultReady(result);
    emit clearHighlightRequested();
}

void SimulationController::run(const std::string& input) {
    m_pendingInput = input;
    if (m_automaton == nullptr || m_automaton->states().empty()) {
        notifyNoStates();
        return;
    }

    try {
        const auto strategy = createStrategy();
        m_lastSimulation = strategy->run(*m_automaton, input);
        m_lastInput = input;
        m_nextStep = m_lastSimulation->steps.size();

        emit resultReady(*m_lastSimulation);
        if (!m_lastSimulation->steps.empty()) {
            emit highlightStatesRequested(m_lastSimulation->steps.back().currentStates);
        } else {
            emit clearHighlightRequested();
        }
    } catch (const std::exception& exception) {
        emit errorOccurred(QStringLiteral("Falló la simulación"),
                           QString::fromStdString(exception.what()));
    }
}

void SimulationController::nextStep() {
    if (m_automaton == nullptr || m_automaton->states().empty()) {
        notifyNoStates();
        return;
    }

    if (!m_lastSimulation || m_pendingInput != m_lastInput) {
        emit resetTraceRequested();
        emit clearHighlightRequested();
        try {
            const auto strategy = createStrategy();
            m_lastSimulation = strategy->run(*m_automaton, m_pendingInput);
        } catch (const std::exception& exception) {
            m_lastSimulation.reset();
            emit errorOccurred(QStringLiteral("Falló la simulación"),
                               QString::fromStdString(exception.what()));
            return;
        }
        m_lastInput = m_pendingInput;
        m_nextStep = 0;
    }

    if (m_lastSimulation->steps.empty()) {
        emit resultReady(*m_lastSimulation);
        emit clearHighlightRequested();
        return;
    }

    if (m_nextStep < m_lastSimulation->steps.size()) {
        const auto& step = m_lastSimulation->steps[m_nextStep++];
        emit stepReady(step);
        emit highlightStatesRequested(step.currentStates);
    }

    if (m_nextStep == m_lastSimulation->steps.size()) {
        emit resultReady(*m_lastSimulation);
    }
}

void SimulationController::reset() {
    m_lastSimulation.reset();
    m_lastInput.clear();
    m_pendingInput.clear();
    m_nextStep = 0;
    emit clearHighlightRequested();
    emit resetPanelRequested();
}

void SimulationController::runAll(const std::vector<std::string>& inputs) {
    m_lastSimulation.reset();
    m_lastInput.clear();
    m_pendingInput.clear();
    m_nextStep = 0;

    QStringList qInputs;
    qInputs.reserve(static_cast<int>(inputs.size()));
    for (const auto& input : inputs) {
        qInputs.push_back(QString::fromStdString(input));
    }

    if (m_automaton == nullptr || m_automaton->states().empty()) {
        fa::sim::SimulationResult result;
        result.error = "Agrega al menos un estado al lienzo; el primero será inicial.";
        emit batchResultsReady(
            qInputs, std::vector<fa::sim::SimulationResult>(inputs.size(), result));
        emit clearHighlightRequested();
        return;
    }

    try {
        const auto strategy = createStrategy();
        const auto results =
            fa::sim::ParallelTester().testAll(*m_automaton, inputs, *strategy);
        emit batchResultsReady(qInputs, results);
        emit clearHighlightRequested();
    } catch (const std::exception& exception) {
        emit errorOccurred(QStringLiteral("Falló la simulación por lotes"),
                           QString::fromStdString(exception.what()));
    }
}

} // namespace fa::ctrl
