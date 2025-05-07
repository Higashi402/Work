#ifndef BRUSHTOOL_H
#define BRUSHTOOL_H

#include <QLabel>
#include <QImage>

class BrushTool : public QObject {
    Q_OBJECT

public:
    explicit BrushTool(QObject* parent = nullptr);
    void setBrushColor(const QColor& color);
    QColor brushColor() const;

signals:
    void brushColorChanged(const QColor& color);

private:
    QColor currentBrushColor;
};


#endif // BRUSHTOOL_H
