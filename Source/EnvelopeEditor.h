#pragma once
#include "JuceHeader.h"
#include "SegmentGenerator.h"

class EnvelopeEditor    : public Component
{
public:
    EnvelopeEditor();
    ~EnvelopeEditor();

    void paint (Graphics&) override;
    void resized() override;

    void mouseDown(const MouseEvent&) override;
    void mouseDrag(const MouseEvent&) override;

protected:
    MultiSegmentEnvelopeGenerator env;
    MultiSegmentEnvelopeGenerator::Descriptor envDesc;

    enum {
        none,
        draggingLeftmostControlPoint,
        draggingRightmostControlPoint,
        draggingInteriorControlPoint,
        draggingSegmentBody
    } actionType;

    Colour backgroundColour;
    int segmentIndex;
    float savedCurvature;

    void paintGraph(Graphics&);
    int getSegmentIndexFor(int sampleIndex);
    int getControlPointIndexFor(int mx, int my, int allowance=10);
    void getSegmentStartAndEndIndices(int segIndex, int &segStart, int& segEnd, int& prevSegStart);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnvelopeEditor)
};
