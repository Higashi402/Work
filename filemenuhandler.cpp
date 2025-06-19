#include <QMessageBox>
#include <QFileDialog>
#include <QMdiSubWindow>

#include "q_image.h"
#include "bmp/bmp_handler.h"
#include "mainwindow.h"
#include "filemenuhandler.h"
#include "tiffio.h"

FileMenuHandler::FileMenuHandler(QObject* parent) : QObject(parent) {}

void FileMenuHandler::openImageFile(MainWindow* mainWindow)
{
    QString fileName = QFileDialog::getOpenFileName(mainWindow, "Открыть файл", "C:/Users/Higashi/Desktop/Photon/Images", "Все файлы (*.*)");

    if (fileName.isEmpty()) {
        QMessageBox::warning(mainWindow, "Ошибка", "Файл не выбран.");
        return;
    }

    QByteArray fn = fileName.toLocal8Bit();
    char *fileNamePath = fn.data();

    QImage image;

    if (fileName.endsWith(".bmp", Qt::CaseInsensitive)) {
        image = openBMPFile(fileNamePath);
    } else if (fileName.endsWith(".tiff", Qt::CaseInsensitive) || fileName.endsWith(".tif", Qt::CaseInsensitive)) {
        image = openTIFFFile(fileNamePath);
    } else {
        QMessageBox::warning(mainWindow, "Ошибка", "Неподдерживаемый формат файла.");
        return;
    }

    if (image.isNull()) {
        QMessageBox::warning(mainWindow, "Ошибка", "Не удалось загрузить изображение.");
        return;
    }

    mainWindow->displayImage(image, fileName);
}


QImage FileMenuHandler::openBMPFile(char *fileNamePath) {
    BMP* bmp = bopen(fileNamePath);
    if (bmp == nullptr) {
        return QImage();
    }

    QImage image;

    image = QImage((uchar*)bmp->pixels, bmp->width, bmp->height, QImage::Format_RGB32).mirrored(false, true);
    for (int y = 0; y < image.height(); y++) {
        uchar* scanLine = image.scanLine(y);
        for (int x = 0; x < image.width(); x++) {
            uchar temp = scanLine[x * 4];
            scanLine[x * 4] = scanLine[x * 4 + 2];
            scanLine[x * 4 + 2] = temp;
        }
    }


    return image;
}

QImage FileMenuHandler::openTIFFFile(const char *fileNamePath) {
    TIFF* tif = TIFFOpen(fileNamePath, "r");
    if (!tif) {
        return QImage();
    }

    uint32 width, height;
    uint16 samplesPerPixel, bitsPerSample;

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
    TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
    TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);

    if (samplesPerPixel != 3 || (bitsPerSample != 8 && bitsPerSample != 16)) {
        TIFFClose(tif);
        return QImage();
    }

    size_t scanlineSize = width * samplesPerPixel * (bitsPerSample / 8);
    unsigned char* buffer = (unsigned char*)_TIFFmalloc(scanlineSize);
    if (!buffer) {
        TIFFClose(tif);
        return QImage();
    }

    QImage image(width, height, QImage::Format_RGB888);

    uint16_t maxR = 0, maxG = 0, maxB = 0;
    if (bitsPerSample == 16) {
        for (uint32 row = 0; row < height; ++row) {
            TIFFReadScanline(tif, buffer, row);
            for (uint32 col = 0; col < width; ++col) {
                uint16_t* pixel = (uint16_t*)&buffer[col * samplesPerPixel * 2];
                if (pixel[0] > maxR) maxR = pixel[0];
                if (pixel[1] > maxG) maxG = pixel[1];
                if (pixel[2] > maxB) maxB = pixel[2];
            }
        }

        auto findBoundaryValue = [](uint16_t maxValue) -> uint16_t {
            if (maxValue == 0) return 1;
            int power = static_cast<int>(std::log2(maxValue)) + 1;
            return static_cast<uint16_t>(std::pow(2, power));
        };

        uint16_t boundaryR = findBoundaryValue(maxR);
        uint16_t boundaryG = findBoundaryValue(maxG);
        uint16_t boundaryB = findBoundaryValue(maxB);

        TIFFSetDirectory(tif, 0);

        for (uint32 row = 0; row < height; ++row) {
            TIFFReadScanline(tif, buffer, row);

            for (uint32 col = 0; col < width; ++col) {
                uint16_t* pixel = (uint16_t*)&buffer[col * samplesPerPixel * 2];
                uint8_t r = static_cast<uint8_t>((pixel[0] * 255) / boundaryR);
                uint8_t g = static_cast<uint8_t>((pixel[1] * 255) / boundaryG);
                uint8_t b = static_cast<uint8_t>((pixel[2] * 255) / boundaryB);
                image.setPixel(col, row, qRgb(r, g, b));
            }
        }
    } else {
        for (uint32 row = 0; row < height; ++row) {
            TIFFReadScanline(tif, buffer, row);
            for (uint32 col = 0; col < width; ++col) {
                uint8_t* pixel = &buffer[col * samplesPerPixel];
                image.setPixel(col, row, qRgb(pixel[0], pixel[1], pixel[2]));
            }
        }
    }

    _TIFFfree(buffer);
    TIFFClose(tif);

    return image;
}

void FileMenuHandler::saveImageFile(MainWindow* mainWindow)
{
    ImageLabel *imageLabel = mainWindow->getActiveImageLabel();
    if (!imageLabel) {
        QMessageBox::warning(mainWindow, "Ошибка", "Нет активного изображения для сохранения.");
        return;
    }

    QMdiSubWindow *subWindow = mainWindow->getMdiArea()->activeSubWindow();
    if (!subWindow) return;

    QString fileName = QFileDialog::getSaveFileName(mainWindow,
                                                    "Сохранить изображение",
                                                    "C:/Users/Higashi/Desktop/Photon/Images",
                                                    "BMP (*.bmp);;TIFF (*.tiff *.tif);;PNG (*.png);;JPEG (*.jpg *.jpeg);;Все файлы (*)");

    if (fileName.isEmpty()) {
        return;
    }

    QImage imageToSave = imageLabel->getCurrentImage();
    if (imageToSave.isNull()) {
        QMessageBox::warning(mainWindow, "Ошибка", "Не удалось получить изображение для сохранения.");
        return;
    }

    QString format;
    if (fileName.endsWith(".bmp", Qt::CaseInsensitive)) {
        format = "BMP";
    } else if (fileName.endsWith(".tiff", Qt::CaseInsensitive) || fileName.endsWith(".tif", Qt::CaseInsensitive)) {
        format = "TIFF";
    } else if (fileName.endsWith(".png", Qt::CaseInsensitive)) {
        format = "PNG";
    } else if (fileName.endsWith(".jpg", Qt::CaseInsensitive) || fileName.endsWith(".jpeg", Qt::CaseInsensitive)) {
        format = "JPEG";
    } else {
        fileName += ".png";
        format = "PNG";
    }

    if (!imageToSave.save(fileName, format.toUtf8().constData())) {
        QMessageBox::warning(mainWindow, "Ошибка", QString("Не удалось сохранить изображение в формате %1").arg(format));
    } else {
        subWindow->setWindowTitle(fileName);
    }
}


