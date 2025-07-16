#include "grapheditor.h"
#include "ui_grapheditor.h"

GraphEditor::GraphEditor(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GraphEditor)
{
    ui->setupUi(this);
}

GraphEditor::~GraphEditor()
{
    delete ui;
}
