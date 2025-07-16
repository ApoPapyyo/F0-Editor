#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , pe(new PitchEditor)

{
    ui->setupUi(this);
    pe->setParent(this);
    ui->verticalLayout->addWidget(pe);
    pe->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}
