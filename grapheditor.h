#ifndef GRAPHEDITOR_H
#define GRAPHEDITOR_H

#include <QWidget>

namespace Ui {
class GraphEditor;
}

class GraphEditor : public QWidget
{
    Q_OBJECT

public:
    explicit GraphEditor(QWidget *parent = nullptr);
    ~GraphEditor();

private:
    Ui::GraphEditor *ui;
};

#endif // GRAPHEDITOR_H
