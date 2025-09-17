#include "scale.hpp"

Scale::Scale(const PitchEditor::Offset& offset_, const PitchEditor::Scale& scale_, const PitchEditor::Config& conf_, QWidget *parent)
    : QWidget{parent}
    , cursor{0.0, false, false, false}
    , offset{offset_}
    , scale{scale_}
    , conf{conf_}
{}
