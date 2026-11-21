#include "SimulationPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFont>
#include <algorithm>

namespace fa::view {

SimulationPanel::SimulationPanel(QWidget* parent)
    : QWidget(parent) {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(6);

    auto* singleGroup = new QGroupBox("Prueba de Cadena", this);
    auto* singleLayout = new QVBoxLayout(singleGroup);

    _inputEdit = new QLineEdit(this);
    _inputEdit->setPlaceholderText("Ingresa cadena (ej: abba)...");

    auto* btnRow = new QHBoxLayout();
    _runBtn = new QPushButton("Ejecutar", this);
    _runBtn->setObjectName(QStringLiteral("runButton"));
    _runBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background:#2E7D32; color:white; border:1px solid #1B5E20;"
        " border-radius:5px; padding:6px; font-weight:600; }"
        "QPushButton:hover { background:#388E3C; }"
        "QPushButton:pressed { background:#1B5E20; }"));
    _stepBtn = new QPushButton("Paso a paso", this);
    _stepBtn->setObjectName(QStringLiteral("stepButton"));
    _stepBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background:#1565C0; color:white; border:1px solid #0D47A1;"
        " border-radius:5px; padding:6px; font-weight:600; }"
        "QPushButton:hover { background:#1976D2; }"
        "QPushButton:pressed { background:#0D47A1; }"));
    btnRow->addWidget(_runBtn);
    btnRow->addWidget(_stepBtn);

    _statusLabel = new QLabel("Estado: Listo", this);
    _statusLabel->setStyleSheet("font-weight: bold; padding: 4px;");

    singleLayout->addWidget(_inputEdit);
    singleLayout->addLayout(btnRow);
    singleLayout->addWidget(_statusLabel);
    mainLayout->addWidget(singleGroup);

    auto* traceGroup = new QGroupBox("Traza de Ejecución", this);
    auto* traceLayout = new QVBoxLayout(traceGroup);
    _logOutput = new QTextEdit(this);
    _logOutput->setObjectName(QStringLiteral("singleRunTrace"));
    _logOutput->setReadOnly(true);
    _logOutput->setFont(QFont("Consolas", 9));
    traceLayout->addWidget(_logOutput);
    mainLayout->addWidget(traceGroup, 2);

    auto* batchGroup = new QGroupBox("Pruebas en Paralelo (una por línea)", this);
    auto* batchLayout = new QVBoxLayout(batchGroup);
    _batchInputEdit = new QTextEdit(this);
    _batchInputEdit->setPlaceholderText("aabb\nab\nbbba\n...");
    _batchInputEdit->setMaximumHeight(78);

    _runAllBtn = new QPushButton("Probar Todas en Paralelo", this);
    _runAllBtn->setObjectName(QStringLiteral("runAllButton"));
    _runAllBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background:#6A1B9A; color:white; border:1px solid #4A148C;"
        " border-radius:5px; padding:6px; font-weight:600; }"
        "QPushButton:hover { background:#7B1FA2; }"
        "QPushButton:pressed { background:#4A148C; }"));
    batchLayout->addWidget(_batchInputEdit);
    batchLayout->addWidget(_runAllBtn);
    mainLayout->addWidget(batchGroup);

    auto* batchResultsGroup = new QGroupBox("Resultados en paralelo", this);
    auto* batchResultsLayout = new QVBoxLayout(batchResultsGroup);
    _batchResultsOutput = new QPlainTextEdit(this);
    _batchResultsOutput->setObjectName(QStringLiteral("batchResultsOutput"));
    _batchResultsOutput->setReadOnly(true);
    _batchResultsOutput->setFont(QFont("Consolas", 9));
    _batchResultsOutput->setMinimumHeight(64);
    _batchResultsOutput->setMaximumHeight(110);
    batchResultsLayout->addWidget(_batchResultsOutput);
    mainLayout->addWidget(batchResultsGroup, 1);
    mainLayout->setStretch(1, 2);

    connect(_runBtn, &QPushButton::clicked, this, [this]() {
        resetTrace();
        emit runRequested(_inputEdit->text());
    });

    connect(_stepBtn, &QPushButton::clicked, this, [this]() {
        emit stepRequested(_inputEdit->text());
    });

    connect(_runAllBtn, &QPushButton::clicked, this, [this]() {
        QString raw = _batchInputEdit->toPlainText();
        QStringList lines = raw.split('\n', Qt::SkipEmptyParts);
        _batchResultsOutput->clear();
        emit runAllRequested(lines);
    });
}

