#include "MainWindow.h"

#include "AutomatonScene.h"
#include "SimulationPanel.h"
#include "StateItem.h"
#include "TransitionItem.h"

#include "../adapters/IAutomatonAdapter.h"
#include "../adapters/JffAdapter.h"
#include "../adapters/JsonAdapter.h"
#include "../controller/AutomatonFactory.h"
#include "../simulation/DFAStrategy.h"
#include "../simulation/NFAStrategy.h"
#include "../simulation/ParallelTester.h"

#include <QAction>
#include <QActionGroup>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsView>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QSplitter>
#include <QStatusBar>
#include <QToolBar>
#include <QMenuBar>

#include <algorithm>
#include <exception>
#include <string>
#include <vector>

namespace fa::view {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_scene(new AutomatonScene(this)) {
    setWindowTitle(QStringLiteral("VisualAutomata"));
    resize(1200, 760);

    auto* splitter = new QSplitter(Qt::Horizontal, this);
    m_graphicsView = new QGraphicsView(m_scene, splitter);
    m_graphicsView->setRenderHint(QPainter::Antialiasing);
    m_graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    m_graphicsView->setFocusPolicy(Qt::StrongFocus);
    m_simulationPanel = new SimulationPanel(splitter);
    splitter->addWidget(m_graphicsView);
    splitter->addWidget(m_simulationPanel);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);
    setCentralWidget(splitter);

    auto* fileMenu = menuBar()->addMenu(QStringLiteral("&Archivo"));
    auto* newDfa = fileMenu->addAction(QStringLiteral("Nuevo AFD"));
    auto* newNfa = fileMenu->addAction(QStringLiteral("Nuevo AFN"));
    fileMenu->addSeparator();
    auto* open = fileMenu->addAction(QStringLiteral("Abrir..."));
    auto* save = fileMenu->addAction(QStringLiteral("Guardar"));
    auto* saveJson = fileMenu->addAction(QStringLiteral("Exportar JSON..."));
    auto* saveJff = fileMenu->addAction(QStringLiteral("Exportar JFLAP..."));

    connect(newDfa, &QAction::triggered, this, [this]() {
        createAutomaton(fa::ctrl::AutomatonType::DFA);
    });
    connect(newNfa, &QAction::triggered, this, [this]() {
        createAutomaton(fa::ctrl::AutomatonType::NFA);
    });
    connect(open, &QAction::triggered, this, [this]() {
        const QString path = QFileDialog::getOpenFileName(
            this, QStringLiteral("Abrir autómata"), QString(),
            QStringLiteral("Automatas (*.json *.jff);;JSON (*.json);;JFLAP (*.jff)"));
        if (!path.isEmpty()) {
            loadAutomaton(path);
        }
    });
    connect(save, &QAction::triggered, this, [this]() {
        saveCurrent();
    });
    connect(saveJson, &QAction::triggered, this, [this]() {
        const QString path = QFileDialog::getSaveFileName(
            this, QStringLiteral("Exportar JSON"), QString(),
            QStringLiteral("JSON (*.json)"));
        if (!path.isEmpty()) {
            saveAutomaton(path, false);
        }
    });
    connect(saveJff, &QAction::triggered, this, [this]() {
        const QString path = QFileDialog::getSaveFileName(
            this, QStringLiteral("Exportar JFLAP"), QString(),
            QStringLiteral("JFLAP (*.jff)"));
        if (!path.isEmpty()) {
            saveAutomaton(path, true);
        }
    });

