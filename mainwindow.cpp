#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QScrollBar>
#include <QFileDialog>

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
    //connect(editor->get_pe(), &PitchEditor::mouseMoved, this, &MainWindow::updateStatusBar);
    //connect(editor->get_piano(), &Piano::mouseMoved, this, &MainWindow::updateStatusBar);
    connect(ui->action_open_f0, &QAction::triggered, this, &MainWindow::on_action_open_f0);
    connect(ui->actio_close_file, &QAction::triggered, this, &MainWindow::on_action_close_f0);
    connect(ui->action_save, &QAction::triggered, this, &MainWindow::on_action_save_f0);
    connect(ui->action_save_as, &QAction::triggered, this, &MainWindow::on_action_save_f0_as);
    //connect(ui->action_open_f0, &QAction::triggered, editor->get_pe(), &PitchEditor::open_f0);
    //connect(ui->actio_close_file, &QAction::triggered, editor->get_pe(), &PitchEditor::close_f0);
    //connect(ui->action_save, &QAction::triggered, editor->get_pe(), &PitchEditor::save_f0);
    //connect(ui->action_save_as, &QAction::triggered, editor->get_pe(), &PitchEditor::save_f0_as);
    //connect(editor->get_pe(), &PitchEditor::titlechange, this, &MainWindow::titleChange);
    PitchEditor::eMouseMode& m(editor->get_pe()->mouse.mode);
    if(m == PitchEditor::eMouseMode::Select) ui->select_tool->setChecked(true);
    else if(m == PitchEditor::eMouseMode::Write) ui->write_tool->setChecked(true);
    else if(m == PitchEditor::eMouseMode::Erase) ui->erase_tool->setChecked(true);
    //connect(ui->action_undo, &QAction::triggered, editor->get_pe(), &PitchEditor::undo);
    //connect(ui->action_redo, &QAction::triggered, editor->get_pe(), &PitchEditor::redo);
    //connect(editor->get_pe(), &PitchEditor::undo_redo_tgl, this, &MainWindow::undo_redo_tgl);
    connect(ui->play_button, &QPushButton::pressed, editor->get_pe(), &PitchEditor::play);
    connect(ui->stop_button, &QPushButton::pressed, editor->get_pe(), &PitchEditor::stop);
    connect(ui->tmpstop_button, &QPushButton::pressed, editor->get_pe(), &PitchEditor::pause);
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
    if(checked) editor->get_pe()->mouse.mode = PitchEditor::eMouseMode::Select;
}


void MainWindow::on_erase_tool_toggled(bool checked)
{
    if(checked) editor->get_pe()->mouse.mode = PitchEditor::eMouseMode::Erase;
}


void MainWindow::on_write_tool_toggled(bool checked)
{
    if(checked) editor->get_pe()->mouse.mode = PitchEditor::eMouseMode::Write;
}

void MainWindow::undo_redo_tgl(bool undo, bool redo)
{
    ui->action_undo->setEnabled(undo);
    ui->action_redo->setEnabled(redo);
}

void MainWindow::on_action_open_f0()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("F0ファイルを開く"),
        QString(),                // 初期ディレクトリ（空ならカレントディレクトリ）
        tr("CSVファイル (*.csv) ;; F0ファイル (*.f0)")    // フィルタ
    );
    editor->get_pe()->openF0(fileName);
}

void MainWindow::on_action_close_f0()
{
    editor->get_pe()->closeF0();
}

void MainWindow::on_action_save_f0()
{
    editor->get_pe()->saveF0();
}

void MainWindow::on_action_save_f0_as()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("F0ファイル保存先を選択"),
        QString(),  // 初期ディレクトリや初期ファイル名
        tr("テキストファイル (*.txt);;すべてのファイル (*)")
    );
    editor->get_pe()->saveF0as(fileName);
}
