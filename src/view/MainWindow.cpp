#include "MainWindow.h"

#include "AutomatonScene.h"
#include "SimulationPanel.h"
#include "StateItem.h"
#include "TransitionItem.h"

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
#include <QToolButton>
#include <QToolBar>
#include <QMenuBar>

#include <algorithm>
#include <set>
#include <string>
#include <vector>

namespace fa::view {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_editorController(new fa::ctrl::EditorController(this)),
      m_simulationController(new fa::ctrl::SimulationController(nullptr, this)),
      m_scene(new AutomatonScene(this)) {
    setWindowTitle(QStringLiteral("VisualAutomata"));
    resize(1200, 760);

    auto* splitter = new QSplitter(Qt::Horizontal, this);
    m_graphicsView = new QGraphicsView(m_scene, splitter);
    m_graphicsView->setRenderHint(QPainter::Antialiasing);
    m_graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    m_graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    m_graphicsView->setFocusPolicy(Qt::StrongFocus);
    m_simulationPanel = new SimulationPanel(splitter);
    splitter->addWidget(m_graphicsView);
    splitter->addWidget(m_simulationPanel);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);
    setCentralWidget(splitter);

    connect(m_editorController, &fa::ctrl::EditorController::automatonChanged,
            this, &MainWindow::refreshScene);
    connect(m_editorController, &fa::ctrl::EditorController::stateMovedOnly,
            this, &MainWindow::resetSimulation);
    connect(m_editorController, &fa::ctrl::EditorController::modified,
            this, &MainWindow::markDirty);
    connect(m_editorController, &fa::ctrl::EditorController::errorOccurred, this,
            [this](const QString& title, const QString& message) {
                QMessageBox::critical(this, title, message);
            });
    connect(m_editorController, &fa::ctrl::EditorController::statusMessage, this,
            [this](const QString& message, int timeoutMs) {
                statusBar()->showMessage(message, timeoutMs);
            });

    connect(m_simulationController, &fa::ctrl::SimulationController::stepReady,
            m_simulationPanel, &SimulationPanel::showStep);
    connect(m_simulationController, &fa::ctrl::SimulationController::resultReady,
            m_simulationPanel, &SimulationPanel::showResult);
    connect(m_simulationController, &fa::ctrl::SimulationController::batchResultsReady,
            m_simulationPanel, &SimulationPanel::showBatchResults);
    connect(m_simulationController,
            &fa::ctrl::SimulationController::highlightStatesRequested,
            m_scene, &AutomatonScene::highlightStates);
    connect(m_simulationController,
            &fa::ctrl::SimulationController::clearHighlightRequested,
            m_scene, &AutomatonScene::clearHighlight);
    connect(m_simulationController,
            &fa::ctrl::SimulationController::resetPanelRequested,
            m_simulationPanel, &SimulationPanel::reset);
    connect(m_simulationController,
            &fa::ctrl::SimulationController::resetTraceRequested,
            m_simulationPanel, &SimulationPanel::resetTrace);
    connect(m_simulationController, &fa::ctrl::SimulationController::errorOccurred, this,
            [this](const QString& title, const QString& message) {
                QMessageBox::critical(this, title, message);
            });

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
    auto* modeGroup = new QActionGroup(this);
    modeGroup->setExclusive(true);
    const auto addModeAction = [this, editToolbar, modeGroup](
                                   const QString& text, AutomatonScene::Mode mode,
                                   bool checked = false) -> QAction* {
        auto* action = editToolbar->addAction(text);
        action->setCheckable(true);
        action->setChecked(checked);
        modeGroup->addAction(action);
        connect(action, &QAction::triggered, this, [this, mode]() {
            m_scene->setMode(mode);
        });
        return action;
    };
    const auto styleAction = [editToolbar](
                                 QAction* action, const QString& background,
                                 const QString& foreground, const QString& border,
                                 const QString& pressed) {
        if (auto* button = qobject_cast<QToolButton*>(editToolbar->widgetForAction(action))) {
            button->setMinimumHeight(28);
            button->setStyleSheet(
                QStringLiteral(
                    "QToolButton { background:%1; color:%2; border:1px solid %3;"
                    " border-radius:4px; padding:4px 7px; font-weight:600; }"
                    "QToolButton:hover { border:1px solid #FFFFFF; }"
                    "QToolButton:pressed, QToolButton:checked { background:%4; color:%2; }")
                    .arg(background, foreground, border, pressed));
        }
    };
    auto* selectMode =
        addModeAction(QStringLiteral("Seleccionar"), AutomatonScene::Mode::Select, true);
    auto* addStateMode =
        addModeAction(QStringLiteral("Agregar estado"), AutomatonScene::Mode::AddState);
    auto* addTransitionMode =
        addModeAction(QStringLiteral("Agregar transición"), AutomatonScene::Mode::AddTransition);
    styleAction(selectMode, QStringLiteral("#455A64"),
                QStringLiteral("#FFFFFF"), QStringLiteral("#263238"),
                QStringLiteral("#263238"));
    styleAction(addStateMode, QStringLiteral("#1565C0"),
                QStringLiteral("#FFFFFF"), QStringLiteral("#0D47A1"),
                QStringLiteral("#0D47A1"));
    styleAction(addTransitionMode, QStringLiteral("#1565C0"),
                QStringLiteral("#FFFFFF"), QStringLiteral("#0D47A1"),
                QStringLiteral("#0D47A1"));
    editToolbar->addSeparator();
    auto* setInitial = editToolbar->addAction(QStringLiteral("Marcar inicial"));
    auto* toggleFinal = editToolbar->addAction(QStringLiteral("Alternar final"));
    auto* renameState = editToolbar->addAction(QStringLiteral("Renombrar estado"));
    auto* deleteState = editToolbar->addAction(QStringLiteral("Eliminar estado"));
    auto* deleteTransition = editToolbar->addAction(QStringLiteral("Eliminar transición"));
    auto* deleteSelection = editToolbar->addAction(QStringLiteral("Eliminar selección"));
    for (QAction* action : {setInitial, toggleFinal, renameState}) {
        styleAction(action, QStringLiteral("#F9A825"), QStringLiteral("#212121"),
                    QStringLiteral("#F57F17"), QStringLiteral("#F57F17"));
    }
    for (QAction* action : {deleteState, deleteTransition, deleteSelection}) {
        styleAction(action, QStringLiteral("#C62828"), QStringLiteral("#FFFFFF"),
                    QStringLiteral("#B71C1C"), QStringLiteral("#B71C1C"));
    }

