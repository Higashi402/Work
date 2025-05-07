#include "medianfilterdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

#include <algorithm>
#include <vector>
#include <QPainter>

MedianFilterDialog::MedianFilterDialog(ImageLabel* target, QWidget* parent)
    : QDialog(parent), imageLabel(target), originalImage(target->getCurrentImage())
{
    setWindowTitle("Медианный фильтр - Расширенные настройки");
    setModal(true);
    //resize(600, 500);

    initUI();
    previewFilter();
}

void MedianFilterDialog::initUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Группа параметров фильтра
    QGroupBox* paramsGroup = new QGroupBox("Параметры фильтра");
    QGridLayout* paramsLayout = new QGridLayout();

    // Размер окна
    paramsLayout->addWidget(new QLabel("Размер окна:"), 0, 0);
    windowSizeSpin = new QSpinBox();
    windowSizeSpin->setRange(3, 25);
    windowSizeSpin->setSingleStep(2);
    windowSizeSpin->setValue(3);
    paramsLayout->addWidget(windowSizeSpin, 0, 1);

    // Тип обработки границ
    paramsLayout->addWidget(new QLabel("Обработка границ:"), 1, 0);
    borderTypeCombo = new QComboBox();
    borderTypeCombo->addItems({"Отражение", "Черные", "Белые", "Продолжение"});
    paramsLayout->addWidget(borderTypeCombo, 1, 1);

    // Цветовой режим
    paramsLayout->addWidget(new QLabel("Цветовой режим:"), 2, 0);
    colorModeCombo = new QComboBox();
    colorModeCombo->addItems({"RGB", "Яркость", "Красный", "Зеленый", "Синий"});
    paramsLayout->addWidget(colorModeCombo, 2, 1);

    // Сохранение границ
    preserveEdgesCheck = new QCheckBox("Сохранять границы");
    preserveEdgesCheck->setChecked(true);
    paramsLayout->addWidget(preserveEdgesCheck, 3, 0, 1, 2);

    // Порог границ
    paramsLayout->addWidget(new QLabel("Порог границ:"), 4, 0);
    edgeThresholdSpin = new QDoubleSpinBox();
    edgeThresholdSpin->setRange(0.0, 1.0);
    edgeThresholdSpin->setSingleStep(0.05);
    edgeThresholdSpin->setValue(0.2);
    paramsLayout->addWidget(edgeThresholdSpin, 4, 1);

    // Адаптивный фильтр
    adaptiveCheck = new QCheckBox("Адаптивный фильтр");
    adaptiveCheck->setChecked(false);
    paramsLayout->addWidget(adaptiveCheck, 5, 0, 1, 2);

    // Количество итераций
    paramsLayout->addWidget(new QLabel("Итерации:"), 6, 0);
    iterationsSpin = new QSpinBox();
    iterationsSpin->setRange(1, 5);
    iterationsSpin->setValue(1);
    paramsLayout->addWidget(iterationsSpin, 6, 1);

    paramsGroup->setLayout(paramsLayout);
    mainLayout->addWidget(paramsGroup);

    // Область предпросмотра
    QGroupBox* previewGroup = new QGroupBox("Предпросмотр");
    QVBoxLayout* previewLayout = new QVBoxLayout();
    previewLabel = new QLabel();
    previewLabel->setAlignment(Qt::AlignCenter);
    previewLabel->setMinimumSize(300, 200);
    previewLayout->addWidget(previewLabel);
    previewGroup->setLayout(previewLayout);
    mainLayout->addWidget(previewGroup);

    // Кнопки
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    previewButton = new QPushButton("Предпросмотр");
    applyButton = new QPushButton("Применить");
    cancelButton = new QPushButton("Отмена");

    connect(previewButton, &QPushButton::clicked, this, &MedianFilterDialog::previewFilter);
    connect(applyButton, &QPushButton::clicked, this, &MedianFilterDialog::applyFilter);
    connect(cancelButton, &QPushButton::clicked, this, &MedianFilterDialog::reject);

    buttonLayout->addWidget(previewButton);
    buttonLayout->addWidget(applyButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    connect(windowSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &MedianFilterDialog::updatePreview);
    connect(borderTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MedianFilterDialog::updatePreview);
    connect(colorModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MedianFilterDialog::updatePreview);
    connect(preserveEdgesCheck, &QCheckBox::stateChanged, this, &MedianFilterDialog::updatePreview);
    connect(edgeThresholdSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MedianFilterDialog::updatePreview);
    connect(adaptiveCheck, &QCheckBox::stateChanged, this, &MedianFilterDialog::updatePreview);
}

