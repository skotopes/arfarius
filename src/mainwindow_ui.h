#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "src/histogramwidget.h"
#include "src/playlistview.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow {
public:
    QAction* actionSave;
    QAction* actionGather;
    QWidget* centralWidget;
    QVBoxLayout* verticalLayout;
    PlayListView* playList;
    QHBoxLayout* panelLayout;
    QGridLayout* controlsLayout;
    QPushButton* playButton;
    QPushButton* nextButton;
    QPushButton* prevButton;
    QLabel* timeLabel;
    HistogramWidget* histogram;
    QMenuBar* menuBar;
    QMenu* menuPlaylist;

    void setupUi(QMainWindow* MainWindow) {
        if(MainWindow->objectName().isEmpty()) MainWindow->setObjectName("MainWindow");
        MainWindow->resize(400, 400);
        MainWindow->setMinimumSize(QSize(400, 400));
        actionSave = new QAction(MainWindow);
        actionSave->setObjectName("actionSave");
        actionGather = new QAction(MainWindow);
        actionGather->setObjectName("actionGather");
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName("centralWidget");
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(0);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        playList = new PlayListView(centralWidget);
        playList->setObjectName("playList");
        playList->setEditTriggers(QAbstractItemView::NoEditTriggers);
        playList->setSelectionBehavior(QAbstractItemView::SelectRows);
        playList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        playList->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        playList->setShowGrid(false);
        playList->setWordWrap(false);
        playList->setCornerButtonEnabled(false);
        playList->horizontalHeader()->setStretchLastSection(true);
        playList->verticalHeader()->setVisible(false);
        playList->verticalHeader()->setMinimumSectionSize(19);
        playList->verticalHeader()->setDefaultSectionSize(19);

        verticalLayout->addWidget(playList);

        panelLayout = new QHBoxLayout();
        panelLayout->setSpacing(0);
        panelLayout->setObjectName("panelLayout");
        panelLayout->setContentsMargins(-1, 1, -1, 1);
        controlsLayout = new QGridLayout();
        controlsLayout->setSpacing(0);
        controlsLayout->setObjectName("controlsLayout");
        controlsLayout->setContentsMargins(-1, -1, -1, 4);
        playButton = new QPushButton(centralWidget);
        playButton->setObjectName("playButton");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(playButton->sizePolicy().hasHeightForWidth());
        playButton->setSizePolicy(sizePolicy);
        playButton->setMaximumSize(QSize(35, 30));
        playButton->setFlat(true);

        controlsLayout->addWidget(playButton, 0, 1, 1, 1);

        nextButton = new QPushButton(centralWidget);
        nextButton->setObjectName("nextButton");
        sizePolicy.setHeightForWidth(nextButton->sizePolicy().hasHeightForWidth());
        nextButton->setSizePolicy(sizePolicy);
        nextButton->setMaximumSize(QSize(35, 30));
        nextButton->setFlat(true);

        controlsLayout->addWidget(nextButton, 0, 2, 1, 1);

        prevButton = new QPushButton(centralWidget);
        prevButton->setObjectName("prevButton");
        sizePolicy.setHeightForWidth(prevButton->sizePolicy().hasHeightForWidth());
        prevButton->setSizePolicy(sizePolicy);
        prevButton->setMaximumSize(QSize(35, 30));
        prevButton->setFlat(true);

        controlsLayout->addWidget(prevButton, 0, 0, 1, 1);

        timeLabel = new QLabel(centralWidget);
        timeLabel->setObjectName("timeLabel");
        QFont font;
        font.setFamilies({QString::fromUtf8("Monaco")});
        font.setPointSize(12);
        timeLabel->setFont(font);
        timeLabel->setAlignment(Qt::AlignCenter);

        controlsLayout->addWidget(timeLabel, 1, 0, 1, 3);

        panelLayout->addLayout(controlsLayout);

        histogram = new HistogramWidget(centralWidget);
        histogram->setObjectName("histogram");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(histogram->sizePolicy().hasHeightForWidth());
        histogram->setSizePolicy(sizePolicy1);

        panelLayout->addWidget(histogram);

        verticalLayout->addLayout(panelLayout);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName("menuBar");
        menuBar->setGeometry(QRect(0, 0, 400, 22));
        menuPlaylist = new QMenu(menuBar);
        menuPlaylist->setObjectName("menuPlaylist");
        MainWindow->setMenuBar(menuBar);

        menuBar->addAction(menuPlaylist->menuAction());
        menuPlaylist->addAction(actionSave);
        menuPlaylist->addAction(actionGather);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow* MainWindow) {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Arfarius", nullptr));
        actionSave->setText(QCoreApplication::translate("MainWindow", "Save", nullptr));
        actionGather->setText(QCoreApplication::translate("MainWindow", "Gather", nullptr));
        timeLabel->setText(QCoreApplication::translate("MainWindow", "=(-_-)=", nullptr));
        menuPlaylist->setTitle(QCoreApplication::translate("MainWindow", "Playlist", nullptr));
    } // retranslateUi
};

namespace Ui {
class MainWindow : public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
