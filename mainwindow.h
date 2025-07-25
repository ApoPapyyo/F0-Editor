#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include "midieditor.h"
#include "pitcheditor.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    void closeEvent(QCloseEvent *ev);

private:
    Ui::MainWindow *ui;
    MidiEditor *editor;

private slots:
    void updateStatusBar(const QString &text);
    void titleChange(const QString &text);

    void on_select_tool_toggled(bool checked);
    void on_erase_tool_toggled(bool checked);
    void on_write_tool_toggled(bool checked);
};
#endif // MAINWINDOW_H
