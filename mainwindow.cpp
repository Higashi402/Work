#include <QMessageBox>
#include <QFileDialog>
#include <QImage>
#include <QPixmap>
#include <QMdiSubWindow>
#include <QScrollArea>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QToolBar>
#include <QIcon>
#include <QSpinBox>
#include <QDockWidget>
#include <QColorDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QListWidget>
#include <cmath>

#include "mainwindow.h"
#include "q_image.h"
#include "./ui_mainwindow.h"
#include "bmp_handler.h"
#include "autocontrastdialog.h"
#include "medianfilterdialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), brushTool(new BrushTool(this))
{
    ui->setupUi(this);

    mainTools = new MainTools(this);
    fileMenuHandler = new FileMenuHandler();

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    mdiArea = new QMdiArea(this);
    mdiArea->setViewMode(QMdiArea::TabbedView);
    layout->addWidget(mdiArea);

    setCentralWidget(centralWidget);

    setupSideTabDock();
    setupToolBar();
    setupStatusBar();

    QMenu *fileMenu = menuBar()->addMenu("Файл");
    QAction *openAction = fileMenu->addAction("Открыть");
    QAction *saveAction = fileMenu->addAction("Сохранить");
    fileMenu->addAction("Создать");
    fileMenu->addAction("Выход");
    menuBar()->addMenu("Вид");
    menuBar()->addMenu("Выделение");
    QMenu *toolsMenu = menuBar()->addMenu("Инструменты");
    QAction* autoContrastAction = toolsMenu->addAction("Автоконтрастирование");
    QMenu *filterMenu = menuBar()->addMenu("Фильтры");
    filterMenu->addAction("Фильтр Гаусса");
    QAction* medianFilterAction = filterMenu->addAction("Медианная фильтрация");
    filterMenu->addAction("Срединная фильтрация");
    filterMenu->addAction("Нелинейный фильтр");
    menuBar()->addMenu("Справка");

    connect(openAction, &QAction::triggered, this, [=]() {
        fileMenuHandler->openImageFile(this);
    });
    connect(saveAction, &QAction::triggered, this, [=]() {
        fileMenuHandler->saveImageFile(this);
    });
    connect(mdiArea, &QMdiArea::subWindowActivated, this, &MainWindow::updateActiveImageControls);

    connect(autoContrastAction, &QAction::triggered, this, &MainWindow::openAutocontrastDialog);
    connect(medianFilterAction, &QAction::triggered, this, &MainWindow::openMedianFilterDialog);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupStatusBar() {
    scaleLabel = new QLabel(this);
    scaleLabel->setText("Масштаб: 100%");
    statusBar()->addWidget(scaleLabel);
    statusLabel = new QLabel(this);
    statusLabel->setText("Готово");
    statusBar()->addPermanentWidget(statusLabel);
}

void MainWindow::setupSideTabDock()
{
    QDockWidget *tabDock = new QDockWidget("Панель", this);
    tabDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QTabWidget *tabWidget = new QTabWidget(tabDock);

    // ----- Вкладка "Слои"
    QWidget *layersTab = new QWidget;
    QVBoxLayout *layersLayout = new QVBoxLayout(layersTab);

    // Заголовок
    QLabel *layersTitle = new QLabel("Слои");
    layersTitle->setStyleSheet("font-weight: bold; font-size: 16px; color: white;");
    layersLayout->addWidget(layersTitle);

    // Список слоёв с миниатюрами
    QListWidget *layerList = new QListWidget;
    layerList->setStyleSheet(R"(
    QListWidget {
        background-color: #2e2e2e;
        border: 1px solid #555;
        color: white;
    }
    QListWidget::item:selected {
        background-color: #505050;
    }
)");
    layerList->setIconSize(QSize(32, 32));
    layerList->addItem("Фон");
    layerList->addItem("Слой 1");
    layerList->addItem("Слой 2 (кисть)");
    layersLayout->addWidget(layerList);

    // Кнопки управления слоями
    QHBoxLayout *layerButtonsLayout = new QHBoxLayout;
    QPushButton *addLayerButton = new QPushButton(QIcon::fromTheme("list-add"), "");
    QPushButton *deleteLayerButton = new QPushButton(QIcon::fromTheme("list-remove"), "");
    QPushButton *moveUpButton = new QPushButton(QIcon::fromTheme("go-up"), "");
    QPushButton *moveDownButton = new QPushButton(QIcon::fromTheme("go-down"), "");

    QString buttonStyle = R"(
    QPushButton {
        background-color: #444;
        border: none;
        padding: 4px;
    }
    QPushButton:hover {
        background-color: #666;
    }
)";
    for (QPushButton *btn : {addLayerButton, deleteLayerButton, moveUpButton, moveDownButton})
        btn->setStyleSheet(buttonStyle);

    layerButtonsLayout->addWidget(addLayerButton);
    layerButtonsLayout->addWidget(deleteLayerButton);
    layerButtonsLayout->addWidget(moveUpButton);
    layerButtonsLayout->addWidget(moveDownButton);
    layersLayout->addLayout(layerButtonsLayout);

    // Секция редактирования слоя
    QGroupBox *editGroup = new QGroupBox("Параметры слоя");
    editGroup->setStyleSheet("QGroupBox { color: white; border: 1px solid gray; }");
    QVBoxLayout *editLayout = new QVBoxLayout(editGroup);
    editLayout->setContentsMargins(10, 20, 10, 10);

    // Имя слоя
    QHBoxLayout *nameLayout = new QHBoxLayout;
    nameLayout->addWidget(new QLabel("Имя:"));
    QLineEdit *layerNameEdit = new QLineEdit("Слой 1");
    layerNameEdit->setStyleSheet("background-color: #444; color: white;");
    nameLayout->addWidget(layerNameEdit);
    editLayout->addLayout(nameLayout);

    // Режим наложения
    QHBoxLayout *blendLayout = new QHBoxLayout;
    blendLayout->addWidget(new QLabel("Режим:"));
    QComboBox *blendModeCombo = new QComboBox;
    blendModeCombo->addItems({"Normal", "Multiply", "Overlay", "Screen", "Difference"});
    blendModeCombo->setStyleSheet("background-color: #444; color: white;");
    blendLayout->addWidget(blendModeCombo);
    editLayout->addLayout(blendLayout);

    // Непрозрачность
    QHBoxLayout *opacityLayout = new QHBoxLayout;
    opacityLayout->addWidget(new QLabel("Непрозрачность:"));
    QSlider *opacitySlider = new QSlider(Qt::Horizontal);
    opacitySlider->setRange(0, 100);
    opacitySlider->setValue(100);
    opacityLayout->addWidget(opacitySlider);
    editLayout->addLayout(opacityLayout);

    // Переключатели
    QHBoxLayout *toggleLayout = new QHBoxLayout;
    QCheckBox *visibleCheck = new QCheckBox("Видим");
    QCheckBox *lockedCheck = new QCheckBox("Заблокирован");
    visibleCheck->setChecked(true);
    for (QCheckBox *chk : {visibleCheck, lockedCheck})
        chk->setStyleSheet("color: white;");
    toggleLayout->addWidget(visibleCheck);
    toggleLayout->addWidget(lockedCheck);
    editLayout->addLayout(toggleLayout);

    // Цвет слоя
    QHBoxLayout *colorLayout = new QHBoxLayout;
    colorLayout->addWidget(new QLabel("Цвет слоя:"));
    QPushButton *colorBtn = new QPushButton;
    colorBtn->setFixedSize(24, 24);
    colorBtn->setStyleSheet("background-color: red; border: 1px solid #ccc;");
    colorLayout->addWidget(colorBtn);
    editLayout->addLayout(colorLayout);

    // Маска слоя
    QCheckBox *maskCheck = new QCheckBox("Маска слоя");
    maskCheck->setStyleSheet("color: white;");
    editLayout->addWidget(maskCheck);

    // Добавляем секцию редактирования в общий макет
    layersLayout->addWidget(editGroup);
    tabWidget->addTab(layersTab, QIcon::fromTheme("layer-visible-on"), "Слои");


    // ----- Вкладка "История"
    QWidget *historyTab = new QWidget;
    QVBoxLayout *historyLayout = new QVBoxLayout(historyTab);

    QLabel *title = new QLabel("История действий");
    title->setStyleSheet("font-weight: bold; font-size: 16px;");
    historyLayout->addWidget(title);

    // Список истории
    QListWidget *historyList = new QListWidget;
    historyList->addItems({
        "Создание нового холста",
        "Добавлен слой: Фон",
        "Инструмент: Кисть",
        "Применён фильтр: Размытие",
        "Изменение параметров слоя",
        "Слой скрыт",
        "Удалена маска"
    });
    historyLayout->addWidget(historyList);

    // Кнопки управления
    QHBoxLayout *historyButtonsLayout = new QHBoxLayout;
    QPushButton *undoButton = new QPushButton("Отменить");
    QPushButton *redoButton = new QPushButton("Повторить");
    QPushButton *clearButton = new QPushButton("Очистить историю");
    undoButton->setIcon(QIcon::fromTheme("edit-undo"));
    redoButton->setIcon(QIcon::fromTheme("edit-redo"));
    clearButton->setIcon(QIcon::fromTheme("edit-delete"));
    historyButtonsLayout->addWidget(undoButton);
    historyButtonsLayout->addWidget(redoButton);
    historyButtonsLayout->addWidget(clearButton);
    historyLayout->addLayout(historyButtonsLayout);

    // Панель фильтрации истории
    QGroupBox *filterGroup = new QGroupBox("Фильтр");
    QFormLayout *filterLayout = new QFormLayout(filterGroup);
    QComboBox *actionTypeCombo = new QComboBox;
    actionTypeCombo->addItems({"Все действия", "Кисть", "Фильтры", "Слои"});
    filterLayout->addRow("Тип действия:", actionTypeCombo);
    historyLayout->addWidget(filterGroup);

    // Информация о действии
    QGroupBox *detailsGroup = new QGroupBox("Информация о действии");
    QFormLayout *detailsLayout = new QFormLayout(detailsGroup);
    detailsLayout->addRow("Имя действия:", new QLabel("Применён фильтр: Размытие"));
    detailsLayout->addRow("Время:", new QLabel("12:45:33"));
    detailsLayout->addRow("Автор:", new QLabel("Пользователь"));
    historyLayout->addWidget(detailsGroup);

    // Маски истории
    QGroupBox *historyMaskGroup = new QGroupBox("Параметры маски истории");
    QVBoxLayout *maskLayout = new QVBoxLayout(historyMaskGroup);
    maskLayout->addWidget(new QCheckBox("Отображать только действия с текущим слоем"));
    maskLayout->addWidget(new QCheckBox("Группировать по типу действия"));
    maskLayout->addWidget(new QCheckBox("Показывать системные действия"));
    historyLayout->addWidget(historyMaskGroup);

    // Пространство
    historyLayout->addStretch();

    tabWidget->addTab(historyTab, QIcon::fromTheme("edit-undo"), "История");


    // ----- Вкладка "Свойства"
    // ----- Вкладка "Свойства" (Professional Edition)
    QWidget *propertiesTab = new QWidget;
    QVBoxLayout *propertiesLayout = new QVBoxLayout(propertiesTab);
    propertiesLayout->setContentsMargins(5, 5, 5, 5);
    propertiesLayout->setSpacing(8);

    // Стиль для профессионального вида
    QString professionalStyle = R"(
    QGroupBox {
        color: #f0f0f0;
        border: 1px solid #444;
        border-radius: 3px;
        margin-top: 10px;
        padding-top: 15px;
    }
    QGroupBox::title {
        subcontrol-origin: margin;
        left: 10px;
        padding: 0 3px;
    }
    QLabel {
        color: #cccccc;
    }
    QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox {
        background-color: #3a3a3a;
        border: 1px solid #555;
        color: #ffffff;
        padding: 3px;
        min-width: 60px;
    }
    QSlider::groove:horizontal {
        height: 4px;
        background: #444;
    }
    QSlider::handle:horizontal {
        width: 12px;
        margin: -4px 0;
        background: #888;
        border-radius: 6px;
    }
    QCheckBox {
        color: #cccccc;
        spacing: 5px;
    }
)";
    propertiesTab->setStyleSheet(professionalStyle);

    // 1. Группа "Основные свойства"
    QGroupBox *basicGroup = new QGroupBox("Основные свойства");
    QVBoxLayout *basicLayout = new QVBoxLayout(basicGroup);

    // Имя документа
    QHBoxLayout *nameLayout1 = new QHBoxLayout;
    QLabel *nameLabel = new QLabel("Имя:");
    nameLabel->setToolTip("Название документа");
    QLineEdit *docNameEdit = new QLineEdit("Безымянный-1.psd");
    nameLayout1->addWidget(nameLabel);
    nameLayout1->addWidget(docNameEdit);
    basicLayout->addLayout(nameLayout1);

    // Размеры документа
    QFormLayout *sizeLayout = new QFormLayout;
    sizeLayout->setHorizontalSpacing(15);
    QDoubleSpinBox *widthSpin = new QDoubleSpinBox;
    widthSpin->setRange(1, 10000);
    widthSpin->setValue(1920);
    widthSpin->setSuffix(" px");
    QDoubleSpinBox *heightSpin = new QDoubleSpinBox;
    heightSpin->setRange(1, 10000);
    heightSpin->setValue(1080);
    heightSpin->setSuffix(" px");
    QComboBox *unitsCombo = new QComboBox;
    unitsCombo->addItems({"пиксели", "дюймы", "см", "мм", "точки", "пики"});
    sizeLayout->addRow("Ширина:", widthSpin);
    sizeLayout->addRow("Высота:", heightSpin);
    sizeLayout->addRow("Единицы:", unitsCombo);
    basicLayout->addLayout(sizeLayout);

    // Разрешение
    QHBoxLayout *resLayout = new QHBoxLayout;
    QLabel *resLabel = new QLabel("Разрешение:");
    QDoubleSpinBox *resSpin = new QDoubleSpinBox;
    resSpin->setRange(1, 10000);
    resSpin->setValue(300);
    resSpin->setSuffix(" ppi");
    QComboBox *resCombo = new QComboBox;
    resCombo->addItems({"пикс./дюйм", "пикс./см"});
    resLayout->addWidget(resLabel);
    resLayout->addWidget(resSpin);
    resLayout->addWidget(resCombo);
    basicLayout->addLayout(resLayout);

    // Цветовой профиль
    QHBoxLayout *profileLayout = new QHBoxLayout;
    QLabel *profileLabel = new QLabel("Профиль:");
    QComboBox *profileCombo = new QComboBox;
    profileCombo->addItems({"sRGB IEC61966-2.1", "Adobe RGB (1998)", "ProPhoto RGB", "Custom..."});
    profileCombo->setToolTip("Цветовой профиль документа");
    profileLayout->addWidget(profileLabel);
    profileLayout->addWidget(profileCombo);
    basicLayout->addLayout(profileLayout);

    propertiesLayout->addWidget(basicGroup);

    // 2. Группа "Слой"
    QGroupBox *layerGroup = new QGroupBox("Текущий слой");
    QVBoxLayout *layerLayout = new QVBoxLayout(layerGroup);

    // Имя слоя и видимость
    QHBoxLayout *layerNameLayout = new QHBoxLayout;
    QLineEdit *layerNameEdit1 = new QLineEdit("Слой 1");
    QCheckBox *visibleCheck1 = new QCheckBox;
    visibleCheck1->setChecked(true);
    visibleCheck1->setToolTip("Видимость слоя");
    layerNameLayout->addWidget(layerNameEdit1);
    layerNameLayout->addWidget(visibleCheck1);
    layerLayout->addLayout(layerNameLayout);

    // Режим наложения и непрозрачность
    QFormLayout *blendLayout1 = new QFormLayout;
    blendLayout1->setHorizontalSpacing(15);
    QComboBox *blendModeCombo1 = new QComboBox;
    blendModeCombo1->addItems({
        "Обычный", "Затемнение", "Умножение", "Затемнение основы", "Линейный затемнитель",
        "Осветление", "Экран", "Осветление основы", "Линейный осветлитель", "Перекрытие",
        "Мягкий свет", "Жесткий свет", "Яркий свет", "Линейный свет", "Точечный свет",
        "Жесткое смешение", "Разница", "Исключение", "Вычитание", "Деление"
    });
    QSlider *opacitySlider1 = new QSlider(Qt::Horizontal);
    opacitySlider1->setRange(0, 100);
    opacitySlider1->setValue(100);
    QSpinBox *opacitySpin = new QSpinBox;
    opacitySpin->setRange(0, 100);
    opacitySpin->setValue(100);
    opacitySpin->setSuffix("%");
    blendLayout1->addRow("Режим:", blendModeCombo1);
    blendLayout1->addRow("Непрозрачность:", opacitySlider1);
    blendLayout1->addRow("", opacitySpin);
    layerLayout->addLayout(blendLayout1);

    // Заливка и блокировка
    QHBoxLayout *fillLockLayout = new QHBoxLayout;
    QLabel *fillLabel = new QLabel("Заливка:");
    QSlider *fillSlider = new QSlider(Qt::Horizontal);
    fillSlider->setRange(0, 100);
    fillSlider->setValue(100);
    QCheckBox *lockPosCheck = new QCheckBox("Поз.");
    lockPosCheck->setToolTip("Блокировка позиции");
    QCheckBox *lockPixCheck = new QCheckBox("Пикс.");
    lockPixCheck->setToolTip("Блокировка пикселей");
    fillLockLayout->addWidget(fillLabel);
    fillLockLayout->addWidget(fillSlider);
    fillLockLayout->addWidget(lockPosCheck);
    fillLockLayout->addWidget(lockPixCheck);
    layerLayout->addLayout(fillLockLayout);

    // Цветовой ярлык слоя
    QHBoxLayout *colorTagLayout = new QHBoxLayout;
    QLabel *colorTagLabel = new QLabel("Цвет метки:");
    QPushButton *colorTagButtons[7];
    QStringList colorTags = {"Нет", "Красный", "Оранжевый", "Желтый", "Зеленый", "Голубой", "Синий"};
    for (int i = 0; i < 7; ++i) {
        colorTagButtons[i] = new QPushButton;
        colorTagButtons[i]->setFixedSize(16, 16);
        colorTagButtons[i]->setCheckable(true);
        colorTagButtons[i]->setToolTip(colorTags[i]);
        if (i > 0) {
            QString color = i==1?"#ff0000":i==2?"#ff9900":i==3?"#ffff00":i==4?"#00ff00":i==5?"#00ffff":"#0000ff";
            colorTagButtons[i]->setStyleSheet(QString("background-color: %1; border: 1px solid #666;").arg(color));
        } else {
            colorTagButtons[i]->setStyleSheet("background-color: transparent; border: 1px solid #666;");
        }
        colorTagLayout->addWidget(colorTagButtons[i]);
    }
    colorTagButtons[0]->setChecked(true);
    layerLayout->addLayout(colorTagLayout);

    propertiesLayout->addWidget(layerGroup);

    // 3. Группа "Эффекты слоя"
    QGroupBox *effectsGroup = new QGroupBox("Эффекты слоя");
    QVBoxLayout *effectsLayout = new QVBoxLayout(effectsGroup);

    QListWidget *effectsList = new QListWidget;
    effectsList->setIconSize(QSize(16, 16));
    QListWidgetItem *item1 = new QListWidgetItem(QIcon::fromTheme("layer-effects"), "Тень");
    QListWidgetItem *item2 = new QListWidgetItem(QIcon::fromTheme("layer-effects"), "Внешнее свечение");
    QListWidgetItem *item3 = new QListWidgetItem(QIcon::fromTheme("layer-effects"), "Наложение градиента");
    effectsList->addItem(item1);
    effectsList->addItem(item2);
    effectsList->addItem(item3);
    effectsList->setStyleSheet("QListWidget { border: 1px solid #444; background: #2a2a2a; }");
    effectsLayout->addWidget(effectsList);

    QPushButton *addEffectBtn = new QPushButton("Добавить эффект...");
    addEffectBtn->setStyleSheet("QPushButton { padding: 3px; }");
    effectsLayout->addWidget(addEffectBtn);

    propertiesLayout->addWidget(effectsGroup);

    // 4. Группа "Маски"
    QGroupBox *maskGroup = new QGroupBox("Маски");
    QVBoxLayout *maskLayout1 = new QVBoxLayout(maskGroup);

    // Миниатюра маски
    QLabel *maskThumbnail = new QLabel;
    maskThumbnail->setFixedSize(64, 64);
    maskThumbnail->setStyleSheet("background-color: #333; border: 1px solid #555;");
    maskThumbnail->setAlignment(Qt::AlignCenter);
    maskThumbnail->setText("Маска не создана");
    maskLayout1->addWidget(maskThumbnail, 0, Qt::AlignHCenter);

    // Плотность и растушевка
    QFormLayout *maskParamsLayout = new QFormLayout;
    maskParamsLayout->setHorizontalSpacing(15);
    QSlider *densitySlider = new QSlider(Qt::Horizontal);
    densitySlider->setRange(0, 100);
    densitySlider->setValue(100);
    QSlider *featherSlider = new QSlider(Qt::Horizontal);
    featherSlider->setRange(0, 250);
    featherSlider->setValue(0);
    maskParamsLayout->addRow("Плотность:", densitySlider);
    maskParamsLayout->addRow("Растушевка:", featherSlider);
    maskLayout->addLayout(maskParamsLayout);

    // Кнопки управления маской
    QHBoxLayout *maskButtonsLayout = new QHBoxLayout;
    QPushButton *addMaskBtn = new QPushButton("Добавить");
    QPushButton *delMaskBtn = new QPushButton("Удалить");
    QPushButton *editMaskBtn = new QPushButton("Редактировать");
    maskButtonsLayout->addWidget(addMaskBtn);
    maskButtonsLayout->addWidget(delMaskBtn);
    maskButtonsLayout->addWidget(editMaskBtn);
    maskLayout->addLayout(maskButtonsLayout);

    propertiesLayout->addWidget(maskGroup);

    // 5. Группа "Каналы"
    QGroupBox *channelsGroup = new QGroupBox("Каналы");
    QVBoxLayout *channelsLayout = new QVBoxLayout(channelsGroup);

    QListWidget *channelsList = new QListWidget;
    channelsList->setStyleSheet("QListWidget { border: 1px solid #444; background: #2a2a2a; }");
    QListWidgetItem *rgbItem = new QListWidgetItem("RGB");
    rgbItem->setCheckState(Qt::Checked);
    QListWidgetItem *rItem = new QListWidgetItem("Красный");
    rItem->setCheckState(Qt::Checked);
    QListWidgetItem *gItem = new QListWidgetItem("Зеленый");
    gItem->setCheckState(Qt::Checked);
    QListWidgetItem *bItem = new QListWidgetItem("Синий");
    bItem->setCheckState(Qt::Checked);
    QListWidgetItem *alphaItem = new QListWidgetItem("Альфа");
    alphaItem->setCheckState(Qt::Checked);
    channelsList->addItem(rgbItem);
    channelsList->addItem(rItem);
    channelsList->addItem(gItem);
    channelsList->addItem(bItem);
    channelsList->addItem(alphaItem);
    channelsLayout->addWidget(channelsList);

    propertiesLayout->addWidget(channelsGroup);

    // Добавляем вкладку в табвиджет
    tabWidget->addTab(propertiesTab, QIcon::fromTheme("preferences-system"), "Свойства");

    // Добавление табов в док-виджет
    tabDock->setWidget(tabWidget);
    addDockWidget(Qt::RightDockWidgetArea, tabDock);
}


