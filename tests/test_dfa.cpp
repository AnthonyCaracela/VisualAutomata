#include <QtTest>

#include "../src/model/DFA.h"
#include "../src/model/Exceptions.h"

class DfaTest : public QObject {
    Q_OBJECT

private slots:
    void rejectsEpsilon() {
        fa::model::DFA automaton;
        automaton.addState({0, "q0", true, false, 0.0, 0.0});
        automaton.addState({1, "q1", false, true, 1.0, 1.0});
        QVERIFY_EXCEPTION_THROWN(automaton.addTransition({0, 1, '\0'}),
                                 fa::model::InvalidTransitionException);
    }

    void rejectsNondeterminism() {
        fa::model::DFA automaton;
        automaton.addState({0, "q0", true, false, 0.0, 0.0});
        automaton.addState({1, "q1", false, false, 1.0, 1.0});
        automaton.addState({2, "q2", false, true, 2.0, 2.0});
        automaton.addTransition({0, 1, 'a'});
        QVERIFY_EXCEPTION_THROWN(automaton.addTransition({0, 2, 'a'}),
                                 fa::model::InvalidTransitionException);
    }

    void validatesAutomatonShape() {
        fa::model::DFA automaton;
        QVERIFY_EXCEPTION_THROWN(automaton.validate(), fa::model::InvalidAutomatonException);
        automaton.addState({0, "q0", true, false, 0.0, 0.0});
        automaton.addState({1, "q1", true, true, 1.0, 1.0});
        QVERIFY_EXCEPTION_THROWN(automaton.validate(), fa::model::InvalidAutomatonException);
    }

    void rejectsMissingTransitionEndpoint() {
        fa::model::DFA automaton;
        automaton.addState({0, "q0", true, false, 0.0, 0.0});
        QVERIFY_EXCEPTION_THROWN(automaton.addTransition({0, 9, 'a'}),
                                 fa::model::InvalidTransitionException);
    }
};

QTEST_GUILESS_MAIN(DfaTest)
#include "test_dfa.moc"
