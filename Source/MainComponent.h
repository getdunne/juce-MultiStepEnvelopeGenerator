#pragma once

#include "JuceHeader.h"
#include "EnvelopeEditor.h"

class MainComponent   : public Component
{
public:
    MainComponent();

    void resized() override;

protected:
    EnvelopeEditor envEd;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
