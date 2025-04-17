#include "mainwindow.h"
#include "q_image.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QImage>
#include <QPixmap>
#include <QMdiSubWindow>
#include "tiffio.h"
#include "bmp_handler.h"
#include <QScrollArea>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QToolBar>
#include <QIcon>
#include <QSpinBox>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    mdiArea = new QMdiArea(this);
    layout->addWidget(mdiArea);

    setCentralWidget(centralWidget);

    setupToolBar();
    setupStatusBar();

    QMenu *fileMenu = menuBar()->addMenu("Файл");
    QAction *openAction = fileMenu->addAction("Открыть изображение");
    connect(openAction, &QAction::triggered, this, &MainWindow::openImageFile);

    // Обновляем элементы управления при смене активного окна
    connect(mdiArea, &QMdiArea::subWindowActivated, this, &MainWindow::updateActiveImageControls);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupStatusBar() {
    statusLabel = new QLabel(this);
    statusBar()->addPermanentWidget(statusLabel);
    statusLabel->setText("Готово");
}

void MainWindow::setupToolBar()
{
    QToolBar *toolBar = addToolBar("Инструменты");
    toolBar->setMovable(false);

    // Добавляем элементы управления автоконтрастом
    toolBar->addWidget(new QLabel("Обрезка слева (%):"));
    leftClipSpinBox = new QSpinBox(toolBar);
    leftClipSpinBox->setRange(0, 100);
    leftClipSpinBox->setValue(1);
    toolBar->addWidget(leftClipSpinBox);

    toolBar->addWidget(new QLabel("Обрезка справа (%):"));
    rightClipSpinBox = new QSpinBox(toolBar);
    rightClipSpinBox->setRange(0, 100);
    rightClipSpinBox->setValue(1);
    toolBar->addWidget(rightClipSpinBox);

    // Кнопка применения автоконтраста
    QAction *applyContrastAction = toolBar->addAction("Применить автоконтраст");
    connect(applyContrastAction, &QAction::triggered, this, &MainWindow::applyAutoContrast);

    // Добавляем масштабирование
    toolBar->addSeparator();
    QAction *zoomInAction = toolBar->addAction(QIcon::fromTheme("zoom-in"), "Увеличить");
    connect(zoomInAction, &QAction::triggered, this, &MainWindow::zoomIn);

    QAction *zoomOutAction = toolBar->addAction(QIcon::fromTheme("zoom-out"), "Уменьшить");
    connect(zoomOutAction, &QAction::triggered, this, &MainWindow::zoomOut);

    scaleLabel = new QLabel("Масштаб: 100%", toolBar);
    toolBar->addWidget(scaleLabel);
}

ImageLabel* MainWindow::getActiveImageLabel() const
{
    if (QMdiSubWindow *activeSubWindow = mdiArea->activeSubWindow()) {
        if (QScrollArea *scrollArea = qobject_cast<QScrollArea*>(activeSubWindow->widget())) {
            return qobject_cast<ImageLabel*>(scrollArea->widget());
        }
    }
    return nullptr;
}

void MainWindow::updateActiveImageControls()
{
    ImageLabel *imageLabel = getActiveImageLabel();
    if (imageLabel) {
        imageLabel->updateScaleLabel(scaleLabel);
    }
}

void MainWindow::applyAutoContrast()
{
    if (ImageLabel *imageLabel = getActiveImageLabel()) {
        imageLabel->applyAutoContrast(leftClipSpinBox->value(), rightClipSpinBox->value());
    }
}

void MainWindow::zoomIn()
{
    if (ImageLabel *imageLabel = getActiveImageLabel()) {
        imageLabel->zoomIn();
        imageLabel->updateScaleLabel(scaleLabel);
    }
}

void MainWindow::zoomOut()
{
    if (ImageLabel *imageLabel = getActiveImageLabel()) {
        imageLabel->zoomOut();
        imageLabel->updateScaleLabel(scaleLabel);
    }
}

void MainWindow::openImageFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Открыть файл", "C:/Users/Higashi/Desktop/Photon/Images", "Все файлы (*.*)");

    if (fileName.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Файл не выбран.");
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
        QMessageBox::warning(this, "Ошибка", "Неподдерживаемый формат файла.");
        return;
    }

    if (image.isNull()) {
        QMessageBox::warning(this, "Ошибка", "Не удалось загрузить изображение.");
        return;
    }

    displayImage(image, fileName);
}

QImage MainWindow::openBMPFile(char *fileNamePath) {
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

QImage MainWindow::openTIFFFile(const char *fileNamePath) {
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

void MainWindow::displayImage(const QImage &image, const QString &fileName) {
    QScrollArea *scrollArea = new QScrollArea;
    ImageLabel *imageLabel = new ImageLabel(image, statusLabel, scrollArea, this);

    scrollArea->setWidget(imageLabel);
    scrollArea->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    QMdiSubWindow *subWindow = mdiArea->addSubWindow(scrollArea);
    subWindow->setWindowTitle(fileName);
    subWindow->resize(800, 600);
    subWindow->show();

    QToolBar *toolBar = new QToolBar(subWindow);
    toolBar->setMovable(false);

    /*
    // Добавляем QSpinBox для обрезки слева
    QLabel *leftClipLabel = new QLabel("Обрезка слева (%):", toolBar);
    QSpinBox *leftClipSpinBox = new QSpinBox(toolBar);
    leftClipSpinBox->setRange(0, 100);
    leftClipSpinBox->setValue(1);

    // Добавляем QSpinBox для обрезки справа
    QLabel *rightClipLabel = new QLabel("Обрезка справа (%):", toolBar);
    QSpinBox *rightClipSpinBox = new QSpinBox(toolBar);
    rightClipSpinBox->setRange(0, 100);
    rightClipSpinBox->setValue(1);

    toolBar->addWidget(leftClipLabel);
    toolBar->addWidget(leftClipSpinBox);
    toolBar->addWidget(rightClipLabel);
    toolBar->addWidget(rightClipSpinBox);

    connect(leftClipSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [imageLabel, leftClipSpinBox, rightClipSpinBox]() {
        imageLabel->applyAutoContrast(leftClipSpinBox->value(), rightClipSpinBox->value());
    });

    connect(rightClipSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [imageLabel, leftClipSpinBox, rightClipSpinBox]() {
        imageLabel->applyAutoContrast(leftClipSpinBox->value(), rightClipSpinBox->value());
    });

    QLabel *scaleLabel = new QLabel("Масштаб: 100%", toolBar);

    QAction *zoomInAction = toolBar->addAction(QIcon::fromTheme("zoom-in"), "Увеличить");
    connect(zoomInAction, &QAction::triggered, [=]() {
        imageLabel->zoomIn();
        imageLabel->updateScaleLabel(scaleLabel);
        scrollArea->updateGeometry();
    });

    QAction *zoomOutAction = toolBar->addAction(QIcon::fromTheme("zoom-out"), "Уменьшить");
    connect(zoomOutAction, &QAction::triggered, [=]() {
        imageLabel->zoomOut();
        imageLabel->updateScaleLabel(scaleLabel);
        scrollArea->updateGeometry();
    });

    toolBar->addWidget(scaleLabel); */
    subWindow->layout()->setMenuBar(toolBar);

    imageLabel->update();
    imageLabel->resize(imageLabel->currentImage.size() * 1);
}
