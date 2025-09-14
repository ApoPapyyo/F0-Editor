#ifndef PIANO_H
#define PIANO_H
#include <QWidget>
#include "note.hpp"
#include "pitcheditor.hpp"
#include "synth.hpp"

class Piano : public QWidget
{
    Q_OBJECT
public:
    explicit Piano(const PitchEditor::Offset& offset, const PitchEditor::Scale& scale, const PitchEditor::Config& conf, QWidget* parent = nullptr);
protected:
    void paintEvent(QPaintEvent* ev) override;
    void mouseMoveEvent(QMouseEvent* ev) override;
    void mousePressEvent(QMouseEvent* ev) override;
    void mouseReleaseEvent(QMouseEvent* ev) override;
private:
    struct {Note pos; bool click = false;} cursor;
    Synth synth;
    const PitchEditor::Offset& offset;
    const PitchEditor::Scale& scale;
    const PitchEditor::Config& conf;
    void freq_changed(double f);
signals:
    void update_statusbar(const QString& str);
};

#endif // PIANO_H