void SimulationPanel::reset() {
    resetTrace();
    _batchResultsOutput->clear();
}

void SimulationPanel::resetTrace() {
    _logOutput->clear();
    _statusLabel->setText(QStringLiteral("Estado: Listo"));
    _statusLabel->setStyleSheet(QStringLiteral("font-weight: bold; padding: 4px;"));
}

void SimulationPanel::showStep(const fa::sim::SimulationStep& step) {
    QStringList stateIds;
    for (int id : step.currentStates) {
        stateIds << QString("q%1").arg(id);
    }

    QString consumedStr = (step.consumed == '\0')
        ? QString::fromUtf8("ε (inicio)")
        : QString("'%1'").arg(QChar(step.consumed));

    QString remainingStr = step.remaining.empty()
        ? QString::fromUtf8("ε")
        : QString::fromStdString(step.remaining);

    QString line = QString("Paso %1 | Leído: %2 | Estados: {%3} | Restante: %4")
                       .arg(step.index)
                       .arg(consumedStr)
                       .arg(stateIds.join(", "))
                       .arg(remainingStr);

    _logOutput->append(line);
    _statusLabel->setText(QString("Estado: Ejecutando paso %1...").arg(step.index));
    _statusLabel->setStyleSheet("font-weight: bold; color: #1565C0; padding: 4px;");
}

void SimulationPanel::showResult(const fa::sim::SimulationResult& result) {
    if (!result.error.empty()) {
        _statusLabel->setText("Error: " + QString::fromStdString(result.error));
        _statusLabel->setStyleSheet("font-weight: bold; color: #D32F2F; background: #FFEBEE; padding: 4px;");
        return;
    }

    if (_logOutput->toPlainText().isEmpty()) {
        for (const auto& step : result.steps) {
            showStep(step);
        }
    }

    if (result.accepted) {
        _statusLabel->setText("Resultado: CADENA ACEPTADA");
        _statusLabel->setStyleSheet("font-weight: bold; color: #2E7D32; background: #E8F5E9; padding: 4px;");
    } else {
        _statusLabel->setText("Resultado: CADENA RECHAZADA");
        _statusLabel->setStyleSheet("font-weight: bold; color: #C62828; background: #FFEBEE; padding: 4px;");
    }
}

void SimulationPanel::showBatchResults(
    const QStringList& inputs,
    const std::vector<fa::sim::SimulationResult>& results) {
    _batchResultsOutput->clear();
    size_t count = std::min(static_cast<size_t>(inputs.size()), results.size());
    if (count == 0) {
        _batchResultsOutput->appendPlainText(QStringLiteral("No hay cadenas para probar."));
        return;
    }

    size_t acceptedCount = 0;
    size_t rejectedCount = 0;
    size_t errorCount = 0;
    for (size_t i = 0; i < count; ++i) {
        const QString input = inputs[static_cast<int>(i)];
        const auto& res = results[i];
        if (!res.error.empty()) {
            ++errorCount;
            _batchResultsOutput->appendPlainText(
                QStringLiteral("\"%1\" -> ERROR: %2")
                    .arg(input, QString::fromStdString(res.error)));
        } else if (res.accepted) {
            ++acceptedCount;
            _batchResultsOutput->appendPlainText(
                QStringLiteral("\"%1\" -> ACEPTADA").arg(input));
        } else {
            ++rejectedCount;
            _batchResultsOutput->appendPlainText(
                QStringLiteral("\"%1\" -> RECHAZADA").arg(input));
        }
    }

    _batchResultsOutput->appendPlainText(
        QStringLiteral("Resumen: %1 aceptada(s), %2 rechazada(s), %3 error(es).")
            .arg(acceptedCount)
            .arg(rejectedCount)
            .arg(errorCount));
}

} // namespace fa::view
