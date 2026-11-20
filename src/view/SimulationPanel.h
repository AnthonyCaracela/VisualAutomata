#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QStringList>
#include <vector>
#include "../simulation/SimulationStep.h"
#include "../simulation/SimulationResult.h"

namespace fa::view {

class SimulationPanel : public QWidget {
    Q_OBJECT
public:
    explicit SimulationPanel(QWidget* parent = nullptr);
    void reset();

signals:
    void runRequested(QString input);
    void stepRequested(QString input);
    void runAllRequested(QStringList inputs);

public slots:
    void showStep(const fa::sim::SimulationStep& step);
    void showResult(const fa::sim::SimulationResult& result);
    void showBatchResults(const QStringList& inputs, const std::vector<fa::sim::SimulationResult>& results);

private:
    QLineEdit* _inputEdit;
    QPushButton* _runBtn;
    QPushButton* _stepBtn;
    QTextEdit* _batchInputEdit;
    QPushButton* _runAllBtn;
    QLabel* _statusLabel;
    QTextEdit* _logOutput;
};

} // namespace fa::view
