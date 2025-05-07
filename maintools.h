#ifndef MAINTOOLS_H
#define MAINTOOLS_H

#include <QObject>

class MainWindow;

class MainTools : public QObject {
    Q_OBJECT

public:
    explicit MainTools(QObject* parent = nullptr);

    void zoomIn(MainWindow* mainWindow);
    void zoomOut(MainWindow* mainWindow);
    void rotateImage(MainWindow* mainWindow);
};
#endif // MAINTOOLS_H
