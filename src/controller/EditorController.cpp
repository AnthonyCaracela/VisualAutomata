#include "EditorController.h"

#include "../adapters/IAutomatonAdapter.h"
#include "../adapters/JffAdapter.h"
#include "../adapters/JsonAdapter.h"
#include "../model/Exceptions.h"

#include <exception>

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

void EditorController::onStateAdded(double x, double y) {
    try {
        const int id = m_automaton->nextStateId();
        const bool initial = !m_automaton->initialStateId().has_value();
        m_automaton->addState({id, "q" + std::to_string(id), initial, false, x, y});
        emit automatonChanged();
    } catch (const std::exception& exception) {
        emit errorOccurred(QStringLiteral("No se pudo agregar el estado"),
                           QString::fromStdString(exception.what()));
    }
}

void EditorController::onStateMoved(int id, double x, double y) {
    try {
        m_automaton->moveState(id, x, y);
        emit stateMovedOnly();
    } catch (const std::exception& exception) {
        emit errorOccurred(QStringLiteral("No se pudo mover el estado"),
                           QString::fromStdString(exception.what()));
        emit automatonChanged();
    }
}

void EditorController::onTransitionAdded(int from, int to, char symbol) {
    try {
        m_automaton->addTransition({from, to, symbol});
        emit automatonChanged();
    } catch (const std::exception& exception) {
        emit errorOccurred(QStringLiteral("No se pudo agregar la transición"),
                           QString::fromStdString(exception.what()));
    }
}

void EditorController::onItemDeleted(int id) {
    try {
        m_automaton->removeState(id);
        emit automatonChanged();
    } catch (const std::exception& exception) {
        emit errorOccurred(QStringLiteral("No se pudo eliminar el estado"),
                           QString::fromStdString(exception.what()));
    }
}

void EditorController::setInitial(int id) {
    try {
        m_automaton->setInitial(id);
        emit automatonChanged();
    } catch (const std::exception& exception) {
        emit errorOccurred(QStringLiteral("No se pudo actualizar el estado"),
                           QString::fromStdString(exception.what()));
    }
}

void EditorController::setFinal(int id, bool value) {
    try {
        m_automaton->setFinal(id, value);
        emit automatonChanged();
    } catch (const std::exception& exception) {
        emit errorOccurred(QStringLiteral("No se pudo actualizar el estado"),
                           QString::fromStdString(exception.what()));
    }
}

void EditorController::onTransitionDeleted(int from, int to, char symbol) {
    try {
        m_automaton->removeTransition({from, to, symbol});
        emit automatonChanged();
    } catch (const std::exception& exception) {
        emit errorOccurred(QStringLiteral("No se pudo eliminar la transición"),
                           QString::fromStdString(exception.what()));
    }
}

void EditorController::setStateName(int id, const std::string& name) {
    try {
        m_automaton->setStateName(id, name);
        emit automatonChanged();
    } catch (const std::exception& exception) {
        emit errorOccurred(QStringLiteral("No se pudo renombrar el estado"),
                           QString::fromStdString(exception.what()));
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
        m_automaton = adapter->importFrom(path);
        emit automatonChanged();
        emit statusMessage(QStringLiteral("Autómata cargado."), 3000);
        return true;
    } catch (const std::exception& exception) {
        emit errorOccurred(QStringLiteral("No se pudo abrir el autómata"),
                           QString::fromStdString(exception.what()));
        return false;
    }
}

bool EditorController::saveToFile(const std::string& path, bool jflap) const {
    try {
        if (jflap) {
            fa::adapters::JffAdapter().exportTo(*m_automaton, path);
        } else {
            fa::adapters::JsonAdapter().exportTo(*m_automaton, path);
        }
        return true;
    } catch (const std::exception& exception) {
        emit const_cast<EditorController*>(this)->errorOccurred(
            QStringLiteral("No se pudo exportar el autómata"),
            QString::fromStdString(exception.what()));
        return false;
    }
}

const fa::model::Automaton& EditorController::automaton() const {
    return *m_automaton;
}

bool EditorController::hasAutomaton() const {
    return static_cast<bool>(m_automaton);
}

} // namespace fa::ctrl