#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , sp(new QSplitter(Qt::Horizontal, this))
    , pe(new PitchEditor(this))
    , piano(new Piano(pe, this))
{
    ui->setupUi(this);

    sp->addWidget(piano);
    sp->addWidget(pe);

    sp->setSizes(QList({static_cast<int>(width()*0.25), static_cast<int>(width()*0.75)}));

    QGridLayout *lay(ui->gridLayout_2);
    int row, column, srow, scolumn;
    lay->getItemPosition(lay->indexOf(ui->dummy), &row, &column, &srow, &scolumn);
    QLayoutItem *item(lay->takeAt(lay->indexOf(ui->dummy)));
    delete item->widget();
    lay->addWidget(sp, row, column, srow, scolumn);
    connect(pe, &PitchEditor::mouseMoved, this, &MainWindow::updateStatusBar);
    connect(pe, &PitchEditor::scrolled, piano, &Piano::scrolled);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateStatusBar(const QString &text)
{
    ui->statusbar->showMessage(text);
}
