#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMdiArea>
#include <QMenuBar>
#include <QAction>
#include <QStatusBar>
#include <QLabel>
#include <QMouseEvent>
#include <QScrollArea>
#include <QImage>
#include <QSpinBox>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ImageLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openImageFile();
    void applyAutoContrast();
    void zoomIn();
    void zoomOut();
    void updateActiveImageControls();

private:
    Ui::MainWindow *ui;
    QMdiArea *mdiArea;
    QLabel *statusLabel;

    // Элементы управления
    QSpinBox *leftClipSpinBox;
    QSpinBox *rightClipSpinBox;
    QLabel *scaleLabel;

    void setupStatusBar();
    void setupToolBar();
    QImage openBMPFile(char *fileNamePath);
    QImage openTIFFFile(const char *fileNamePath);
    void displayImage(const QImage &image, const QString &fileName);

    ImageLabel* getActiveImageLabel() const;
};

#endif // MAINWINDOW_H