void MainWindow::setupToolBar()
{
    QToolBar *toolBar = addToolBar("Инструменты");
    toolBar->setOrientation(Qt::Vertical);
    toolBar->setMovable(false);

    toolBar->addSeparator();
    QAction *zoomInAction = toolBar->addAction(QIcon(":/main-tools/img/scale+.png"), "Увеличение");
    connect(zoomInAction, &QAction::triggered, this, [=]() {
        mainTools->zoomIn(this);
    });

    QAction *zoomOutAction = toolBar->addAction(QIcon(":/main-tools/img/scale-.png"), "Уменьшение");
    connect(zoomOutAction, &QAction::triggered, this, [=]() {
        mainTools->zoomOut(this);
    });

    QAction *fillAction = toolBar->addAction(QIcon(":/main-tools/img/filling.png"), "Заливка");
    //connect(fillAction, &QAction::triggered, this, &MainWindow::activateFillTool);

    QAction *brushAction = toolBar->addAction(QIcon(":/main-tools/img/brush.png"), "Кисть");
    //connect(brushAction, &QAction::triggered, this, &MainWindow::activateBrushTool);

    QAction *textAction = toolBar->addAction(QIcon(":/main-tools/img/text.png"), "Текст");
    //connect(textAction, &QAction::triggered, this, &MainWindow::activateTextTool);

    QAction *rotateAction = toolBar->addAction(QIcon(":/main-tools/img/rotate.png"), "Вращение");
    connect(rotateAction, &QAction::triggered, this, [=]() {
        mainTools->rotateImage(this);
    });

    QAction *moveAction = toolBar->addAction(QIcon(":/main-tools/img/move.png"), "Перемещение");
    //connect(textAction, &QAction::triggered, this, &MainWindow::activateTextTool);

    QAction *eraseAction = toolBar->addAction(QIcon(":/main-tools/img/eraser.png"), "Ластик");
    //connect(textAction, &QAction::triggered, this, &MainWindow::activateTextTool);

    QPushButton* colorIndicator = new QPushButton;
    colorIndicator->setFixedSize(30, 30);
    colorIndicator->setStyleSheet("background-color: black; border: 1px solid gray;");
    toolBar->addWidget(colorIndicator);

    connect(colorIndicator, &QPushButton::clicked, this, [=]() {
        QColor selectedColor = QColorDialog::getColor(brushTool->brushColor(), this, "Выберите цвет кисти");
        if (selectedColor.isValid()) {
            brushTool->setBrushColor(selectedColor);
        }
    });

    connect(brushTool, &BrushTool::brushColorChanged, this, [=](const QColor& color) {
        colorIndicator->setStyleSheet(QString("background-color: %1; border: 1px solid gray;").arg(color.name()));
    });


    scaleLabel = new QLabel("Масштаб: 100%");
    addToolBar(Qt::LeftToolBarArea, toolBar);
}

