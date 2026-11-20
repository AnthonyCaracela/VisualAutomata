#include <QtTest>

#include "../src/model/DFA.h"
#include "../src/view/AutomatonScene.h"
#include "../src/view/TransitionItem.h"

#include <QKeyEvent>

class TestableAutomatonScene final : public fa::view::AutomatonScene {
public:
    using fa::view::AutomatonScene::keyPressEvent;
};

class ViewTest : public QObject {
    Q_OBJECT

private slots:
    void loadingAndDeletingTransition() {
        fa::model::DFA automaton;
        automaton.addState({0, "q0", true, false, 0.0, 0.0});
        automaton.addState({1, "q1", false, true, 120.0, 0.0});
        automaton.addTransition({0, 1, 'a'});

        TestableAutomatonScene scene;
        scene.loadAutomaton(automaton);
        QCOMPARE(scene.items().size(), 3);

        fa::view::TransitionItem* transition = nullptr;
        for (QGraphicsItem* item : scene.items()) {
            transition = dynamic_cast<fa::view::TransitionItem*>(item);
            if (transition != nullptr) {
                break;
            }
        }
        QVERIFY(transition != nullptr);
        transition->setSelected(true);

        QSignalSpy deletedSpy(&scene, &fa::view::AutomatonScene::transitionDeleted);
        QKeyEvent deleteKey(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
        scene.keyPressEvent(&deleteKey);
        QCOMPARE(deletedSpy.count(), 1);

        scene.loadAutomaton(automaton);
        QCOMPARE(scene.items().size(), 3);
    }
};

QTEST_MAIN(ViewTest)
#include "test_view.moc"
