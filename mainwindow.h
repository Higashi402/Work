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

#include "brushtool.h"
#include "maintools.h"
#include "filemenuhandler.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ImageLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ImageLabel* getActiveImageLabel() const;

    void displayImage(const QImage &image, const QString &fileName);

    QLabel* getScaleLabel(){
        return scaleLabel;
    }

    QMdiArea* getMdiArea(){
        return mdiArea;
    }

    ~MainWindow();

private slots:
    void updateActiveImageControls();
    void openAutocontrastDialog();
    void openMedianFilterDialog();


private:
    Ui::MainWindow *ui;
    QMdiArea *mdiArea;
    QLabel *statusLabel;

    QLabel *scaleLabel;

    BrushTool* brushTool;

    void setupStatusBar();
    void setupToolBar();
    void setupSideTabDock();

    MainTools* mainTools;
    FileMenuHandler* fileMenuHandler;
};

#endif // MAINWINDOW_H
