#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , pe(new PitchEditor)

{
    ui->setupUi(this);
    pe->setParent(ui->centralwidget);
    setCentralWidget(pe);
    pe->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}
