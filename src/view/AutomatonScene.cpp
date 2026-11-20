#include "AutomatonScene.h"
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <algorithm>

namespace fa::view {

AutomatonScene::AutomatonScene(QObject* parent)
    : QGraphicsScene(parent) {
    setSceneRect(-1000, -1000, 2000, 2000);
    setBackgroundBrush(QColor("#FAFAFA"));
}

AutomatonScene::~AutomatonScene() {
    clearAutomatonItems();
}

void AutomatonScene::clearAutomatonItems() {
    for (auto* transition : _transitionItems) {
        delete transition;
    }
    _transitionItems.clear();
    clear();
    _stateItems.clear();
}

void AutomatonScene::setMode(Mode mode) {
    _mode = mode;
    _pendingSource = nullptr;
}

StateItem* AutomatonScene::stateItemAt(const QPointF& pos) const {
    const auto itemsAtPos = items(pos);
    for (QGraphicsItem* item : itemsAtPos) {
        if (auto* stateItem = dynamic_cast<StateItem*>(item)) {
            return stateItem;
        }
    }
    return nullptr;
}

void AutomatonScene::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
        QGraphicsScene::mousePressEvent(event);
        return;
    }

    QPointF pos = event->scenePos();

    if (_mode == Mode::AddState) {
        emit stateAdded(pos.x(), pos.y());
        return;
    }

    if (_mode == Mode::AddTransition) {
        StateItem* clickedState = stateItemAt(pos);
        if (!clickedState) {
            _pendingSource = nullptr;
            return;
        }

        if (!_pendingSource) {
            _pendingSource = clickedState;
            _pendingSource->setSelected(true);
        } else {
            StateItem* targetState = clickedState;
            bool ok = false;
            QString text = QInputDialog::getText(
                nullptr,
                "Nueva Transición",
                "Símbolo (dejar vacío para ε):",
                QLineEdit::Normal,
                "",
                &ok
            );

            _pendingSource->setSelected(false);
            if (ok) {
                const QByteArray encoded = text.toUtf8();
                if (text.isEmpty() || encoded.size() == 1) {
                    const char symbol = text.isEmpty() ? '\0' : encoded.at(0);
                    emit transitionAdded(_pendingSource->id(), targetState->id(), symbol);
                } else {
                    QMessageBox::warning(nullptr, "Transición inválida",
                                         "El símbolo debe ser un carácter de un byte.");
                }
            }
            _pendingSource = nullptr;
        }
        return;
    }

    if (_mode == Mode::Select) {
        _draggedState = stateItemAt(pos);
        if (_draggedState) {
            _dragStartPos = _draggedState->pos();
        }
    }

    QGraphicsScene::mousePressEvent(event);
}

void AutomatonScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsScene::mouseReleaseEvent(event);

    if (_mode == Mode::Select && _draggedState) {
        QPointF newPos = _draggedState->pos();
        if (newPos != _dragStartPos) {
            emit stateMoved(_draggedState->id(), newPos.x(), newPos.y());
        }
        _draggedState = nullptr;
    }
}

void AutomatonScene::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        for (QGraphicsItem* item : selectedItems()) {
            if (auto* stateItem = dynamic_cast<StateItem*>(item)) {
                emit itemDeleted(stateItem->id());
                return;
            }
            if (auto* transitionItem = dynamic_cast<TransitionItem*>(item)) {
                emit transitionDeleted(transitionItem->fromId(), transitionItem->toId(),
                                      transitionItem->symbol());
                return;
            }
        }
    }
    QGraphicsScene::keyPressEvent(event);
}

void AutomatonScene::highlightStates(const std::set<int>& activeIds) {
    for (auto& [id, item] : _stateItems) {
        item->setHighlighted(activeIds.find(id) != activeIds.end());
    }
}

void AutomatonScene::clearHighlight() {
    for (auto& [id, item] : _stateItems) {
        item->setHighlighted(false);
    }
}

void AutomatonScene::loadAutomaton(const fa::model::Automaton& automaton) {
    clearAutomatonItems();
    _pendingSource = nullptr;
    _draggedState = nullptr;

    for (const auto& s : automaton.states()) {
        auto* item = new StateItem(s.id, s.name, s.initial, s.final, s.x, s.y);
        addItem(item);
        _stateItems[s.id] = item;
    }

    for (const auto& t : automaton.transitions()) {
        auto itFrom = _stateItems.find(t.from);
        auto itTo = _stateItems.find(t.to);
        if (itFrom != _stateItems.end() && itTo != _stateItems.end()) {
            auto* tItem = new TransitionItem(itFrom->second, itTo->second, t.symbol, 0.0);
            addItem(tItem);
            _transitionItems.push_back(tItem);
        }
    }

    recalculateOffsets();
}

void AutomatonScene::recalculateOffsets() {
    std::map<std::pair<int, int>, std::vector<TransitionItem*>> groups;

    for (auto* t : _transitionItems) {
        int u = t->fromId();
        int v = t->toId();
        if (u == v) {
            groups[{u, v}].push_back(t);
        } else {
            groups[{std::min(u, v), std::max(u, v)}].push_back(t);
        }
    }

    for (auto& [pair, list] : groups) {
        if (pair.first == pair.second) {
            for (size_t i = 0; i < list.size(); ++i) {
                list[i]->setCurveOffset(static_cast<double>(i) * 28.0);
            }
            continue;
        }

        if (list.size() == 1) {
            list[0]->setCurveOffset(0.0);
        } else {
            int baseId = pair.first;
            double step = 35.0;
            for (size_t i = 0; i < list.size(); ++i) {
                int level = static_cast<int>(i / 2) + 1;
                double sign = (i % 2 == 0) ? 1.0 : -1.0;
                double directionFactor = (list[i]->fromId() == baseId) ? 1.0 : -1.0;
                list[i]->setCurveOffset(sign * level * step * directionFactor);
            }
        }
    }
}

} // namespace fa::view
