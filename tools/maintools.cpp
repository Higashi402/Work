#include <QMessageBox>

#include "q_image.h"
#include "mainwindow.h"
#include "maintools.h"

MainTools::MainTools(QObject* parent) : QObject(parent) {}

void MainTools::zoomIn(MainWindow* mainWindow) {
    if (ImageLabel *imageLabel = mainWindow->getActiveImageLabel()) {
        imageLabel->zoomIn();
        imageLabel->updateScaleLabel(mainWindow->getScaleLabel());
    }
}

void MainTools::zoomOut(MainWindow* mainWindow) {
    if (ImageLabel *imageLabel = mainWindow->getActiveImageLabel()) {
        imageLabel->zoomOut();
        imageLabel->updateScaleLabel(mainWindow->getScaleLabel());
    }
}

void MainTools::rotateImage(MainWindow* mainWindow)
{
    ImageLabel *imageLabel = mainWindow->getActiveImageLabel();
    if (!imageLabel) {
        QMessageBox::warning(mainWindow, "Ошибка", "Нет активного изображения для поворота.");
        return;
    }

    QImage currentImage = imageLabel->getCurrentImage();
    if (currentImage.isNull()) {
        QMessageBox::warning(mainWindow, "Ошибка", "Не удалось получить изображение для поворота.");
        return;
    }

    QTransform transform;
    transform.rotate(90);
    QImage rotatedImage = currentImage.transformed(transform, Qt::SmoothTransformation);

    imageLabel->setCurrentImage(rotatedImage);
    imageLabel->setOriginalImage(rotatedImage);

    imageLabel->resize(rotatedImage.size());
    imageLabel->update();
    imageLabel->updateScaleLabel(mainWindow->getScaleLabel());
}
