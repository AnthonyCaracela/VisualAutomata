#include <QtTest>

#include "../src/controller/EditorController.h"
#include "../src/controller/SimulationController.h"
#include "../src/model/DFA.h"

#include <QTemporaryDir>

class ControllersTest : public QObject {
    Q_OBJECT

private slots:
    void editorMutationsUpdateModelAndSignals() {
        fa::ctrl::EditorController editor;
        QSignalSpy changedSpy(&editor, &fa::ctrl::EditorController::automatonChanged);
        QSignalSpy modifiedSpy(&editor, &fa::ctrl::EditorController::modified);

        QVERIFY(editor.onStateAdded(10.0, 20.0));
        QVERIFY(editor.onStateAdded(30.0, 40.0));
        QVERIFY(editor.setFinal(1, true));
        QVERIFY(editor.onTransitionAdded(0, 1, 'a'));

        QCOMPARE(editor.automaton().states().size(), std::size_t(2));
        QCOMPARE(editor.automaton().transitions().size(), std::size_t(1));
        QVERIFY(editor.automaton().states().front().initial);
        QVERIFY(editor.automaton().states().back().final);
        QCOMPARE(changedSpy.count(), 4);
        QCOMPARE(modifiedSpy.count(), 4);
    }

    void editorLoadsAndSavesThroughAdapters() {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath(QStringLiteral("controller.json"));

        fa::ctrl::EditorController editor;
        QVERIFY(editor.onStateAdded(1.5, 2.5));
        QVERIFY(editor.onStateAdded(3.5, 4.5));
        QVERIFY(editor.onTransitionAdded(0, 1, 'b'));
        QVERIFY(editor.saveToFile(path.toStdString(), false));

        editor.newAutomaton(fa::ctrl::AutomatonType::NFA);
        QVERIFY(editor.loadFromFile(path.toStdString()));
        QCOMPARE(QString::fromStdString(editor.automaton().typeName()),
                 QStringLiteral("DFA"));
        QCOMPARE(editor.automaton().states().size(), std::size_t(2));
        QCOMPARE(editor.automaton().transitions().size(), std::size_t(1));
        QCOMPARE(editor.automaton().states().front().x, 1.5);
    }

    void batchDeletionRemovesSelectedItemsAndIncidentTransitionsOnce() {
        fa::ctrl::EditorController editor;
        QVERIFY(editor.onStateAdded(0.0, 0.0));
        QVERIFY(editor.onStateAdded(1.0, 1.0));
        QVERIFY(editor.onStateAdded(2.0, 2.0));
        QVERIFY(editor.onTransitionAdded(0, 1, 'a'));
        QVERIFY(editor.onTransitionAdded(1, 2, 'b'));
        QVERIFY(editor.onTransitionAdded(0, 2, 'c'));

        QSignalSpy changedSpy(&editor, &fa::ctrl::EditorController::automatonChanged);
        QSignalSpy modifiedSpy(&editor, &fa::ctrl::EditorController::modified);

        QVERIFY(editor.deleteItems({1}, {{0, 2, 'c'}, {0, 1, 'a'}}));
        QCOMPARE(editor.automaton().states().size(), std::size_t(2));
        QVERIFY(editor.automaton().transitions().empty());
        QCOMPARE(changedSpy.count(), 1);
        QCOMPARE(modifiedSpy.count(), 1);
    }

    void simulationRunsStepsAndBatches() {
        fa::model::DFA automaton;
        automaton.addState({0, "q0", true, false, 0.0, 0.0});
        automaton.addState({1, "q1", false, true, 1.0, 1.0});
        automaton.addTransition({0, 1, 'a'});

        fa::ctrl::SimulationController simulation(&automaton);
        bool lastAccepted = false;
        int stepCount = 0;
        std::vector<fa::sim::SimulationResult> batchResults;

        connect(&simulation, &fa::ctrl::SimulationController::resultReady, this,
                [&lastAccepted](const fa::sim::SimulationResult& result) {
                    lastAccepted = result.accepted;
                });
        connect(&simulation, &fa::ctrl::SimulationController::stepReady, this,
                [&stepCount](const fa::sim::SimulationStep&) { ++stepCount; });
        connect(&simulation, &fa::ctrl::SimulationController::batchResultsReady, this,
                [&batchResults](
                    const QStringList&,
                    const std::vector<fa::sim::SimulationResult>& results) {
                    batchResults = results;
                });

        simulation.run("a");
        QVERIFY(lastAccepted);

        simulation.reset();
        simulation.setPendingInput("a");
        simulation.nextStep();
        simulation.nextStep();
        QCOMPARE(stepCount, 2);
        QVERIFY(lastAccepted);

        simulation.runAll({"a", "b"});
        QCOMPARE(batchResults.size(), std::size_t(2));
        QVERIFY(batchResults[0].accepted);
        QVERIFY(!batchResults[1].accepted);
    }

    void emptyAutomatonProducesUsefulSimulationError() {
        fa::ctrl::EditorController editor;
        fa::ctrl::SimulationController simulation(&editor.automaton());
        QString error;
        connect(&simulation, &fa::ctrl::SimulationController::resultReady, this,
                [&error](const fa::sim::SimulationResult& result) {
                    error = QString::fromStdString(result.error);
                });

        simulation.run("a");
        QVERIFY(error.contains(QStringLiteral("Agrega al menos un estado")));
    }
};

QTEST_GUILESS_MAIN(ControllersTest)
#include "test_controllers.moc"
