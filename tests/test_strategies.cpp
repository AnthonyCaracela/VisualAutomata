#include <QtTest>

#include "../src/model/DFA.h"
#include "../src/model/NFA.h"
#include "../src/simulation/DFAStrategy.h"
#include "../src/simulation/NFAStrategy.h"
#include "../src/simulation/ParallelTester.h"

#include <stdexcept>

class ThrowingStrategy final : public fa::sim::ISimulationStrategy {
public:
    fa::sim::SimulationResult run(const fa::model::Automaton&,
                                  const std::string& input) const override {
        if (input == "boom") {
            throw std::runtime_error("simulated failure");
        }
        fa::sim::SimulationResult result;
        result.accepted = input == "accepted";
        return result;
    }
};

class StrategiesTest : public QObject {
    Q_OBJECT

private slots:
    void dfaStepsAndAcceptance() {
        fa::model::DFA automaton;
        automaton.addState({0, "q0", true, false, 0.0, 0.0});
        automaton.addState({1, "q1", false, true, 1.0, 1.0});
        automaton.addTransition({0, 1, 'a'});

        const auto result = fa::sim::DFAStrategy().run(automaton, "a");
        QVERIFY(result.accepted);
        QCOMPARE(result.steps.size(), std::size_t(2));
        QVERIFY(result.steps[0].currentStates.count(0) == 1);
        QVERIFY(result.steps[1].currentStates.count(1) == 1);
        QCOMPARE(QString::fromStdString(result.steps[1].remaining), QString());

        const auto rejected = fa::sim::DFAStrategy().run(automaton, "b");
        QVERIFY(!rejected.accepted);
        QVERIFY(!rejected.error.empty());
    }

    void nfaEpsilonClosureAndBatch() {
        fa::model::NFA automaton;
        automaton.addState({0, "q0", true, false, 0.0, 0.0});
        automaton.addState({1, "q1", false, false, 1.0, 1.0});
        automaton.addState({2, "q2", false, true, 2.0, 2.0});
        automaton.addTransition({0, 1, '\0'});
        automaton.addTransition({1, 2, 'a'});

        const fa::sim::NFAStrategy strategy;
        const auto result = strategy.run(automaton, "a");
        QVERIFY(result.accepted);
        QVERIFY(result.steps.front().currentStates.count(1) == 1);
        QCOMPARE(result.steps.size(), std::size_t(2));

        const auto results = fa::sim::ParallelTester().testAll(automaton, {"a", ""}, strategy);
        QCOMPARE(results.size(), std::size_t(2));
        QVERIFY(results[0].accepted);
        QVERIFY(!results[1].accepted);
    }

    void parallelTesterPreservesOrderAndReportsWorkerErrors() {
        const auto results = fa::sim::ParallelTester(2).testAll(
            fa::model::DFA(), {"accepted", "boom", "rejected"}, ThrowingStrategy());

        QCOMPARE(results.size(), std::size_t(3));
        QVERIFY(results[0].accepted);
        QVERIFY(!results[1].accepted);
        QCOMPARE(QString::fromStdString(results[1].error), QStringLiteral("simulated failure"));
        QVERIFY(!results[2].accepted);
        QVERIFY(results[2].error.empty());
    }
};

QTEST_GUILESS_MAIN(StrategiesTest)
#include "test_strategies.moc"
