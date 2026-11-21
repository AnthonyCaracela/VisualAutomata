#pragma once

#include "../controller/AutomatonFactory.h"
#include "../controller/EditorController.h"
#include "../controller/SimulationController.h"
#include "../model/Transition.h"

#include <QMainWindow>
#include <QString>

#include <optional>

class QCloseEvent;
class QGraphicsView;

namespace fa::view {

class AutomatonScene;
class SimulationPanel;

class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    fa::ctrl::EditorController* m_editorController{};
    fa::ctrl::SimulationController* m_simulationController{};
    AutomatonScene* m_scene{};
    SimulationPanel* m_simulationPanel{};
    QGraphicsView* m_graphicsView{};

    QString m_filePath;
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
    void deleteSelectedItems();
    std::optional<int> selectedStateId() const;
    std::optional<fa::model::Transition> selectedTransition() const;

protected:
    void closeEvent(QCloseEvent* event) override;
};

} // namespace fa::view
