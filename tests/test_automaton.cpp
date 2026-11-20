#include <QtTest>

#include "../src/model/DFA.h"
#include "../src/model/Exceptions.h"

#include <sstream>

class AutomatonTest : public QObject {
    Q_OBJECT

private slots:
    void stateAndTransitionEquality() {
        const fa::model::State first{1, "q0", true, false, 1.0, 2.0};
        const fa::model::State sameId{1, "other", false, true, 3.0, 4.0};
        QVERIFY(first == sameId);
        QVERIFY((fa::model::Transition{1, 2, 'a'} == fa::model::Transition{1, 2, 'a'}));
        QVERIFY(!(fa::model::Transition{1, 2, 'a'} == fa::model::Transition{1, 2, 'b'}));
    }

    void cascadeRemovalAndAlphabet() {
        fa::model::DFA automaton;
        automaton.addState({0, "q0", true, false, 0.0, 0.0});
        automaton.addState({1, "q1", false, true, 1.0, 1.0});
        automaton.addState({2, "q2", false, false, 2.0, 2.0});
        automaton.addTransition({0, 1, 'a'});
        automaton.addTransition({2, 0, 'b'});
        QCOMPARE(automaton.alphabet().size(), std::size_t(2));

        automaton.removeState(1);
        QCOMPARE(automaton.transitions().size(), std::size_t(1));
        QVERIFY(automaton.alphabet().count('a') == 0);
        QVERIFY(automaton.alphabet().count('b') == 1);
        automaton.removeTransition({2, 0, 'b'});
        QVERIFY(automaton.alphabet().empty());
    }

    void streamOutput() {
        fa::model::DFA automaton;
        automaton.addState({0, "q0", true, false, 0.0, 0.0});
        std::ostringstream stream;
        stream << automaton;
        QVERIFY(QString::fromStdString(stream.str()).contains(QStringLiteral("DFA")));
        QVERIFY(QString::fromStdString(stream.str()).contains(QStringLiteral("states: 1")));
    }

    void stateEditingHelpers() {
        fa::model::DFA automaton;
        automaton.addState({0, "q0", true, false, 0.0, 0.0});
        automaton.addState({2, "q2", false, false, 2.0, 2.0});

        QCOMPARE(automaton.initialStateId().value(), 0);
        QCOMPARE(automaton.nextStateId(), 1);
        automaton.setInitial(2);
        automaton.setFinal(2, true);
        automaton.setStateName(2, "accept");
        automaton.moveState(2, 20.0, 30.0);

        QCOMPARE(automaton.initialStateId().value(), 2);
        const auto& moved = automaton.states().back();
        QCOMPARE(QString::fromStdString(moved.name), QStringLiteral("accept"));
        QVERIFY(moved.final);
        QCOMPARE(moved.x, 20.0);
        QCOMPARE(moved.y, 30.0);
        QVERIFY_EXCEPTION_THROWN(
            automaton.setStateName(2, ""), fa::model::InvalidAutomatonException);
    }
};

QTEST_GUILESS_MAIN(AutomatonTest)
#include "test_automaton.moc"
