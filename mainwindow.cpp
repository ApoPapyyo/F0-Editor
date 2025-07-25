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
    connect(ui->action_save, &QAction::triggered, editor->get_pe(), &PitchEditor::save_f0);
    connect(ui->action_save_as, &QAction::triggered, editor->get_pe(), &PitchEditor::save_f0_as);
    connect(editor->get_pe(), &PitchEditor::titlechange, this, &MainWindow::titleChange);
    PitchEditor::eMouseMode m(editor->get_pe()->getMode());
    if(m == PitchEditor::eMouseMode::Select) ui->select_tool->setChecked(true);
    else if(m == PitchEditor::eMouseMode::Write) ui->write_tool->setChecked(true);
    else if(m == PitchEditor::eMouseMode::Erase) ui->erase_tool->setChecked(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateStatusBar(const QString &text)
{
    ui->statusbar->showMessage(text);
}

void MainWindow::titleChange(const QString &text)
{
    this->setWindowTitle(tr("F0エディタ %1").arg(text));
    update();
}

void MainWindow::closeEvent(QCloseEvent *ev)
{
    editor->get_pe()->closeEvent(ev);
}

void MainWindow::on_select_tool_toggled(bool checked)
{
    if(checked) editor->get_pe()->setMode(PitchEditor::eMouseMode::Select);
}


void MainWindow::on_erase_tool_toggled(bool checked)
{
    if(checked) editor->get_pe()->setMode(PitchEditor::eMouseMode::Erase);
}


void MainWindow::on_write_tool_toggled(bool checked)
{
    if(checked) editor->get_pe()->setMode(PitchEditor::eMouseMode::Write);
}

