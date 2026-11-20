#pragma once
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <set>
#include <map>
#include <vector>
#include "StateItem.h"
#include "TransitionItem.h"
#include "../model/Automaton.h"

namespace fa::view {

class AutomatonScene : public QGraphicsScene {
    Q_OBJECT
public:
    enum class Mode { Select, AddState, AddTransition };

    explicit AutomatonScene(QObject* parent = nullptr);
    ~AutomatonScene() override;
    void setMode(Mode mode);
    Mode mode() const { return _mode; }

signals:
    void stateAdded(double x, double y);
    void stateMoved(int id, double x, double y);
    void transitionAdded(int from, int to, char symbol);
    void itemDeleted(int id);
    void transitionDeleted(int from, int to, char symbol);

public slots:
    void highlightStates(const std::set<int>& activeIds);
    void clearHighlight();
    void loadAutomaton(const fa::model::Automaton& automaton);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    Mode _mode = Mode::Select;
    StateItem* _pendingSource = nullptr;
    StateItem* _draggedState = nullptr;
    QPointF _dragStartPos;

    std::map<int, StateItem*> _stateItems;
    std::vector<TransitionItem*> _transitionItems;

    StateItem* stateItemAt(const QPointF& pos) const;
    void recalculateOffsets();
    void clearAutomatonItems();
};

} // namespace fa::view
