#include "EditorController.h"

#include "../adapters/IAutomatonAdapter.h"
#include "../adapters/JffAdapter.h"
#include "../adapters/JsonAdapter.h"
#include "../model/Exceptions.h"

#include <algorithm>
#include <exception>
#include <set>
#include <utility>

namespace fa::ctrl {

EditorController::EditorController(QObject* parent)
    : QObject(parent),
      m_automaton(AutomatonFactory::create(AutomatonType::DFA)) {}

void EditorController::newAutomaton(AutomatonType type) {
    m_automaton = AutomatonFactory::create(type);
    emit automatonChanged();
    emit statusMessage(
        QStringLiteral("Nuevo autómata: ") + QString::fromStdString(m_automaton->typeName()),
        3000);
}

bool EditorController::onStateAdded(double x, double y) {
    try {
        const int id = m_automaton->nextStateId();
        const bool initial = !m_automaton->initialStateId().has_value();
        m_automaton->addState({id, "q" + std::to_string(id), initial, false, x, y});
        emit automatonChanged();
        emit modified();
        return true;
    } catch (const std::exception& exception) {
        return reportFailure(QStringLiteral("No se pudo agregar el estado"), exception);
    }
}

bool EditorController::onStateMoved(int id, double x, double y) {
    try {
        m_automaton->moveState(id, x, y);
        emit stateMovedOnly();
        emit modified();
        return true;
    } catch (const std::exception& exception) {
        emit automatonChanged();
        return reportFailure(QStringLiteral("No se pudo mover el estado"), exception);
    }
}

bool EditorController::onTransitionAdded(int from, int to, char symbol) {
    try {
        m_automaton->addTransition({from, to, symbol});
        emit automatonChanged();
        emit modified();
        return true;
    } catch (const std::exception& exception) {
        return reportFailure(QStringLiteral("No se pudo agregar la transición"), exception);
    }
}

bool EditorController::onItemDeleted(int id) {
    try {
        m_automaton->removeState(id);
        emit automatonChanged();
        emit modified();
        return true;
    } catch (const std::exception& exception) {
        return reportFailure(QStringLiteral("No se pudo eliminar el estado"), exception);
    }
}

bool EditorController::setInitial(int id) {
    try {
        m_automaton->setInitial(id);
        emit automatonChanged();
        emit modified();
        return true;
    } catch (const std::exception& exception) {
        return reportFailure(QStringLiteral("No se pudo actualizar el estado"), exception);
    }
}

bool EditorController::setFinal(int id, bool value) {
    try {
        m_automaton->setFinal(id, value);
        emit automatonChanged();
        emit modified();
        return true;
    } catch (const std::exception& exception) {
        return reportFailure(QStringLiteral("No se pudo actualizar el estado"), exception);
    }
}

bool EditorController::onTransitionDeleted(int from, int to, char symbol) {
    try {
        m_automaton->removeTransition({from, to, symbol});
        emit automatonChanged();
        emit modified();
        return true;
    } catch (const std::exception& exception) {
        return reportFailure(QStringLiteral("No se pudo eliminar la transición"), exception);
    }
}

bool EditorController::deleteItems(
    const std::vector<int>& stateIds,
    const std::vector<fa::model::Transition>& transitions) {
    std::set<int> statesToDelete(stateIds.begin(), stateIds.end());
    std::vector<fa::model::Transition> transitionsToDelete;

    if (statesToDelete.empty() && transitions.empty()) {
        return false;
    }

    for (const int id : statesToDelete) {
        const auto& states = m_automaton->states();
        if (std::none_of(states.begin(), states.end(),
                         [id](const fa::model::State& state) { return state.id == id; })) {
            return reportFailure(
                QStringLiteral("No se pudo eliminar la selección"),
                fa::model::InvalidAutomatonException(
                    "The selection contains a state that does not exist."));
        }
    }

    for (const auto& transition : transitions) {
        const bool incidentToDeletedState =
            statesToDelete.count(transition.from) != 0 ||
            statesToDelete.count(transition.to) != 0;
        if (incidentToDeletedState) {
            continue;
        }

        const auto& currentTransitions = m_automaton->transitions();
        if (std::find(currentTransitions.begin(), currentTransitions.end(), transition) ==
            currentTransitions.end()) {
            return reportFailure(
                QStringLiteral("No se pudo eliminar la selección"),
                fa::model::InvalidTransitionException(
                    "The selection contains a transition that does not exist."));
        }
        if (std::find(transitionsToDelete.begin(), transitionsToDelete.end(), transition) ==
            transitionsToDelete.end()) {
            transitionsToDelete.push_back(transition);
        }
    }

    try {
        for (const auto& transition : transitionsToDelete) {
            m_automaton->removeTransition(transition);
        }
        for (const int id : statesToDelete) {
            m_automaton->removeState(id);
        }
    } catch (const std::exception& exception) {
        return reportFailure(QStringLiteral("No se pudo eliminar la selección"), exception);
    }

    emit automatonChanged();
    emit modified();
    return true;
}

bool EditorController::setStateName(int id, const std::string& name) {
    try {
        m_automaton->setStateName(id, name);
        emit automatonChanged();
        emit modified();
        return true;
    } catch (const std::exception& exception) {
        return reportFailure(QStringLiteral("No se pudo renombrar el estado"), exception);
    }
}

bool EditorController::loadFromFile(const std::string& path) {
    try {
        const QString qPath = QString::fromStdString(path);
        const bool isJff = qPath.endsWith(QStringLiteral(".jff"), Qt::CaseInsensitive);
        std::unique_ptr<fa::adapters::IAutomatonAdapter> adapter;
        if (isJff) {
            adapter = std::make_unique<fa::adapters::JffAdapter>();
        } else {
            adapter = std::make_unique<fa::adapters::JsonAdapter>();
        }

        auto loaded = adapter->importFrom(path);
        m_automaton = std::move(loaded);
        emit automatonChanged();
        emit statusMessage(QStringLiteral("Autómata cargado."), 3000);
        return true;
    } catch (const std::exception& exception) {
        return reportFailure(QStringLiteral("No se pudo abrir el autómata"), exception);
    }
}

bool EditorController::saveToFile(const std::string& path, bool jflap) {
    try {
        if (jflap) {
            fa::adapters::JffAdapter().exportTo(*m_automaton, path);
        } else {
            fa::adapters::JsonAdapter().exportTo(*m_automaton, path);
        }
        return true;
    } catch (const std::exception& exception) {
        return reportFailure(QStringLiteral("No se pudo exportar el autómata"), exception);
    }
}

const fa::model::Automaton& EditorController::automaton() const {
    return *m_automaton;
}

bool EditorController::reportFailure(const QString& title,
                                     const std::exception& exception) {
    emit errorOccurred(title, QString::fromStdString(exception.what()));
    return false;
}

} // namespace fa::ctrl
