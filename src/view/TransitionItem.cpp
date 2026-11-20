#include "TransitionItem.h"
#include <QPen>
#include <QFont>
#include <QFontMetricsF>
#include <cmath>

namespace fa::view {

namespace {
constexpr double PI = 3.14159265358979323846;
constexpr double ARROW_SIZE = 10.0;
constexpr double ARROW_ANGLE = PI / 6.0;
} // namespace

TransitionItem::TransitionItem(StateItem* from, StateItem* to, char symbol,
                               double curveOffset, QGraphicsItem* parent)
    : QGraphicsPathItem(parent),
      _from(from), _to(to), _symbol(symbol), _curveOffset(curveOffset) {
    setFlags(ItemIsSelectable);
    setZValue(1.0);
    _from->addTransition(this);
    if (_to != _from) {
        _to->addTransition(this);
    }
    updatePosition();
}

TransitionItem::~TransitionItem() {
    if (_from) _from->removeTransition(this);
    if (_to && _to != _from) _to->removeTransition(this);
}

void TransitionItem::setCurveOffset(double offset) {
    _curveOffset = offset;
    updatePosition();
}

QPolygonF TransitionItem::buildArrowHead(const QPointF& tip, double angleRad) const {
    QPointF p1(tip.x() - ARROW_SIZE * std::cos(angleRad - ARROW_ANGLE),
               tip.y() - ARROW_SIZE * std::sin(angleRad - ARROW_ANGLE));
    QPointF p2(tip.x() - ARROW_SIZE * std::cos(angleRad + ARROW_ANGLE),
               tip.y() - ARROW_SIZE * std::sin(angleRad + ARROW_ANGLE));

    QPolygonF poly;
    poly << tip << p1 << p2;
    return poly;
}

void TransitionItem::updatePosition() {
    prepareGeometryChange();
    QPainterPath path;
    const double R = StateItem::RADIUS;

    if (_from == _to) {
        QPointF center = _from->pos();
        double a1 = -125.0 * PI / 180.0;
        double a2 = -55.0 * PI / 180.0;

        QPointF start(center.x() + R * std::cos(a1), center.y() + R * std::sin(a1));
        QPointF end(center.x() + R * std::cos(a2), center.y() + R * std::sin(a2));

        double loopHeight = 55.0 + std::abs(_curveOffset);
        QPointF c1(center.x() - 32.0, center.y() - R - loopHeight);
        QPointF c2(center.x() + 32.0, center.y() - R - loopHeight);

        path.moveTo(start);
        path.cubicTo(c1, c2, end);

        double angle = std::atan2(end.y() - c2.y(), end.x() - c2.x());
        _arrowHead = buildArrowHead(end, angle);
        _labelPos = QPointF(center.x(), center.y() - R - loopHeight * 0.78);

        setPath(path);
        return;
    }

    QPointF p1 = _from->pos();
    QPointF p2 = _to->pos();
    double dx = p2.x() - p1.x();
    double dy = p2.y() - p1.y();
    double dist = std::hypot(dx, dy);

    if (dist < 2.0 * R) {
        setPath(path);
        _arrowHead.clear();
        return;
    }

    double ux = dx / dist;
    double uy = dy / dist;
    double nx = -uy;
    double ny = ux;

    QPointF mid((p1.x() + p2.x()) * 0.5, (p1.y() + p2.y()) * 0.5);
    QPointF control(mid.x() + nx * _curveOffset, mid.y() + ny * _curveOffset);

    double angleStart = std::atan2(control.y() - p1.y(), control.x() - p1.x());
    double angleEnd   = std::atan2(p2.y() - control.y(), p2.x() - control.x());

    QPointF start(p1.x() + R * std::cos(angleStart), p1.y() + R * std::sin(angleStart));
    QPointF end(p2.x() - R * std::cos(angleEnd),     p2.y() - R * std::sin(angleEnd));

    path.moveTo(start);
    if (std::abs(_curveOffset) < 1e-3) {
        path.lineTo(end);
    } else {
        path.quadTo(control, end);
    }

    _arrowHead = buildArrowHead(end, angleEnd);

    QPointF curveMid = 0.25 * start + 0.5 * control + 0.25 * end;
    double labelShift = (_curveOffset >= 0) ? 12.0 : -12.0;
    _labelPos = QPointF(curveMid.x() + nx * labelShift, curveMid.y() + ny * labelShift);

    setPath(path);
}

QRectF TransitionItem::boundingRect() const {
    return path().boundingRect().adjusted(-25.0, -25.0, 25.0, 25.0);
}

void TransitionItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    if (path().isEmpty()) return;

    painter->setRenderHint(QPainter::Antialiasing);

    QColor strokeColor = isSelected() ? QColor("#1976D2") : QColor("#37474F");
    painter->setPen(QPen(strokeColor, isSelected() ? 2.2 : 1.6));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path());

    painter->setBrush(strokeColor);
    painter->drawPolygon(_arrowHead);

    QString text = (_symbol == '\0') ? QString::fromUtf8("ε") : QString(QChar(_symbol));
    QFont font("Segoe UI", 10, QFont::Bold);
    painter->setFont(font);

    QFontMetricsF fm(font);
    QRectF textRect = fm.boundingRect(text);
    textRect.moveCenter(_labelPos);
    textRect.adjust(-4, -2, 4, 2);

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(255, 255, 255, 220));
    painter->drawRoundedRect(textRect, 3, 3);

    painter->setPen(QColor("#D32F2F"));
    painter->drawText(textRect, Qt::AlignCenter, text);
}

} // namespace fa::view
