#include "autocontrastdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

AutocontrastDialog::AutocontrastDialog(ImageLabel* target, QWidget* parent)
    : QDialog(parent), imageLabel(target)
{
    setWindowTitle("Автоконтрастирование");
    setFixedSize(300, 150);

    leftSpin = new QSpinBox(this);
    leftSpin->setRange(0, 10);
    leftSpin->setValue(1);

    rightSpin = new QSpinBox(this);
    rightSpin->setRange(0, 10);
    rightSpin->setValue(1);

    auto* layout = new QVBoxLayout(this);

    auto* hLayout1 = new QHBoxLayout();
    hLayout1->addWidget(new QLabel("Обрезка слева (%):"));
    hLayout1->addWidget(leftSpin);

    auto* hLayout2 = new QHBoxLayout();
    hLayout2->addWidget(new QLabel("Обрезка справа (%):"));
    hLayout2->addWidget(rightSpin);

    auto* applyButton = new QPushButton("Применить", this);
    connect(applyButton, &QPushButton::clicked, this, &AutocontrastDialog::applyAutocontrast);

    layout->addLayout(hLayout1);
    layout->addLayout(hLayout2);
    layout->addWidget(applyButton);


    separateChannelsCheck = new QCheckBox("Обрабатывать каналы отдельно", this);
    separateChannelsCheck->setChecked(false); // по умолчанию — общий режим

    contrastStrengthSlider = new QSlider(Qt::Horizontal, this);
    contrastStrengthSlider->setRange(1, 300);  // от 1% до 300%
    contrastStrengthSlider->setValue(100);     // по умолчанию — 100%

    auto* contrastLabel = new QLabel("Усиление (%):");

    auto* contrastLayout = new QHBoxLayout();
    contrastLayout->addWidget(contrastLabel);
    contrastLayout->addWidget(contrastStrengthSlider);

    layout->addLayout(hLayout1);
    layout->addLayout(hLayout2);
    layout->addWidget(separateChannelsCheck);
    layout->addLayout(contrastLayout);
    layout->addWidget(applyButton);


}

void AutocontrastDialog::applyAutocontrast() {
    if (imageLabel) {
        int left = leftSpin->value();
        int right = rightSpin->value();
        bool separate = separateChannelsCheck->isChecked();
        int strength = contrastStrengthSlider->value();

        imageLabel->applyAutoContrast(left, right, separate, strength);
    }
    accept();
}
