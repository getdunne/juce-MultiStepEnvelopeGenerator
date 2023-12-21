#pragma once
#include "JuceHeader.h"
#include "SegmentGenerator.h"
#include "Settings.h"

class EnvelopeEditor : public Component, private ChangeListener, public ChangeBroadcaster
{
public:
    EnvelopeEditor();
    ~EnvelopeEditor();

    void paint (Graphics&) override;
    void resized() override;

    void mouseDown(const MouseEvent&) override;
    void mouseDrag(const MouseEvent&) override;

    void fillModulationBuffer(std::vector<float>& buffer);

    void flipH();
    void flipV();
    void clearSegments();

    static const int  MSEG_DEFAULT_PIXELS_WIDTH = 1024;
    static const int  MSEG_DEFAULT_PIXELS_HEIGHT = 512;
    static const int MIN_CURVATURE = -10.0f;
    static const int MAX_CURVATURE = 10.0f;

private:

    void changeListenerCallback(ChangeBroadcaster* source) override;

    MultiSegmentEnvelopeGenerator env;
    MultiSegmentEnvelopeGenerator::Descriptor envDesc; // fixed pixels size envelope descriptor
    MultiSegmentEnvelopeGenerator::Descriptor envDescCopy; // for flip
    MultiSegmentEnvelopeGenerator::Descriptor paintingEnv;

    Colour backgroundColour;


    class MouseEditModel : public ChangeBroadcaster // the model will use a fixed pixel size to avoid rescaling accurancy issues - mouse x/y input is scaled to fit the model
    {
    private:
        MultiSegmentEnvelopeGenerator::Descriptor& envDesc; // for storage - all values should be floats..

        enum {
            none,
            draggingLeftmostControlPoint,
            draggingRightmostControlPoint,
            draggingInteriorControlPoint,
            draggingSegmentBody
        } actionType = none;

        int segmentIndex = 0;
        float savedCurvature = 0;

        juce::Rectangle<int> screenBounds;

    public:
        MouseEditModel(MultiSegmentEnvelopeGenerator::Descriptor& envDesc_) : envDesc(envDesc_) {}
        ~MouseEditModel() {}

        int getSegmentIndexFor(int sampleIndex);
        int getControlPointIndexFor(int mx, int my, int xAllowance, int yAllowance);
        void getSegmentStartAndEndIndices(int segIndex, int& segStart, int& segEnd, int& prevSegStart);

        void setBounds(juce::Rectangle<int> bounds) { screenBounds = bounds; }

        void mouseDown(float mx, float my, int numberOfClicks);
        void mouseDrag(float mx, float my, int dragYdistance);

    };

    MouseEditModel mouseEditModel;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnvelopeEditor)
};
