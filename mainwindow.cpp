#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , pe(new PitchEditor(this))

{
    ui->setupUi(this);
    ui->verticalLayout->addWidget(pe);
    pe->show();
    connect(pe, &PitchEditor::mouseMoved, this, &MainWindow::updateStatusBar);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateStatusBar(const QString &text)
{
    ui->statusbar->showMessage(text);
}