    auto* editToolbar = addToolBar(QStringLiteral("Edición"));
    editToolbar->setStyleSheet(
        QStringLiteral("QToolButton:checked { color: white; background-color: #455A64; }"));
    auto* modeGroup = new QActionGroup(this);
    modeGroup->setExclusive(true);
    const auto addModeAction = [this, editToolbar, modeGroup](
                                   const QString& text, AutomatonScene::Mode mode,
                                   bool checked = false) {
        auto* action = editToolbar->addAction(text);
        action->setCheckable(true);
        action->setChecked(checked);
        modeGroup->addAction(action);
        connect(action, &QAction::triggered, this, [this, mode]() {
            m_scene->setMode(mode);
        });
    };
    addModeAction(QStringLiteral("Seleccionar"), AutomatonScene::Mode::Select, true);
    addModeAction(QStringLiteral("Agregar estado"), AutomatonScene::Mode::AddState);
    addModeAction(QStringLiteral("Agregar transición"), AutomatonScene::Mode::AddTransition);
    editToolbar->addSeparator();
    auto* setInitial = editToolbar->addAction(QStringLiteral("Marcar inicial"));
    auto* toggleFinal = editToolbar->addAction(QStringLiteral("Alternar final"));
    auto* renameState = editToolbar->addAction(QStringLiteral("Renombrar estado"));
    auto* deleteState = editToolbar->addAction(QStringLiteral("Eliminar estado"));
    auto* deleteTransition = editToolbar->addAction(QStringLiteral("Eliminar transición"));

    connect(setInitial, &QAction::triggered, this, [this]() {
        const auto id = selectedStateId();
        if (!id) {
            statusBar()->showMessage(QStringLiteral("Selecciona un estado primero."), 3000);
            return;
        }
        try {
            m_automaton->setInitial(*id);
            markDirty();
            refreshScene();
        } catch (const std::exception& exception) {
            QMessageBox::critical(this, QStringLiteral("No se pudo actualizar el estado"),
                                  QString::fromStdString(exception.what()));
        }
    });
    connect(toggleFinal, &QAction::triggered, this, [this]() {
        const auto id = selectedStateId();
        if (!id) {
            statusBar()->showMessage(QStringLiteral("Selecciona un estado primero."), 3000);
            return;
        }
        const auto state = std::find_if(
            m_automaton->states().begin(), m_automaton->states().end(),
            [id](const fa::model::State& candidate) { return candidate.id == *id; });
        if (state == m_automaton->states().end()) {
            return;
        }
        try {
            m_automaton->setFinal(*id, !state->final);
            markDirty();
            refreshScene();
        } catch (const std::exception& exception) {
            QMessageBox::critical(this, QStringLiteral("No se pudo actualizar el estado"),
                                  QString::fromStdString(exception.what()));
        }
    });
    connect(renameState, &QAction::triggered, this, [this]() {
        const auto id = selectedStateId();
        if (!id) {
            statusBar()->showMessage(QStringLiteral("Selecciona un estado primero."), 3000);
            return;
        }
        const auto state = std::find_if(
            m_automaton->states().begin(), m_automaton->states().end(),
            [id](const fa::model::State& candidate) { return candidate.id == *id; });
        if (state == m_automaton->states().end()) {
            return;
        }
        bool accepted = false;
        const QString name = QInputDialog::getText(
            this, QStringLiteral("Renombrar estado"), QStringLiteral("Nombre:"),
            QLineEdit::Normal, QString::fromStdString(state->name), &accepted);
        if (!accepted) {
            return;
        }
        try {
            m_automaton->setStateName(*id, name.toStdString());
            markDirty();
            refreshScene();
        } catch (const std::exception& exception) {
            QMessageBox::critical(this, QStringLiteral("No se pudo renombrar el estado"),
                                  QString::fromStdString(exception.what()));
        }
    });
    connect(deleteState, &QAction::triggered, this, [this]() {
        const auto id = selectedStateId();
        if (!id) {
            statusBar()->showMessage(QStringLiteral("Selecciona un estado primero."), 3000);
            return;
        }
        try {
            m_automaton->removeState(*id);
            markDirty();
            refreshScene();
        } catch (const std::exception& exception) {
            QMessageBox::critical(this, QStringLiteral("No se pudo eliminar el estado"),
                                  QString::fromStdString(exception.what()));
        }
    });
    connect(deleteTransition, &QAction::triggered, this, [this]() {
        const auto transition = selectedTransition();
        if (!transition) {
            statusBar()->showMessage(QStringLiteral("Selecciona una transición primero."), 3000);
            return;
        }
        try {
            m_automaton->removeTransition(*transition);
            markDirty();
            refreshScene();
        } catch (const std::exception& exception) {
            QMessageBox::critical(this, QStringLiteral("No se pudo eliminar la transición"),
                                  QString::fromStdString(exception.what()));
        }
    });