ImageLabel* MainWindow::getActiveImageLabel() const
{
    if (QMdiSubWindow *activeSubWindow = mdiArea->activeSubWindow()) {
        if (QScrollArea *scrollArea = qobject_cast<QScrollArea*>(activeSubWindow->widget())) {
            return qobject_cast<ImageLabel*>(scrollArea->widget());
        }
    }
    return nullptr;
}

void MainWindow::updateActiveImageControls()
{
    ImageLabel *imageLabel = getActiveImageLabel();
    if (imageLabel) {
        imageLabel->updateScaleLabel(scaleLabel);
    }
}

void MainWindow::openAutocontrastDialog() {
    auto subWindow = mdiArea->activeSubWindow();
    if (!subWindow) return;

    auto scrollArea = qobject_cast<QScrollArea*>(subWindow->widget());
    if (!scrollArea) return;

    auto imageLabel = scrollArea->findChild<ImageLabel*>();
    if (!imageLabel) return;

    AutocontrastDialog dialog(imageLabel, this);
    dialog.exec();
}

void MainWindow::openMedianFilterDialog() {
    ImageLabel* imageLabel = getActiveImageLabel();
    if (!imageLabel) {
        QMessageBox::warning(this, "Ошибка", "Нет активного изображения.");
        return;
    }

    MedianFilterDialog dialog(imageLabel,this);
    dialog.exec();
}

void MainWindow::displayImage(const QImage &image, const QString &fileName) {
    QScrollArea *scrollArea = new QScrollArea;
    ImageLabel *imageLabel = new ImageLabel(image, statusLabel, scrollArea, this);

    scrollArea->setWidget(imageLabel);
    scrollArea->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    QMdiSubWindow *subWindow = mdiArea->addSubWindow(scrollArea);
    subWindow->setWindowTitle(fileName);
    subWindow->resize(800, 600);
    subWindow->show();

    QToolBar *toolBar = new QToolBar(subWindow);
    toolBar->setMovable(false);

    subWindow->layout()->setMenuBar(toolBar);

    imageLabel->update();
    imageLabel->resize(imageLabel->currentImage.size() * 1);
}
