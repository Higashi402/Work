#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    qApp->setStyleSheet(R"(
    QMainWindow {
        background-color: #000000;
    }
    QMdiArea {
        background-color: #2b2b2b;
    }

    QLabel,QMenuBar, QAction, QToolBar, QPushButton {
        color: #dddddd;
        background-color: #3c3f41;
    }
    QScrollBar:vertical, QScrollBar:horizontal {
        background: #2b2b2b;
    }
    QScrollBar::handle {
        background: #5a5a5a;
    }
    QTabBar::tab {
       background: #3c3f41;
      color: #dddddd;
      padding: 5px 10px;
     border: 1px solid #5a5a5a;
    }

    QTabBar::tab:selected {
        background: #5a5a5a;
       color: white;
       border-bottom: 2px solid #bbbbbb;
    }

    QTabBar::tab:hover {
        background: #4c4f51;
    }
    QLineEdit, QSpinBox {
        background-color: #3c3f41;
        color: #dddddd;
        border: 1px solid #5a5a5a;
    }

    QMenu {
        background-color: #2b2b2b;
        color: #dddddd;
    }

    QMenu::item {
        background-color: transparent;
        padding: 5px 20px;
    }

    QMenu::item:selected {
        background-color: #444444; /* или #000000, если нужен полностью чёрный */
        color: white;
    }

    QMenuBar {
        background-color: #3c3f41;
        color: #dddddd;
    }

    QMenuBar::item {
        background-color: transparent;
        padding: 4px 10px;
    }

    QMenuBar::item:selected {
       background-color: #444444; /* цвет при наведении */
       color: white;
    }

    QMenuBar::item:pressed {
        background-color: #2b2b2b; /* цвет при нажатии */
    }

    QMdiSubWindow {
       background-color: #2b2b2b;
       border: 1px solid #5a5a5a;
    }

    QScrollArea {
       background-color: #2b2b2b;
    }

    QDialog {
        background-color: #2b2b2b;
        color: #dddddd;
    }
    QLabel, QSpinBox, QPushButton {
        background-color: #3c3f41;
        color: #dddddd;
        border: 1px solid #5a5a5a;
    }

    QDockWidget {
        background-color: #2b2b2b;
        border: none;
        titlebar-close-icon: none;
        titlebar-normal-icon: none;
    }

    QListWidget, QLineEdit, QLabel, QPushButton, QSlider {
            color: white;
        }





    QTabBar::tab {
        background: #3c3f41;
        color: #dddddd;
        padding: 6px;
    }

    QTabBar::tab:selected {
        background: #555555;
        font-weight: bold;
    }

    QLabel {
        color: #dddddd;
    }

    QWidget {
        background-color: #2b2b2b;
    }

QLabel, QCheckBox, QGroupBox, QRadioButton, QSpinBox, QDoubleSpinBox, QComboBox {
        color: white;
    }

    QGroupBox::title {
        color: white;
    }

)");

    w.show();
    return a.exec();
}