    connect(m_scene, &AutomatonScene::stateAdded, this, [this](double x, double y) {
        try {
            const int id = m_automaton->nextStateId();
            const bool initial = !m_automaton->initialStateId().has_value();
            m_automaton->addState(
                {id, "q" + std::to_string(id), initial, false, x, y});
            markDirty();
            refreshScene();
        } catch (const std::exception& exception) {
            QMessageBox::critical(this, QStringLiteral("No se pudo agregar el estado"),
                                  QString::fromStdString(exception.what()));
        }
    });
    connect(m_scene, &AutomatonScene::stateMoved, this, [this](int id, double x, double y) {
        try {
            m_automaton->moveState(id, x, y);
            markDirty();
            resetSimulation();
        } catch (const std::exception& exception) {
            QMessageBox::critical(this, QStringLiteral("No se pudo mover el estado"),
                                  QString::fromStdString(exception.what()));
            refreshScene();
        }
    });
    connect(m_scene, &AutomatonScene::transitionAdded, this,
            [this](int from, int to, char symbol) {
                try {
                    m_automaton->addTransition({from, to, symbol});
                    markDirty();
                    refreshScene();
                } catch (const std::exception& exception) {
                    QMessageBox::critical(this, QStringLiteral("No se pudo agregar la transición"),
                                          QString::fromStdString(exception.what()));
                }
            });
    connect(m_scene, &AutomatonScene::itemDeleted, this, [this](int id) {
        try {
            m_automaton->removeState(id);
            markDirty();
            refreshScene();
        } catch (const std::exception& exception) {
            QMessageBox::critical(this, QStringLiteral("No se pudo eliminar el estado"),
                                  QString::fromStdString(exception.what()));
        }
    });
    connect(m_scene, &AutomatonScene::transitionDeleted, this,
            [this](int from, int to, char symbol) {
                try {
                    m_automaton->removeTransition({from, to, symbol});
                    markDirty();
                    refreshScene();
                } catch (const std::exception& exception) {
                    QMessageBox::critical(this, QStringLiteral("No se pudo eliminar la transición"),
                                          QString::fromStdString(exception.what()));
                }
            });

    connect(m_simulationPanel, &SimulationPanel::runRequested, this,
            [this](const QString& input) { runInput(input); });
    connect(m_simulationPanel, &SimulationPanel::stepRequested, this,
            [this](const QString& input) { stepInput(input); });
    connect(m_simulationPanel, &SimulationPanel::runAllRequested, this,
            [this](const QStringList& inputs) { runBatch(inputs); });

    createAutomaton(fa::ctrl::AutomatonType::DFA);
}

void MainWindow::createAutomaton(fa::ctrl::AutomatonType type) {
    if (!confirmDiscardChanges()) {
        return;
    }
    m_automaton = fa::ctrl::AutomatonFactory::create(type);
    m_filePath.clear();
    m_fileIsJff = false;
    markClean();
    refreshScene();
    statusBar()->showMessage(
        QStringLiteral("Nuevo autómata: ") + QString::fromStdString(m_automaton->typeName()));
}

