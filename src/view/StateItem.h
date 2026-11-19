#pragma once
#include <QGraphicsEllipseItem>
#include <QPainter>
#include <string>
#include <vector>

namespace fa::view {

class TransitionItem;

class StateItem : public QGraphicsEllipseItem {
public:
    static constexpr double RADIUS = 25.0;

    StateItem(int id, const std::string& name, bool initial, bool final,
              double x, double y, QGraphicsItem* parent = nullptr);

    int id() const { return _id; }
    void setHighlighted(bool highlighted);
    void setInitial(bool initial);
    void setFinal(bool final);
    void addTransition(TransitionItem* transition);
    void removeTransition(TransitionItem* transition);

    QRectF boundingRect() const override;

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    int _id;
    std::string _name;
    bool _initial;
    bool _final;
    bool _highlighted = false;
    std::vector<TransitionItem*> _transitions;
};

} // namespace fa::view