    connect(setInitial, &QAction::triggered, this, [this]() {
        const auto id = selectedStateId();
        if (!id) {
            statusBar()->showMessage(QStringLiteral("Selecciona un estado primero."), 3000);
            return;
        }
        m_editorController->setInitial(*id);
    });
    connect(toggleFinal, &QAction::triggered, this, [this]() {
        const auto id = selectedStateId();
        if (!id) {
            statusBar()->showMessage(QStringLiteral("Selecciona un estado primero."), 3000);
            return;
        }
        const auto& states = m_editorController->automaton().states();
        const auto state = std::find_if(
            states.begin(), states.end(),
            [id](const fa::model::State& candidate) { return candidate.id == *id; });
        if (state == states.end()) {
            return;
        }
        m_editorController->setFinal(*id, !state->final);
    });
    connect(renameState, &QAction::triggered, this, [this]() {
        const auto id = selectedStateId();
        if (!id) {
            statusBar()->showMessage(QStringLiteral("Selecciona un estado primero."), 3000);
            return;
        }
        const auto& states = m_editorController->automaton().states();
        const auto state = std::find_if(
            states.begin(), states.end(),
            [id](const fa::model::State& candidate) { return candidate.id == *id; });
        if (state == states.end()) {
            return;
        }
        bool accepted = false;
        const QString name = QInputDialog::getText(
            this, QStringLiteral("Renombrar estado"), QStringLiteral("Nombre:"),
            QLineEdit::Normal, QString::fromStdString(state->name), &accepted);
        if (!accepted) {
            return;
        }
        m_editorController->setStateName(*id, name.toStdString());
    });
    connect(deleteState, &QAction::triggered, this, [this]() {
        const auto id = selectedStateId();
        if (!id) {
            statusBar()->showMessage(QStringLiteral("Selecciona un estado primero."), 3000);
            return;
        }
        m_editorController->onItemDeleted(*id);
    });
    connect(deleteTransition, &QAction::triggered, this, [this]() {
        const auto transition = selectedTransition();
        if (!transition) {
            statusBar()->showMessage(QStringLiteral("Selecciona una transición primero."), 3000);
            return;
        }
        m_editorController->onTransitionDeleted(transition->from, transition->to,
                                                transition->symbol);
    });
    connect(deleteSelection, &QAction::triggered,
            this, &MainWindow::deleteSelectedItems);

