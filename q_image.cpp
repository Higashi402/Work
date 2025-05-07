#include "q_image.h"
#include <QPixmap>
#include <QScrollBar>
#include <QPainter>
#include <QString>
#include <QImageWriter>
#include <QMessageBox>

ImageLabel::ImageLabel(const QImage &image, QLabel *statusLabel, QScrollArea *scrollArea, QWidget *parent)
    : QLabel(parent), originalImage(image), statusLabel(statusLabel), scrollArea(scrollArea) {
    currentImage = originalImage;
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
}

void ImageLabel::setOriginalImage(const QImage &image)
{
    if (image.isNull()) {
        qWarning() << "Attempt to set null image as original";
        return;
    }

    originalImage = image.copy();

    setCurrentImage(originalImage);

    //scaleFactor = 1.0;
    resize(originalImage.size());

    update();
}

void ImageLabel::zoomIn() {
    scaleFactor *= 1.1;
    update();
    resize(currentImage.size() * scaleFactor);
}

void ImageLabel::zoomOut() {
    scaleFactor /= 1.1;
    update();
    resize(currentImage.size() * scaleFactor);
}

void ImageLabel::updateScaleLabel(QLabel *scaleLabel) {
    scaleLabel->setText(QString("Масштаб: %1%").arg(static_cast<int>(scaleFactor * 100)));
}

void ImageLabel::mouseMoveEvent(QMouseEvent *event) {
    if (!scrollArea) {
        statusLabel->setText("");
        return;
    }

    QPointF originalPos = event->pos() / scaleFactor;

    if (!currentImage.rect().contains(originalPos.toPoint())) {
        statusLabel->setText("");
        return;
    }

    QColor pixelColor = currentImage.pixelColor(originalPos.toPoint());

    statusLabel->setText(QString("X: %1, Y: %2 | R: %3, G: %4, B: %5")
                             .arg(originalPos.x())
                             .arg(originalPos.y())
                             .arg(pixelColor.red())
                             .arg(pixelColor.green())
                             .arg(pixelColor.blue()));
}

void ImageLabel::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    QTransform transform;
    transform.scale(scaleFactor, scaleFactor);
    painter.setTransform(transform);
    painter.drawImage(0, 0, currentImage);
}

QImage ImageLabel::getCurrentImage() const {
    return currentImage;
}

void ImageLabel::setCurrentImage(const QImage &newImage) {
    currentImage = newImage;
    update();
}

void ImageLabel::applyAutoContrast(int leftClipPercent, int rightClipPercent, bool separateChannels, int strengthPercent)
{
    if (originalImage.isNull() || originalImage.format() != QImage::Format_RGB888) {
        return;
    }

    int width = originalImage.width();
    int height = originalImage.height();

    QImage newImage = originalImage;

    auto calculateScale = [](int minValue, int maxValue, int strengthPercent) -> double {
        if (maxValue == minValue) return 1.0;
        double scale = 255.0 / (maxValue - minValue);
        return scale * (strengthPercent / 100.0);
    };

    if (separateChannels) {
        // Считаем гистограммы для каждого канала
        std::vector<int> histogramR(256, 0), histogramG(256, 0), histogramB(256, 0);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                QRgb pixel = originalImage.pixel(x, y);
                histogramR[qRed(pixel)]++;
                histogramG[qGreen(pixel)]++;
                histogramB[qBlue(pixel)]++;
            }
        }

        int totalPixels = width * height;
        int leftClipCount = totalPixels * leftClipPercent / 100;
        int rightClipCount = totalPixels * rightClipPercent / 100;

        auto findBounds = [](const std::vector<int>& histogram, int leftClip, int rightClip) -> std::pair<int, int> {
            int minValue = 0, maxValue = 255;
            int currentCount = 0;

            for (int i = 0; i < 256; ++i) {
                currentCount += histogram[i];
                if (currentCount >= leftClip) {
                    minValue = i;
                    break;
                }
            }

            currentCount = 0;
            for (int i = 255; i >= 0; --i) {
                currentCount += histogram[i];
                if (currentCount >= rightClip) {
                    maxValue = i;
                    break;
                }
            }

            return {minValue, maxValue};
        };

        auto [minR, maxR] = findBounds(histogramR, leftClipCount, rightClipCount);
        auto [minG, maxG] = findBounds(histogramG, leftClipCount, rightClipCount);
        auto [minB, maxB] = findBounds(histogramB, leftClipCount, rightClipCount);

        double scaleR = calculateScale(minR, maxR, strengthPercent);
        double scaleG = calculateScale(minG, maxG, strengthPercent);
        double scaleB = calculateScale(minB, maxB, strengthPercent);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                QRgb pixel = originalImage.pixel(x, y);
                int r = std::clamp(static_cast<int>((qRed(pixel) - minR) * scaleR), 0, 255);
                int g = std::clamp(static_cast<int>((qGreen(pixel) - minG) * scaleG), 0, 255);
                int b = std::clamp(static_cast<int>((qBlue(pixel) - minB) * scaleB), 0, 255);
                newImage.setPixel(x, y, qRgb(r, g, b));
            }
        }
    } else {
        // Обработка по яркости (высчитывается средняя яркость)
        std::vector<int> histogramGray(256, 0);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                QRgb pixel = originalImage.pixel(x, y);
                int gray = qGray(pixel);
                histogramGray[gray]++;
            }
        }

        int totalPixels = width * height;
        int leftClipCount = totalPixels * leftClipPercent / 100;
        int rightClipCount = totalPixels * rightClipPercent / 100;

        auto [minGray, maxGray] = [&]() {
            int minVal = 0, maxVal = 255, count = 0;

            for (int i = 0; i < 256; ++i) {
                count += histogramGray[i];
                if (count >= leftClipCount) {
                    minVal = i;
                    break;
                }
            }

            count = 0;
            for (int i = 255; i >= 0; --i) {
                count += histogramGray[i];
                if (count >= rightClipCount) {
                    maxVal = i;
                    break;
                }
            }

            return std::make_pair(minVal, maxVal);
        }();

        double scale = calculateScale(minGray, maxGray, strengthPercent);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                QRgb pixel = originalImage.pixel(x, y);
                int r = qRed(pixel);
                int g = qGreen(pixel);
                int b = qBlue(pixel);
                int gray = qGray(pixel);
                int mapped = std::clamp(static_cast<int>((gray - minGray) * scale), 0, 255);

                double factor = mapped / 255.0;
                r = std::clamp(static_cast<int>(r * factor), 0, 255);
                g = std::clamp(static_cast<int>(g * factor), 0, 255);
                b = std::clamp(static_cast<int>(b * factor), 0, 255);

                newImage.setPixel(x, y, qRgb(r, g, b));
            }
        }
    }

    setCurrentImage(newImage);
}

