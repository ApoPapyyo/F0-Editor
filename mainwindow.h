#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include "pitcheditor.h"
#include "piano.h"

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

private:
    Ui::MainWindow *ui;
    QSplitter *sp;
    PitchEditor *pe;
    Piano *piano;

private slots:
    void updateStatusBar(const QString &text);
    void updateScrollBar(int value);
signals:
    void scrollbarsChanged(int x, int y);

};
#endif // MAINWINDOW_H