void MainWindow::loadAutomaton(const QString& path) {
    if (!confirmDiscardChanges()) {
        return;
    }
    try {
        std::unique_ptr<fa::adapters::IAutomatonAdapter> adapter;
        const bool jflap = path.endsWith(QStringLiteral(".jff"), Qt::CaseInsensitive);
        if (jflap) {
            adapter = std::make_unique<fa::adapters::JffAdapter>();
        } else {
            adapter = std::make_unique<fa::adapters::JsonAdapter>();
        }
        m_automaton = adapter->importFrom(path.toStdString());
        m_filePath = path;
        m_fileIsJff = jflap;
        markClean();
        refreshScene();
        statusBar()->showMessage(QStringLiteral("Autómata cargado."), 3000);
    } catch (const std::exception& exception) {
        QMessageBox::critical(this, QStringLiteral("No se pudo abrir el autómata"),
                              QString::fromStdString(exception.what()));
    }
}

bool MainWindow::saveAutomaton(const QString& path, bool jflap) {
    try {
        QString outputPath = path;
        const QString extension = jflap ? QStringLiteral(".jff") : QStringLiteral(".json");
        if (!outputPath.endsWith(extension, Qt::CaseInsensitive)) {
            outputPath += extension;
        }
        if (jflap) {
            fa::adapters::JffAdapter().exportTo(*m_automaton, outputPath.toStdString());
        } else {
            fa::adapters::JsonAdapter().exportTo(*m_automaton, outputPath.toStdString());
        }
        m_filePath = outputPath;
        m_fileIsJff = jflap;
        markClean();
        statusBar()->showMessage(QStringLiteral("Autómata guardado."), 3000);
        return true;
    } catch (const std::exception& exception) {
        QMessageBox::critical(this, QStringLiteral("No se pudo exportar el autómata"),
                              QString::fromStdString(exception.what()));
        return false;
    }
}

bool MainWindow::saveCurrent() {
    if (!m_filePath.isEmpty()) {
        return saveAutomaton(m_filePath, m_fileIsJff);
    }
    const QString filter = QStringLiteral("JSON (*.json);;JFLAP (*.jff)");
    QString selectedFilter = filter;
    const QString path = QFileDialog::getSaveFileName(
        this, QStringLiteral("Guardar autómata"), QString(), filter, &selectedFilter);
    if (path.isEmpty()) {
        return false;
    }
    return saveAutomaton(path, selectedFilter.contains(QStringLiteral("JFLAP")));
}

