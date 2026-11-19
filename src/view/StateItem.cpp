#include "StateItem.h"
#include "TransitionItem.h"
#include <QPen>
#include <QBrush>
#include <QFont>
#include <algorithm>

namespace fa::view {

StateItem::StateItem(int id, const std::string& name, bool initial, bool final,
                     double x, double y, QGraphicsItem* parent)
    : QGraphicsEllipseItem(-RADIUS, -RADIUS, RADIUS * 2, RADIUS * 2, parent),
      _id(id), _name(name), _initial(initial), _final(final) {
    setPos(x, y);
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
    setZValue(2.0);
}

void StateItem::setHighlighted(bool highlighted) {
    if (_highlighted != highlighted) {
        _highlighted = highlighted;
        update();
    }
}

void StateItem::setInitial(bool initial) {
    _initial = initial;
    update();
}

void StateItem::setFinal(bool final) {
    _final = final;
    update();
}

void StateItem::addTransition(TransitionItem* transition) {
    _transitions.push_back(transition);
}

void StateItem::removeTransition(TransitionItem* transition) {
    _transitions.erase(
        std::remove(_transitions.begin(), _transitions.end(), transition),
        _transitions.end()
    );
}

QRectF StateItem::boundingRect() const {
    return QRectF(-RADIUS - 35.0, -RADIUS - 5.0, RADIUS * 2 + 40.0, RADIUS * 2 + 10.0);
}

void StateItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing);

    QColor fillColor = _highlighted ? QColor("#FFF59D") : QColor("#FFFFFF");
    QColor borderColor = isSelected() ? QColor("#1976D2") : QColor("#263238");
    double borderWidth = isSelected() ? 2.5 : 1.8;

    painter->setBrush(QBrush(fillColor));
    painter->setPen(QPen(borderColor, borderWidth));
    painter->drawEllipse(QRectF(-RADIUS, -RADIUS, RADIUS * 2, RADIUS * 2));

    if (_final) {
        double innerR = RADIUS - 5.0;
        painter->drawEllipse(QRectF(-innerR, -innerR, innerR * 2, innerR * 2));
    }

    if (_initial) {
        QPointF tip(-RADIUS, 0);
        QPointF tail(-RADIUS - 28.0, 0);
        painter->drawLine(tail, tip);

        QPolygonF arrow;
        arrow << tip
              << QPointF(tip.x() - 9.0, tip.y() - 5.0)
              << QPointF(tip.x() - 9.0, tip.y() + 5.0);
        painter->setBrush(borderColor);
        painter->drawPolygon(arrow);
    }

    QFont font("Segoe UI", 10, QFont::Bold);
    painter->setFont(font);
    painter->setPen(QColor("#212121"));
    painter->drawText(QRectF(-RADIUS, -RADIUS, RADIUS * 2, RADIUS * 2),
                      Qt::AlignCenter, QString::fromStdString(_name));
}

QVariant StateItem::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == ItemPositionHasChanged) {
        for (auto* t : _transitions) {
            t->updatePosition();
        }
    }
    return QGraphicsEllipseItem::itemChange(change, value);
}

} // namespace fa::view
