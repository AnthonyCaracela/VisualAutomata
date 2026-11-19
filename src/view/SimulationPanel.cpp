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

    auto* singleGroup = new QGroupBox("Prueba de Cadena", this);
    auto* singleLayout = new QVBoxLayout(singleGroup);

    _inputEdit = new QLineEdit(this);
    _inputEdit->setPlaceholderText("Ingresa cadena (ej: abba)...");

    auto* btnRow = new QHBoxLayout();
    _runBtn = new QPushButton("Ejecutar", this);
    _stepBtn = new QPushButton("Paso a paso", this);
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
    _logOutput->setReadOnly(true);
    _logOutput->setFont(QFont("Consolas", 9));
    traceLayout->addWidget(_logOutput);
    mainLayout->addWidget(traceGroup);

    auto* batchGroup = new QGroupBox("Pruebas en Paralelo (una por línea)", this);
    auto* batchLayout = new QVBoxLayout(batchGroup);
    _batchInputEdit = new QTextEdit(this);
    _batchInputEdit->setPlaceholderText("aabb\nab\nbbba\n...");
    _batchInputEdit->setMaximumHeight(110);

    _runAllBtn = new QPushButton("Probar Todas en Paralelo", this);
    batchLayout->addWidget(_batchInputEdit);
    batchLayout->addWidget(_runAllBtn);
    mainLayout->addWidget(batchGroup);

    connect(_runBtn, &QPushButton::clicked, this, [this]() {
        _logOutput->clear();
        emit runRequested(_inputEdit->text());
    });

    connect(_stepBtn, &QPushButton::clicked, this, [this]() {
        emit stepRequested();
    });

    connect(_runAllBtn, &QPushButton::clicked, this, [this]() {
        QString raw = _batchInputEdit->toPlainText();
        QStringList lines = raw.split('\n', Qt::SkipEmptyParts);
        _logOutput->clear();
        emit runAllRequested(lines);
    });
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

void SimulationPanel::showBatchResults(const QStringList& inputs, const std::vector<fa::sim::SimulationResult>& results) {
    _logOutput->clear();
    _logOutput->append("=== RESULTADOS EN PARALELO ===");

    size_t count = std::min(static_cast<size_t>(inputs.size()), results.size());
    for (size_t i = 0; i < count; ++i) {
        QString str = inputs[static_cast<int>(i)];
        const auto& res = results[i];
        QString verdict = !res.error.empty()
            ? QString("ERROR (%1)").arg(QString::fromStdString(res.error))
            : (res.accepted ? "ACEPTADA" : "RECHAZADA");

        _logOutput->append(QString("\"%1\" -> %2").arg(str, verdict));
    }

    _statusLabel->setText(QString("Lote completado (%1 cadenas)").arg(count));
    _statusLabel->setStyleSheet("font-weight: bold; color: #263238; padding: 4px;");
}

} // namespace fa::view
