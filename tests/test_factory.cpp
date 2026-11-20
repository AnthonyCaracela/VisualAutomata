#include <QtTest>

#include "../src/controller/AutomatonFactory.h"
#include "../src/model/DFA.h"
#include "../src/model/NFA.h"

class FactoryTest : public QObject {
    Q_OBJECT

private slots:
    void createsDfaAndNfa() {
        const auto dfa = fa::ctrl::AutomatonFactory::create(fa::ctrl::AutomatonType::DFA);
        const auto nfa = fa::ctrl::AutomatonFactory::create(fa::ctrl::AutomatonType::NFA);
        QVERIFY(dynamic_cast<fa::model::DFA*>(dfa.get()) != nullptr);
        QVERIFY(dynamic_cast<fa::model::NFA*>(nfa.get()) != nullptr);
        QCOMPARE(QString::fromStdString(dfa->typeName()), QStringLiteral("DFA"));
        QCOMPARE(QString::fromStdString(nfa->typeName()), QStringLiteral("NFA"));
    }
};

QTEST_GUILESS_MAIN(FactoryTest)
#include "test_factory.moc"
