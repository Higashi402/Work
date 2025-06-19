#ifndef SOBELDIALOG_H
#define SOBELDIALOG_H

#include <QDialog>
#include <QImage>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include "q_image.h"

class SobelFilterDialog : public QDialog {
    Q_OBJECT

public:
    explicit SobelFilterDialog(ImageLabel* target, QWidget* parent = nullptr);

private slots:
    void previewFilter();
    void applyFilter();
    void updatePreview();

private:
    void initUI();
    QImage applySobel(const QImage& image);

    ImageLabel* imageLabel;
    QImage originalImage;
    QImage previewImage;

    // Параметры фильтра
    QComboBox* directionCombo;
    QComboBox* colorModeCombo;
    QDoubleSpinBox* thresholdSpin;

    // Элементы интерфейса
    QLabel* previewLabel;
    QPushButton* applyButton;
    QPushButton* cancelButton;
    QPushButton* previewButton;
};

#endif // SOBELDIALOG_H
