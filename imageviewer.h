#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QImage>
#include <QMouseEvent>
#include <QString>
#include "bmp_handler.h" // Подключаем ваш заголовочный файл для работы с BMP

class ImageViewer : public QWidget
{
    Q_OBJECT

public:
    ImageViewer(const QString &filePath, QWidget *parent = nullptr);
    bool isValid() const;

protected:
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QLabel *imageLabel;
    QScrollArea *scrollArea;
    QImage image;
    bool valid;

    void loadTiffImage(const QString &filePath);
    void loadBmpImage(const QString &filePath);
};

#endif // IMAGEVIEWER_H
