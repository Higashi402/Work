#include "sobeldialog.h"
#include <QtMath>
#include <QPainter>
#include <QImage>
#include <QPixmap>

SobelFilterDialog::SobelFilterDialog(ImageLabel* target, QWidget* parent)
    : QDialog(parent), imageLabel(target), originalImage(target->getCurrentImage())
{
    setWindowTitle("Фильтр Собеля - Выделение границ");
    setModal(true);

    initUI();
    //previewFilter();
}

void SobelFilterDialog::initUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QGroupBox* paramsGroup = new QGroupBox("Параметры фильтра");
    QGridLayout* paramsLayout = new QGridLayout();

    paramsLayout->addWidget(new QLabel("Направление:"), 0, 0);
    directionCombo = new QComboBox();
    directionCombo->addItems({"X", "Y", "Оба"});
    paramsLayout->addWidget(directionCombo, 0, 1);

    paramsLayout->addWidget(new QLabel("Цветовой режим:"), 1, 0);
    colorModeCombo = new QComboBox();
    colorModeCombo->addItems({"Яркость", "RGB"});
    paramsLayout->addWidget(colorModeCombo, 1, 1);

    paramsLayout->addWidget(new QLabel("Порог:"), 2, 0);
    thresholdSpin = new QDoubleSpinBox();
    thresholdSpin->setRange(0.0, 1.0);
    thresholdSpin->setSingleStep(0.05);
    thresholdSpin->setValue(0.3);
    paramsLayout->addWidget(thresholdSpin, 2, 1);

    paramsGroup->setLayout(paramsLayout);
    mainLayout->addWidget(paramsGroup);

    QGroupBox* previewGroup = new QGroupBox("Предпросмотр");
    QVBoxLayout* previewLayout = new QVBoxLayout();
    previewLabel = new QLabel();
    previewLabel->setAlignment(Qt::AlignCenter);
    previewLabel->setMinimumSize(300, 200);
    previewLayout->addWidget(previewLabel);
    previewGroup->setLayout(previewLayout);
    mainLayout->addWidget(previewGroup);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    previewButton = new QPushButton("Предпросмотр");
    applyButton = new QPushButton("Применить");
    cancelButton = new QPushButton("Отмена");

    connect(previewButton, &QPushButton::clicked, this, &SobelFilterDialog::previewFilter);
    connect(applyButton, &QPushButton::clicked, this, &SobelFilterDialog::applyFilter);
    connect(cancelButton, &QPushButton::clicked, this, &SobelFilterDialog::reject);

    buttonLayout->addWidget(previewButton);
    buttonLayout->addWidget(applyButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    connect(directionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SobelFilterDialog::updatePreview);
    connect(colorModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SobelFilterDialog::updatePreview);
    connect(thresholdSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SobelFilterDialog::updatePreview);
}





void SobelFilterDialog::previewFilter()
{
    previewImage = applySobel(originalImage);
    previewLabel->setPixmap(QPixmap::fromImage(previewImage).scaled(previewLabel->size(), Qt::KeepAspectRatio));
}

void SobelFilterDialog::applyFilter()
{
    imageLabel->setImage(previewImage);
    accept();
}

void SobelFilterDialog::updatePreview()
{
    previewFilter();
}

QImage SobelFilterDialog::applySobel(const QImage& image)
{
    QImage gray = image.convertToFormat(QImage::Format_Grayscale8);
    QImage result(image.size(), QImage::Format_Grayscale8);

    int w = image.width();
    int h = image.height();

    int dir = directionCombo->currentIndex(); // 0 = X, 1 = Y, 2 = оба
    double threshold = thresholdSpin->value() * 255.0;

    const int Gx[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };

    const int Gy[3][3] = {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };

    for (int y = 1; y < h - 1; ++y) {
        for (int x = 1; x < w - 1; ++x) {
            int sumX = 0, sumY = 0;

            for (int j = -1; j <= 1; ++j) {
                for (int i = -1; i <= 1; ++i) {
                    int val = qGray(gray.pixel(x + i, y + j));
                    sumX += val * Gx[j + 1][i + 1];
                    sumY += val * Gy[j + 1][i + 1];
                }
            }

            int magnitude = 0;
            if (dir == 0)      magnitude = qAbs(sumX);
            else if (dir == 1) magnitude = qAbs(sumY);
            else               magnitude = qSqrt(sumX * sumX + sumY * sumY);

            magnitude = qBound(0, magnitude, 255);
            result.setPixel(x, y, (magnitude > threshold ? qRgb(255, 255, 255) : qRgb(0, 0, 0)));
        }
    }

    return result;
}
