#include <QtTest>

#include "../src/model/NFA.h"

class NfaTest : public QObject {
    Q_OBJECT

private slots:
    void supportsEpsilonAndNondeterminism() {
        fa::model::NFA automaton;
        automaton.addState({0, "q0", true, false, 0.0, 0.0});
        automaton.addState({1, "q1", false, false, 1.0, 1.0});
        automaton.addState({2, "q2", false, true, 2.0, 2.0});
        automaton.addTransition({0, 1, '\0'});
        automaton.addTransition({0, 1, 'a'});
        automaton.addTransition({0, 2, 'a'});

        QCOMPARE(automaton.transitions().size(), std::size_t(3));
        QCOMPARE(automaton.alphabet().size(), std::size_t(1));
        QVERIFY(automaton.alphabet().count('a') == 1);
        QVERIFY(automaton.alphabet().count('\0') == 0);
        automaton.validate();
    }
};

QTEST_GUILESS_MAIN(NfaTest)
#include "test_nfa.moc"
