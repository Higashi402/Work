#include "imageviewer.h"
#include <QVBoxLayout>
#include <QStatusBar>
#include <QFileInfo>
#include <QMainWindow>
#include "tiffio.h"
#include "bmp_handler.h"

ImageViewer::ImageViewer(const QString &filePath, QWidget *parent)
    : QWidget(parent), valid(false)
{
    imageLabel = new QLabel(this);
    scrollArea = new QScrollArea(this);
    scrollArea->setWidget(imageLabel);
    scrollArea->setWidgetResizable(true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(scrollArea);
    setLayout(layout);

    QFileInfo fileInfo(filePath);
    if (fileInfo.suffix().toLower() == "bmp") {
        loadBmpImage(filePath);
    } else if (fileInfo.suffix().toLower() == "tiff") {
        loadTiffImage(filePath);
    }
}

void ImageViewer::loadBmpImage(const QString &filePath)
{
    QByteArray fn = filePath.toLocal8Bit();
    char *fileNamePath = fn.data();

    BMP* bmp = bopen(fileNamePath);
    if (bmp == nullptr) {
        valid = false;
        return;
    }

    // Используем ваш код для создания QImage из BMP
    QImage image((uchar*)bmp->pixels, bmp->width, bmp->height, QImage::Format_RGB32);

    // Отражение изображения по вертикали
    image = image.mirrored(false, true);

    // Обработка цветовой компоненты, если глубина не 8 бит
    if (bmp->depth != 8) {
        for (int y = 0; y < image.height(); y++) {
            uchar* scanLine = image.scanLine(y);
            for (int x = 0; x < image.width(); x++) {
                uchar temp = scanLine[x * 4];
                scanLine[x * 4] = scanLine[x * 4 + 2];
                scanLine[x * 4 + 2] = temp;
            }
        }
    }

    if (!image.isNull()) {
        imageLabel->setPixmap(QPixmap::fromImage(image));
        valid = true;
    }

    bclose(bmp); // Освобождаем ресурсы, если у вас есть функция закрытия BMP
}

void ImageViewer::loadTiffImage(const QString &filePath)
{
    TIFF* tif = TIFFOpen(filePath.toStdString().c_str(), "r");
    if (!tif) {
        return;
    }

    uint32 width, height;
    uint16 samplesPerPixel, bitsPerSample;
    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
    TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
    TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);

    if (samplesPerPixel != 1 || bitsPerSample != 8) {
        TIFFClose(tif);
        return;
    }

    unsigned char *buffer = (unsigned char *)_TIFFmalloc(width * height);
    for (uint32 row = 0; row < height; row++) {
        TIFFReadScanline(tif, &buffer[row * width], row);
    }
    TIFFClose(tif);

    image = QImage(buffer, width, height, QImage::Format_Grayscale8).copy();
    _TIFFfree(buffer);

    if (!image.isNull()) {
        imageLabel->setPixmap(QPixmap::fromImage(image));
        valid = true;
    }
}

bool ImageViewer::isValid() const
{
    return valid;
}

void ImageViewer::mouseMoveEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();
    if (image.rect().contains(pos)) {
        QRgb pixelValue = image.pixel(pos);
        int brightness = qGray(pixelValue);
        QString statusText = QString("Координаты: (%1, %2), Яркость: %3")
                                 .arg(pos.x())
                                 .arg(pos.y())
                                 .arg(brightness);
        dynamic_cast<QMainWindow *>(parentWidget()->parentWidget())->statusBar()->showMessage(statusText);
    }
}
