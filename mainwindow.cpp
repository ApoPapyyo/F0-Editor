#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QScrollBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , editor(new MidiEditor(this))
{
    ui->setupUi(this);

    int id(ui->gridLayout_2->indexOf(ui->dummy));
    int r, c, rs, cs;
    ui->gridLayout_2->getItemPosition(id, &r, &c, &rs, &cs);
    QLayoutItem *item(ui->gridLayout_2->takeAt(id));
    delete item->widget();
    ui->gridLayout_2->addWidget(editor, r, c, rs, cs);
    connect(editor->get_pe(), &PitchEditor::mouseMoved, this, &MainWindow::updateStatusBar);
    connect(ui->action_open_f0, &QAction::triggered, editor->get_pe(), &PitchEditor::open_f0);
    connect(ui->actio_close_file, &QAction::triggered, editor->get_pe(), &PitchEditor::close_f0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateStatusBar(const QString &text)
{
    ui->statusbar->showMessage(text);
}

