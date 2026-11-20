#include <QtTest>

#include "../src/adapters/JffAdapter.h"
#include "../src/adapters/JsonAdapter.h"
#include "../src/model/DFA.h"
#include "../src/model/NFA.h"

#include <QFile>
#include <QTemporaryDir>

#include <stdexcept>

namespace {

void compareAutomata(const fa::model::Automaton& expected,
                     const fa::model::Automaton& actual) {
    QCOMPARE(QString::fromStdString(actual.typeName()),
             QString::fromStdString(expected.typeName()));
    QCOMPARE(actual.states().size(), expected.states().size());
    QCOMPARE(actual.transitions().size(), expected.transitions().size());
    for (std::size_t i = 0; i < expected.states().size(); ++i) {
        const auto& left = expected.states()[i];
        const auto& right = actual.states()[i];
        QCOMPARE(right.id, left.id);
        QCOMPARE(QString::fromStdString(right.name), QString::fromStdString(left.name));
        QCOMPARE(right.initial, left.initial);
        QCOMPARE(right.final, left.final);
        QCOMPARE(right.x, left.x);
        QCOMPARE(right.y, left.y);
    }
    for (std::size_t i = 0; i < expected.transitions().size(); ++i) {
        QVERIFY(actual.transitions()[i] == expected.transitions()[i]);
    }
}

} // namespace

class AdaptersTest : public QObject {
    Q_OBJECT

private slots:
    void jsonRoundTrip() {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        fa::model::DFA expected;
        expected.addState({0, "q0", true, false, 12.5, 23.75});
        expected.addState({1, "q1", false, true, -4.25, 8.5});
        expected.addTransition({0, 1, 'a'});

        const QString path = directory.filePath(QStringLiteral("automaton.json"));
        const fa::adapters::JsonAdapter adapter;
        adapter.exportTo(expected, path.toStdString());
        const auto actual = adapter.importFrom(path.toStdString());
        compareAutomata(expected, *actual);
    }

    void jsonEpsilonRoundTrip() {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        fa::model::NFA expected;
        expected.addState({0, "q0", true, false, 1.0, 2.0});
        expected.addState({1, "q1", false, true, 3.0, 4.0});
        expected.addTransition({0, 1, '\0'});

        const QString path = directory.filePath(QStringLiteral("epsilon.json"));
        const fa::adapters::JsonAdapter adapter;
        adapter.exportTo(expected, path.toStdString());
        const auto actual = adapter.importFrom(path.toStdString());
        compareAutomata(expected, *actual);
    }

    void jffRoundTripAndEpsilonInference() {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        fa::model::NFA expected;
        expected.addState({0, "start", true, false, 100.125, 150.25});
        expected.addState({1, "accept", false, true, 200.5, 250.75});
        expected.addTransition({0, 1, '\0'});

        const QString path = directory.filePath(QStringLiteral("automaton.jff"));
        const fa::adapters::JffAdapter adapter;
        adapter.exportTo(expected, path.toStdString());
        const auto actual = adapter.importFrom(path.toStdString());
        compareAutomata(expected, *actual);
    }

    void malformedJsonFailsExplicitly() {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath(QStringLiteral("invalid.json"));
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("{ invalid");
        file.close();
        QVERIFY_EXCEPTION_THROWN(fa::adapters::JsonAdapter().importFrom(path.toStdString()),
                                 std::runtime_error);
    }

    void missingJffFailsExplicitly() {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        QVERIFY_EXCEPTION_THROWN(
            fa::adapters::JffAdapter().importFrom(
                directory.filePath(QStringLiteral("missing.jff")).toStdString()),
            std::runtime_error);
    }
};

QTEST_GUILESS_MAIN(AdaptersTest)
#include "test_adapters.moc"
