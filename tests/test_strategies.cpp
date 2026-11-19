#include "model/DFA.h"
#include "model/NFA.h"
#include "simulation/DFAStrategy.h"
#include "simulation/NFAStrategy.h"
#include "simulation/ParallelTester.h"
#include "simulation/Result.h"

#include <iostream>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

using namespace fa::model;
using namespace fa::sim;

static int g_checks = 0;
static int g_failures = 0;

#define CHECK(cond)                                                          \
    do {                                                                     \
        ++g_checks;                                                          \
        if (!(cond)) {                                                       \
            ++g_failures;                                                    \
            std::cerr << "FALLO " << __FILE__ << ":" << __LINE__ << " -> "  \
                      << #cond << "\n";                                      \
        }                                                                    \
    } while (0)

// ---------- Automatas de prueba ----------

// AFD: cantidad par de 'a' sobre {a,b}
static DFA makeEvenA() {
    DFA dfa;
    dfa.addState(State{0, "q0", true, true, 0, 0});
    dfa.addState(State{1, "q1", false, false, 100, 0});
    dfa.addTransition(Transition{0, 1, 'a'});
    dfa.addTransition(Transition{1, 0, 'a'});
    dfa.addTransition(Transition{0, 0, 'b'});
    dfa.addTransition(Transition{1, 1, 'b'});
    dfa.validate();
    return dfa;
}

// AFN: cadenas sobre {a,b} que terminan en "ab"
static NFA makeEndsInAb() {
    NFA nfa;
    nfa.addState(State{0, "q0", true, false, 0, 0});
    nfa.addState(State{1, "q1", false, false, 100, 0});
    nfa.addState(State{2, "q2", false, true, 200, 0});
    nfa.addTransition(Transition{0, 0, 'a'});
    nfa.addTransition(Transition{0, 0, 'b'});
    nfa.addTransition(Transition{0, 1, 'a'});
    nfa.addTransition(Transition{1, 2, 'b'});
    nfa.validate();
    return nfa;
}

// AFN con epsilon: acepta exactamente "a" o "b"
static NFA makeAorB() {
    NFA nfa;
    nfa.addState(State{0, "q0", true, false, 0, 0});
    nfa.addState(State{1, "q1", false, false, 100, 0});
    nfa.addState(State{2, "q2", false, false, 100, 100});
    nfa.addState(State{3, "q3", false, true, 200, 50});
    nfa.addTransition(Transition{0, 1, EPSILON});
    nfa.addTransition(Transition{0, 2, EPSILON});
    nfa.addTransition(Transition{1, 3, 'a'});
    nfa.addTransition(Transition{2, 3, 'b'});
    nfa.validate();
    return nfa;
}

// ---------- DFAStrategy ----------

static void testDfaAcceptance() {
    DFA dfa = makeEvenA();
    DFAStrategy s;
    CHECK(s.run(dfa, "").accepted);
    CHECK(s.run(dfa, "aa").accepted);
    CHECK(!s.run(dfa, "ab").accepted);
    CHECK(s.run(dfa, "abba").accepted);
}

static void testDfaStepTrace() {
    DFA dfa = makeEvenA();
    SimulationResult r = DFAStrategy{}.run(dfa, "ab");
    CHECK(r.steps.size() == 3);
    CHECK(r.steps[0].currentStates == std::set<int>{0});
    CHECK(r.steps[0].remaining == "ab");
    CHECK(r.steps[1].consumed == 'a');
    CHECK(r.steps[1].currentStates == std::set<int>{1});
    CHECK(r.steps[1].remaining == "b");
    CHECK(r.steps[2].remaining.empty());
}

static void testDfaInvalidSymbol() {
    DFA dfa = makeEvenA();
    SimulationResult r = DFAStrategy{}.run(dfa, "abc");
    CHECK(!r.accepted);
    CHECK(!r.error.empty());
}

static void testDfaPartialRejectsWithoutError() {
    DFA dfa;  // solo q0 -a-> q1
    dfa.addState(State{0, "q0", true, false, 0, 0});
    dfa.addState(State{1, "q1", false, true, 100, 0});
    dfa.addTransition(Transition{0, 1, 'a'});
    SimulationResult r = DFAStrategy{}.run(dfa, "aa");
    CHECK(!r.accepted);
    CHECK(r.error.empty());
}

