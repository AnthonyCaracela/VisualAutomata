#pragma once

#include "../model/Automaton.h"
#include "../controller/AutomatonFactory.h"
#include "../simulation/SimulationResult.h"
#include <QStringList>

#include <QMainWindow>
#include <QString>

#include <memory>
#include <optional>

class QCloseEvent;
class QGraphicsView;

namespace fa::view {

class AutomatonScene;
class SimulationPanel;

class MainWindow final : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    std::unique_ptr<fa::model::Automaton> m_automaton;
    AutomatonScene* m_scene{};
    SimulationPanel* m_simulationPanel{};
    QGraphicsView* m_graphicsView{};
    std::optional<fa::sim::SimulationResult> m_lastSimulation;
    QString m_lastInput;
    QString m_filePath;
    std::size_t m_nextStep{};
    bool m_fileIsJff{false};
    bool m_dirty{false};

    void createAutomaton(fa::ctrl::AutomatonType type);
    void loadAutomaton(const QString& path);
    bool saveAutomaton(const QString& path, bool jflap);
    bool saveCurrent();
    bool confirmDiscardChanges();
    void markDirty();
    void markClean();
    void updateWindowTitle();
    void refreshScene();
    void resetSimulation();
    void showNoStatesError();
    void runInput(const QString& input);
    void stepInput(const QString& input);
    void runBatch(const QStringList& inputs);
    std::optional<int> selectedStateId() const;
    std::optional<fa::model::Transition> selectedTransition() const;

protected:
    void closeEvent(QCloseEvent* event) override;
};

} // namespace fa::view