void MedianFilterDialog::applyMedianFilter(QImage& dest, const QImage& src, int windowSize, bool preview)
{
    int halfSize = windowSize / 2;
    int width = src.width();
    int height = src.height();

    std::vector<int> rValues, gValues, bValues;
    rValues.reserve(windowSize * windowSize);
    gValues.reserve(windowSize * windowSize);
    bValues.reserve(windowSize * windowSize);

    int borderType = borderTypeCombo->currentIndex();
    int colorMode = colorModeCombo->currentIndex();
    bool preserveEdges = preserveEdgesCheck->isChecked();
    double edgeThreshold = edgeThresholdSpin->value();
    bool adaptive = adaptiveCheck->isChecked();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            rValues.clear();
            gValues.clear();
            bValues.clear();

            // Собираем окрестность с учетом границ
            for (int ky = -halfSize; ky <= halfSize; ++ky) {
                for (int kx = -halfSize; kx <= halfSize; ++kx) {
                    int px = x + kx;
                    int py = y + ky;

                    // Обработка границ
                    if (px < 0 || px >= width || py < 0 || py >= height) {
                        switch (borderType) {
                        case 0: // Отражение
                            px = qBound(0, px, width-1);
                            py = qBound(0, py, height-1);
                            break;
                        case 1: // Черные
                            rValues.push_back(0);
                            gValues.push_back(0);
                            bValues.push_back(0);
                            continue;
                        case 2: // Белые
                            rValues.push_back(255);
                            gValues.push_back(255);
                            bValues.push_back(255);
                            continue;
                        case 3: // Продолжение
                            px = qBound(0, px, width-1);
                            py = qBound(0, py, height-1);
                            break;
                        }
                    }

                    QRgb pixel = src.pixel(px, py);
                    rValues.push_back(qRed(pixel));
                    gValues.push_back(qGreen(pixel));
                    bValues.push_back(qBlue(pixel));
                }
            }

            // Адаптивный фильтр - пропускаем "плоские" области
            if (adaptive) {
                int minR = *std::min_element(rValues.begin(), rValues.end());
                int maxR = *std::max_element(rValues.begin(), rValues.end());
                int minG = *std::min_element(gValues.begin(), gValues.end());
                int maxG = *std::max_element(gValues.begin(), gValues.end());
                int minB = *std::min_element(bValues.begin(), bValues.end());
                int maxB = *std::max_element(bValues.begin(), bValues.end());

                if ((maxR - minR) < 30 && (maxG - minG) < 30 && (maxB - minB) < 30) {
                    dest.setPixel(x, y, src.pixel(x, y));
                    continue;
                }
            }

            // Сохранение границ
            if (preserveEdges && !preview) {
                QRgb centerPixel = src.pixel(x, y);
                int centerR = qRed(centerPixel);
                int centerG = qGreen(centerPixel);
                int centerB = qBlue(centerPixel);

                double maxDiff = 0;
                for (int ky = -1; ky <= 1; ++ky) {
                    for (int kx = -1; kx <= 1; ++kx) {
                        if (x + kx >= 0 && x + kx < width && y + ky >= 0 && y + ky < height) {
                            QRgb neighbor = src.pixel(x + kx, y + ky);
                            double diff = sqrt(pow(qRed(neighbor) - centerR, 2) +
                                               pow(qGreen(neighbor) - centerG, 2) +
                                               pow(qBlue(neighbor) - centerB, 2)) / 441.67;
                            if (diff > maxDiff) maxDiff = diff;
                        }
                    }
                }

                if (maxDiff > edgeThreshold) {
                    dest.setPixel(x, y, centerPixel);
                    continue;
                }
            }

            // Находим медиану в зависимости от цветового режима
            QRgb resultPixel;
            if (colorMode == 0) { // RGB
                auto mid = rValues.begin() + rValues.size() / 2;
                std::nth_element(rValues.begin(), mid, rValues.end());
                int r = *mid;
                mid = gValues.begin() + gValues.size() / 2;
                std::nth_element(gValues.begin(), mid, gValues.end());
                int g = *mid;
                mid = bValues.begin() + bValues.size() / 2;
                std::nth_element(bValues.begin(), mid, bValues.end());
                int b = *mid;
                resultPixel = qRgb(r, g, b);
            }
            else { // Обработка отдельных каналов
                QRgb srcPixel = src.pixel(x, y);
                int r = qRed(srcPixel);
                int g = qGreen(srcPixel);
                int b = qBlue(srcPixel);

                auto mid = rValues.begin() + rValues.size() / 2;
                std::nth_element(rValues.begin(), mid, rValues.end());
                int median = *mid;

                switch (colorMode) {
                case 1: // Яркость
                    resultPixel = qRgb(median, median, median);
                    break;
                case 2: // Красный
                    resultPixel = qRgb(median, g, b);
                    break;
                case 3: // Зеленый
                    resultPixel = qRgb(r, median, b);
                    break;
                case 4: // Синий
                    resultPixel = qRgb(r, g, median);
                    break;
                }
            }

            dest.setPixel(x, y, resultPixel);
        }
    }
}

void MedianFilterDialog::previewFilter()
{
    int windowSize = windowSizeSpin->value();
    previewImage = originalImage.copy();

    applyMedianFilter(previewImage, originalImage, windowSize, true);

    QPixmap pixmap = QPixmap::fromImage(previewImage.scaled(previewLabel->size(),
                                                            Qt::KeepAspectRatio, Qt::SmoothTransformation));
    previewLabel->setPixmap(pixmap);
}

void MedianFilterDialog::updatePreview()
{
    previewFilter();
}

void MedianFilterDialog::applyFilter()
{
    int windowSize = windowSizeSpin->value();
    QImage resultImage = originalImage.copy();

    applyMedianFilter(resultImage, originalImage, windowSize, false);

    imageLabel->setImage(resultImage);

    accept();
}
