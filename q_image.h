#ifndef Q_IMAGE_H
#define Q_IMAGE_H

#include <QLabel>
#include <QImage>
#include <QScrollArea>
#include <QMouseEvent>

class ImageLabel : public QLabel {
    Q_OBJECT

public:
    ImageLabel(const QImage &image, QLabel *statusLabel, QScrollArea *scrollArea, QWidget *parent = nullptr);
    QImage currentImage;

    void zoomIn();
    void zoomOut();
    void updateScaleLabel(QLabel *scaleLabel);

    QImage getCurrentImage() const;
    void setCurrentImage(const QImage &image);
    void applyAutoContrast(int leftClipPercent, int rightClipPercent, bool separateChannels, int strengthPercent);

    void setOriginalImage(const QImage &image);

    void setImage(const QImage& img)
    {
        currentImage = img;
        setPixmap(QPixmap::fromImage(img));
    }

    QImage getOriginalImage(){
        return originalImage;
    }

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QImage originalImage;
    QLabel *statusLabel;
    QScrollArea *scrollArea;
    double scaleFactor = 1.0;
};

#endif // Q_IMAGE_H
