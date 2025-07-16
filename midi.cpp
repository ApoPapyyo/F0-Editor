#include "midi.h"
#include <QRegularExpression>
#include <QRegularExpressionMatch>

Note::Note()
    : name(None)
    , oct(0)
{}

Note::Note(Note &n)
    : name(n.name)
    , oct(n.oct)
{}

Note::Note(eNoteName n, int o)
    : name(n)
    , oct(o)
{}

Note::Note(QString str)
    : name(None)
    , oct(0)
{
}

Note Note::fromStr(const QString str)
{
    QRegularExpression re("^([A-Ga-g])([#b]?)(\\d+)$");
    QRegularExpressionMatch match = re.match(str.trimmed());

    if (!match.hasMatch()) {
        return Note(C, 4);
    }

    QString note = match.captured(1).toUpper() + match.captured(2); // 例: "C#", "Db"
    if(note.length() == 2) {
        if(note[1] == QChar('b')) {
            if(note[0] != QChar('A'))
                note[0] = QChar(note[0].unicode() - 1);
            else
                note[0] = QChar('A');
            note[1] = QChar('#');
        }
    }
    eNoteName n;
    if(note == "C") n = C;
    else if(note == "C#") n = Cs;
    else if(note == "D") n = D;
    else if(note == "D#") n = Ds;
    else if(note == "E") n = E;
    else if(note == "F") n = F;
    else if(note == "F#") n = Fs;
    else if(note == "G") n = G;
    else if(note == "G#") n = Gs;
    else if(note == "A") n = A;
    else if(note == "A#") n = As;
    else if(note == "B") n = B;
    else n = None;
    int octave = match.captured(3).toInt();

    return Note(n, octave);
}

