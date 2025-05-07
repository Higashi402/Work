#ifndef MEDIANFILTERDIALOG_H
#define MEDIANFILTERDIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QComboBox>
#include <QGroupBox>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "q_image.h"

class MedianFilterDialog : public QDialog {
    Q_OBJECT

public:
    MedianFilterDialog(ImageLabel* target, QWidget* parent = nullptr);

private slots:
    void applyFilter();
    void previewFilter();
    void updatePreview();

private:
    ImageLabel* imageLabel;
    QImage originalImage;
    QImage previewImage;

    // Параметры фильтра
    QSpinBox* windowSizeSpin;
    QComboBox* borderTypeCombo;
    QComboBox* colorModeCombo;
    QCheckBox* preserveEdgesCheck;
    QDoubleSpinBox* edgeThresholdSpin;
    QCheckBox* adaptiveCheck;
    QSpinBox* iterationsSpin;

    // Элементы предпросмотра
    QLabel* previewLabel;
    QPushButton* applyButton;
    QPushButton* cancelButton;
    QPushButton* previewButton;

    void initUI();
    void applyMedianFilter(QImage& dest, const QImage& src, int windowSize, bool preview=false);
};



#endif // MEDIANFILTERDIALOG_H
