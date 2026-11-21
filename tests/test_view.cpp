#include <QtTest>

#include "../src/model/DFA.h"
#include "../src/view/AutomatonScene.h"
#include "../src/view/SimulationPanel.h"
#include "../src/view/StateItem.h"
#include "../src/view/TransitionItem.h"

#include <QKeyEvent>
#include <QPlainTextEdit>
#include <QTextEdit>

class TestableAutomatonScene final : public fa::view::AutomatonScene {
public:
    using fa::view::AutomatonScene::keyPressEvent;
};

class ViewTest : public QObject {
    Q_OBJECT

private slots:
    void loadingAndRequestingBatchDeletion() {
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

        fa::view::StateItem* state = nullptr;
        for (QGraphicsItem* item : scene.items()) {
            state = dynamic_cast<fa::view::StateItem*>(item);
            if (state != nullptr) {
                break;
            }
        }
        QVERIFY(state != nullptr);
        state->setSelected(true);
        QCOMPARE(scene.selectedItems().size(), 2);

        QSignalSpy deletedSpy(&scene, &fa::view::AutomatonScene::deleteSelectionRequested);
        QKeyEvent deleteKey(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
        scene.keyPressEvent(&deleteKey);
        QCOMPARE(deletedSpy.count(), 1);

        scene.loadAutomaton(automaton);
        QCOMPARE(scene.items().size(), 3);
    }

    void batchResultsStaySeparateFromSingleRunTrace() {
        fa::view::SimulationPanel panel;
        fa::sim::SimulationStep step;
        step.index = 1;
        step.currentStates.insert(1);
        step.consumed = 'a';
        panel.showStep(step);

        fa::sim::SimulationResult accepted;
        accepted.accepted = true;
        fa::sim::SimulationResult rejected;
        rejected.accepted = false;
        panel.showBatchResults({QStringLiteral("a"), QStringLiteral("b")},
                               {accepted, rejected});

        auto* trace = panel.findChild<QTextEdit*>(QStringLiteral("singleRunTrace"));
        auto* batch = panel.findChild<QPlainTextEdit*>(QStringLiteral("batchResultsOutput"));
        QVERIFY(trace != nullptr);
        QVERIFY(batch != nullptr);
        QVERIFY(trace->toPlainText().contains(QStringLiteral("Paso 1")));
        QVERIFY(!trace->toPlainText().contains(QStringLiteral("ACEPTADA")));
        QVERIFY(batch->toPlainText().contains(QStringLiteral("\"a\" -> ACEPTADA")));
        QVERIFY(batch->toPlainText().contains(QStringLiteral("\"b\" -> RECHAZADA")));

        panel.resetTrace();
        QVERIFY(trace->toPlainText().isEmpty());
        QVERIFY(!batch->toPlainText().isEmpty());
        panel.reset();
        QVERIFY(batch->toPlainText().isEmpty());
    }
};

QTEST_MAIN(ViewTest)
#include "test_view.moc"
