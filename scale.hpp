#ifndef SCALE_HPP
#define SCALE_HPP

#include <QWidget>
#include "pitcheditor.hpp"

class Scale : public QWidget
{
    Q_OBJECT
public:
    explicit Scale(const PitchEditor::Offset& offset, const PitchEditor::Scale& scale, const PitchEditor::Config& conf, QWidget *parent = nullptr);
protected:
    void paintEvent(QPaintEvent* ev) override;
    void mouseMoveEvent(QMouseEvent* ev) override;
    void mousePressEvent(QMouseEvent* ev) override;
    void mouseReleaseEvent(QMouseEvent* ev) override;
private:
    struct {double pos; bool rclick, cclick, lclick;} cursor;
    const PitchEditor::Offset& offset;
    const PitchEditor::Scale& scale;
    const PitchEditor::Config& conf;

signals:
};

#endif // SCALE_HPP