bool MainWindow::confirmDiscardChanges() {
    if (!m_dirty) {
        return true;
    }
    const auto choice = QMessageBox::warning(
        this, QStringLiteral("Cambios sin guardar"),
        QStringLiteral("Hay cambios sin guardar. ¿Quieres guardarlos antes de continuar?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save);
    if (choice == QMessageBox::Save) {
        return saveCurrent();
    }
    return choice == QMessageBox::Discard;
}

void MainWindow::markDirty() {
    m_dirty = true;
    updateWindowTitle();
}

void MainWindow::markClean() {
    m_dirty = false;
    updateWindowTitle();
}

void MainWindow::updateWindowTitle() {
    QString title = QStringLiteral("VisualAutomata");
    if (!m_filePath.isEmpty()) {
        title += QStringLiteral(" - ") + QFileInfo(m_filePath).fileName();
    }
    if (m_dirty) {
        title += QStringLiteral(" *");
    }
    setWindowTitle(title);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (confirmDiscardChanges()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::refreshScene() {
    m_scene->loadAutomaton(*m_automaton);
    resetSimulation();
}

void MainWindow::resetSimulation() {
    m_lastSimulation.reset();
    m_lastInput.clear();
    m_nextStep = 0;
    if (m_scene) {
        m_scene->clearHighlight();
    }
    if (m_simulationPanel) {
        m_simulationPanel->reset();
    }
}

void MainWindow::showNoStatesError() {
    fa::sim::SimulationResult result;
    result.error = "Agrega al menos un estado al lienzo; el primero será inicial.";
    m_simulationPanel->showResult(result);
    m_scene->clearHighlight();
}

void MainWindow::runInput(const QString& input) {
    if (m_automaton->states().empty()) {
        showNoStatesError();
        return;
    }
    const std::string text = input.toUtf8().toStdString();
    m_lastSimulation = m_automaton->typeName() == "DFA"
        ? fa::sim::DFAStrategy().run(*m_automaton, text)
        : fa::sim::NFAStrategy().run(*m_automaton, text);
    m_lastInput = input;
    m_nextStep = m_lastSimulation->steps.size();
    m_simulationPanel->showResult(*m_lastSimulation);
    if (!m_lastSimulation->steps.empty()) {
        m_scene->highlightStates(m_lastSimulation->steps.back().currentStates);
    } else {
        m_scene->clearHighlight();
    }
}

void MainWindow::stepInput(const QString& input) {
    if (m_automaton->states().empty()) {
        showNoStatesError();
        return;
    }
    if (!m_lastSimulation || input != m_lastInput) {
        m_simulationPanel->reset();
        m_scene->clearHighlight();
        const std::string text = input.toUtf8().toStdString();
        m_lastSimulation = m_automaton->typeName() == "DFA"
            ? fa::sim::DFAStrategy().run(*m_automaton, text)
            : fa::sim::NFAStrategy().run(*m_automaton, text);
        m_lastInput = input;
        m_nextStep = 0;
    }

    if (m_lastSimulation->steps.empty()) {
        m_simulationPanel->showResult(*m_lastSimulation);
        m_scene->clearHighlight();
        return;
    }
    if (m_nextStep < m_lastSimulation->steps.size()) {
        const auto& step = m_lastSimulation->steps[m_nextStep++];
        m_simulationPanel->showStep(step);
        m_scene->highlightStates(step.currentStates);
    }
    if (m_nextStep == m_lastSimulation->steps.size()) {
        m_simulationPanel->showResult(*m_lastSimulation);
    }
}

void MainWindow::runBatch(const QStringList& inputs) {
    m_lastSimulation.reset();
    m_lastInput.clear();
    m_nextStep = 0;
    std::vector<std::string> strings;
    strings.reserve(static_cast<std::size_t>(inputs.size()));
    for (const auto& input : inputs) {
        strings.push_back(input.toUtf8().toStdString());
    }
    if (m_automaton->states().empty()) {
        fa::sim::SimulationResult result;
        result.error = "Agrega al menos un estado al lienzo; el primero será inicial.";
        m_simulationPanel->showBatchResults(inputs,
                                            std::vector<fa::sim::SimulationResult>(
                                                inputs.size(), result));
        return;
    }
    try {
        std::vector<fa::sim::SimulationResult> results;
        if (m_automaton->typeName() == "DFA") {
            results = fa::sim::ParallelTester().testAll(
                *m_automaton, strings, fa::sim::DFAStrategy());
        } else {
            results = fa::sim::ParallelTester().testAll(
                *m_automaton, strings, fa::sim::NFAStrategy());
        }
        m_simulationPanel->showBatchResults(inputs, results);
        m_scene->clearHighlight();
    } catch (const std::exception& exception) {
        QMessageBox::critical(this, QStringLiteral("Falló la simulación por lotes"),
                              QString::fromStdString(exception.what()));
    }
}

std::optional<int> MainWindow::selectedStateId() const {
    for (QGraphicsItem* item : m_scene->selectedItems()) {
        if (const auto* state = dynamic_cast<const StateItem*>(item)) {
            return state->id();
        }
    }
    return std::nullopt;
}

std::optional<fa::model::Transition> MainWindow::selectedTransition() const {
    for (QGraphicsItem* item : m_scene->selectedItems()) {
        if (const auto* transition = dynamic_cast<const TransitionItem*>(item)) {
            return fa::model::Transition{transition->fromId(), transition->toId(),
                                         transition->symbol()};
        }
    }
    return std::nullopt;
}

} // namespace fa::view
