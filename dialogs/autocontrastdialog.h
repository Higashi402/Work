#ifndef AUTOCONTRASTDIALOG_H
#define AUTOCONTRASTDIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include <QSlider>
#include "q_image.h"

class AutocontrastDialog : public QDialog {
    Q_OBJECT

public:
    AutocontrastDialog(ImageLabel* target, QWidget* parent = nullptr);

private slots:
    void applyAutocontrast();

private:
    ImageLabel* imageLabel;
    QSpinBox* leftSpin;
    QSpinBox* rightSpin;
    QCheckBox* separateChannelsCheck;
    QSlider* contrastStrengthSlider;
};


#endif // AUTOCONTRASTDIALOG_H