    connect(m_scene, &AutomatonScene::stateAdded, this,
            [this](double x, double y) { m_editorController->onStateAdded(x, y); });
    connect(m_scene, &AutomatonScene::stateMoved, this, [this](int id, double x, double y) {
        m_editorController->onStateMoved(id, x, y);
    });
    connect(m_scene, &AutomatonScene::transitionAdded, this,
            [this](int from, int to, char symbol) {
                m_editorController->onTransitionAdded(from, to, symbol);
            });
    connect(m_scene, &AutomatonScene::deleteSelectionRequested,
            this, &MainWindow::deleteSelectedItems);

    connect(m_simulationPanel, &SimulationPanel::runRequested, this,
            [this](const QString& input) {
                m_simulationController->run(input.toUtf8().toStdString());
            });
    connect(m_simulationPanel, &SimulationPanel::stepRequested, this,
            [this](const QString& input) {
                m_simulationController->setPendingInput(input.toUtf8().toStdString());
                m_simulationController->nextStep();
            });
    connect(m_simulationPanel, &SimulationPanel::runAllRequested, this,
            [this](const QStringList& inputs) {
                std::vector<std::string> strings;
                strings.reserve(static_cast<std::size_t>(inputs.size()));
                for (const auto& input : inputs) {
                    strings.push_back(input.toUtf8().toStdString());
                }
                m_simulationController->runAll(strings);
            });

    createAutomaton(fa::ctrl::AutomatonType::DFA);
}

void MainWindow::createAutomaton(fa::ctrl::AutomatonType type) {
    if (!confirmDiscardChanges()) {
        return;
    }
    m_editorController->newAutomaton(type);
    m_filePath.clear();
    m_fileIsJff = false;
    markClean();
}

void MainWindow::loadAutomaton(const QString& path) {
    if (!confirmDiscardChanges()) {
        return;
    }
    if (m_editorController->loadFromFile(path.toStdString())) {
        m_filePath = path;
        m_fileIsJff = path.endsWith(QStringLiteral(".jff"), Qt::CaseInsensitive);
        markClean();
    }
}

bool MainWindow::saveAutomaton(const QString& path, bool jflap) {
    QString outputPath = path;
    const QString extension = jflap ? QStringLiteral(".jff") : QStringLiteral(".json");
    if (!outputPath.endsWith(extension, Qt::CaseInsensitive)) {
        outputPath += extension;
    }
    if (!m_editorController->saveToFile(outputPath.toStdString(), jflap)) {
        return false;
    }
    m_filePath = outputPath;
    m_fileIsJff = jflap;
    markClean();
    statusBar()->showMessage(QStringLiteral("Autómata guardado."), 3000);
    return true;
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
    const auto& automaton = m_editorController->automaton();
    m_simulationController->setAutomaton(&automaton);
    m_scene->loadAutomaton(automaton);
}

void MainWindow::resetSimulation() {
    m_simulationController->reset();
}

void MainWindow::deleteSelectedItems() {
    std::vector<int> stateIds;
    std::vector<fa::model::Transition> transitions;
    for (QGraphicsItem* item : m_scene->selectedItems()) {
        if (const auto* state = dynamic_cast<const StateItem*>(item)) {
            stateIds.push_back(state->id());
        } else if (const auto* transition = dynamic_cast<const TransitionItem*>(item)) {
            transitions.push_back(
                {transition->fromId(), transition->toId(), transition->symbol()});
        }
    }

    if (stateIds.empty() && transitions.empty()) {
        statusBar()->showMessage(QStringLiteral("Selecciona uno o más elementos para eliminar."),
                                 3000);
        return;
    }

    const std::set<int> statesToDelete(stateIds.begin(), stateIds.end());
    std::size_t affectedTransitions = 0;
    for (const auto& transition : m_editorController->automaton().transitions()) {
        const bool selected =
            std::find(transitions.begin(), transitions.end(), transition) != transitions.end();
        const bool removedWithState =
            statesToDelete.count(transition.from) != 0 ||
            statesToDelete.count(transition.to) != 0;
        if (selected || removedWithState) {
            ++affectedTransitions;
        }
    }

    const std::size_t affectedItems = stateIds.size() + affectedTransitions;
    if (affectedItems > 1) {
        const auto choice = QMessageBox::warning(
            this, QStringLiteral("Eliminar selección"),
            QStringLiteral("Se eliminarán %1 estado(s) y %2 transición(es), incluidas las "
                           "conectadas a los estados seleccionados. ¿Continuar?")
                .arg(stateIds.size())
                .arg(affectedTransitions),
            QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
        if (choice != QMessageBox::Yes) {
            return;
        }
    }

    m_editorController->deleteItems(stateIds, transitions);
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