static void testNoInitialState() {
    DFA dfa;
    dfa.addState(State{0, "q0", false, true, 0, 0});
    SimulationResult r = DFAStrategy{}.run(dfa, "");
    CHECK(!r.accepted);
    CHECK(!r.error.empty());
}

static void testWrongAutomatonType() {
    NFA nfa = makeEndsInAb();
    DFA dfa = makeEvenA();
    CHECK(!DFAStrategy{}.run(nfa, "a").error.empty());
    CHECK(!NFAStrategy{}.run(dfa, "a").error.empty());
}

// ---------- NFAStrategy ----------

static void testNfaEndsInAb() {
    NFA nfa = makeEndsInAb();
    NFAStrategy s;
    CHECK(s.run(nfa, "ab").accepted);
    CHECK(s.run(nfa, "aab").accepted);
    CHECK(!s.run(nfa, "abb").accepted);
    CHECK(!s.run(nfa, "").accepted);
    CHECK(!s.run(nfa, "ba").accepted);
}

static void testNfaEpsilon() {
    NFA nfa = makeAorB();
    NFAStrategy s;
    CHECK(s.run(nfa, "a").accepted);
    CHECK(s.run(nfa, "b").accepted);
    CHECK(!s.run(nfa, "ab").accepted);
    CHECK(!s.run(nfa, "").accepted);
    // El paso 0 ya incluye la clausura epsilon del estado inicial.
    CHECK(s.run(nfa, "a").steps[0].currentStates == (std::set<int>{0, 1, 2}));
}

static void testNfaDeadEnd() {
    NFA nfa = makeAorB();
    SimulationResult r = NFAStrategy{}.run(nfa, "ab");
    CHECK(!r.accepted);
    CHECK(r.steps.size() == 3);
    CHECK(r.steps.back().currentStates.empty());
}

// ---------- ParallelTester ----------

class ThrowingStrategy : public ISimulationStrategy {
public:
    SimulationResult run(const Automaton& a, const std::string& in) const override {
        if (in == "boom") throw std::runtime_error("fallo simulado");
        return DFAStrategy{}.run(a, in);
    }
};

static void testParallelKeepsOrder() {
    DFA dfa = makeEvenA();
    std::vector<std::string> inputs = {"", "aa", "ab", "abba"};
    auto results = ParallelTester(3).testAll(dfa, inputs, DFAStrategy{});
    CHECK(results.size() == 4);
    CHECK(results[0].accepted);
    CHECK(results[1].accepted);
    CHECK(!results[2].accepted);
    CHECK(results[3].accepted);
}

static void testParallelEmptyInput() {
    DFA dfa = makeEvenA();
    CHECK(ParallelTester().testAll(dfa, {}, DFAStrategy{}).empty());
}

static void testParallelIsolatesExceptions() {
    DFA dfa = makeEvenA();
    std::vector<std::string> inputs = {"aa", "boom", "ab"};
    auto results = ParallelTester(2).testAll(dfa, inputs, ThrowingStrategy{});
    CHECK(results.size() == 3);
    CHECK(results[0].accepted);
    CHECK(!results[1].accepted && results[1].error == "fallo simulado");
    CHECK(!results[2].accepted && results[2].error.empty());
}

// ---------- Result<T> ----------

static void testResult() {
    auto ok = Result<int>::success(5);
    auto bad = Result<int>::failure("mal");
    CHECK(ok.ok() && ok.value() == 5);
    CHECK(!bad.ok() && bad.error() == "mal");
    CHECK(bad.valueOr(-1) == -1);
    CHECK(ok.map([](int x) { return x * 2; }).value() == 10);
    CHECK(bad.map([](int x) { return x * 2; }).error() == "mal");

    bool threw = false;
    try {
        bad.value();
    } catch (const std::logic_error&) {
        threw = true;
    }
    CHECK(threw);
}

int main() {
    testDfaAcceptance();
    testDfaStepTrace();
    testDfaInvalidSymbol();
    testDfaPartialRejectsWithoutError();
    testNoInitialState();
    testWrongAutomatonType();
    testNfaEndsInAb();
    testNfaEpsilon();
    testNfaDeadEnd();
    testParallelKeepsOrder();
    testParallelEmptyInput();
    testParallelIsolatesExceptions();
    testResult();

    std::cout << "checks: " << g_checks << " | fallos: " << g_failures << "\n";
    return g_failures == 0 ? 0 : 1;
}
