#pragma once
#include <QGraphicsPathItem>
#include <QPainter>
#include "StateItem.h"

namespace fa::view {

class TransitionItem : public QGraphicsPathItem {
public:
    TransitionItem(StateItem* from, StateItem* to, char symbol,
                   double curveOffset = 0.0, QGraphicsItem* parent = nullptr);
    ~TransitionItem() override;

    int fromId() const { return _from->id(); }
    int toId() const { return _to->id(); }
    char symbol() const { return _symbol; }
    void setCurveOffset(double offset);
    void updatePosition();

    QRectF boundingRect() const override;

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    StateItem* _from;
    StateItem* _to;
    char _symbol;
    double _curveOffset;
    QPolygonF _arrowHead;
    QPointF _labelPos;

    QPolygonF buildArrowHead(const QPointF& tip, double angleRad) const;
};

} // namespace fa::view
