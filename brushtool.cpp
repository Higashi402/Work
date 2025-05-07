#include "brushtool.h"

BrushTool::BrushTool(QObject* parent) : QObject(parent), currentBrushColor(Qt::black) {}

void BrushTool::setBrushColor(const QColor& color) {
    if (color != currentBrushColor) {
        currentBrushColor = color;
        emit brushColorChanged(color);
    }
}

QColor BrushTool::brushColor() const {
    return currentBrushColor;
}
