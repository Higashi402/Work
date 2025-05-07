#ifndef FILEMENUHANDLER_H
#define FILEMENUHANDLER_H

#include <QObject>

class MainWindow;

class FileMenuHandler : public QObject {
    Q_OBJECT

public:
    explicit FileMenuHandler(QObject* parent = nullptr);

    static void openImageFile(MainWindow* mainWindow);
    static QImage openBMPFile(char *fileNamePath);
    static QImage openTIFFFile(const char *fileNamePath);
    static void saveImageFile(MainWindow* mainWindow);

};

#endif // FILEMENUHANDLER_H
