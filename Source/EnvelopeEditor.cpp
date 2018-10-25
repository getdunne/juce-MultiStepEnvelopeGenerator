#include "EnvelopeEditor.h"

EnvelopeEditor::EnvelopeEditor()
{
    backgroundColour = getLookAndFeel().findColour(ResizableWindow::backgroundColourId);
    envDesc.push_back({ 0.0f, 1.0f, 0.0f, 600 });
}

EnvelopeEditor::~EnvelopeEditor()
{
}

void EnvelopeEditor::paintGraph(Graphics& g)
{
    // paint the graph
    Path p;
    float fx, fy;
    bool endOfEnvelope = env.getSample(fy);
    p.startNewSubPath(0.0f, (1.0f - fy) * getHeight());
    for (int ix = 1; !endOfEnvelope && ix < getWidth(); ix++)
    {
        endOfEnvelope = env.getSample(fy);
        fx = float(ix);
        p.lineTo(fx, (1.0f - fy) * getHeight());
    }
    g.strokePath(p, PathStrokeType(2.0f));

    // put a circle at every segment start point
    Rectangle<float> dotRect(10.0f, 10.0f);
    int segStart = 0;
    int segEnd = 0;
    int si = 0;
    for ( ; si < envDesc.size(); si++)
    {
        fx = float(segStart);
        fy = (1.0f - envDesc[si].initialValue) * getHeight();
        g.setColour(backgroundColour);
        g.fillEllipse(dotRect.withCentre(Point<float>(fx, fy)));
        g.setColour(Colours::white);
        g.drawEllipse(dotRect.withCentre(Point<float>(fx, fy)), 2.0f);

        int segLength = envDesc[si].lengthSamples;
        segEnd = segStart + segLength;
        segStart += segLength;
    }

    // put a circle at the last segment end point
    fx = float(segEnd);
    fy = (1.0f - envDesc[si-1].finalValue) * getHeight();
    g.setColour(backgroundColour);
    g.fillEllipse(dotRect.withCentre(Point<float>(fx, fy)));
    g.setColour(Colours::white);
    g.drawEllipse(dotRect.withCentre(Point<float>(fx, fy)), 2.0f);
}

void EnvelopeEditor::paint (Graphics& g)
{
    g.fillAll(backgroundColour);
    env.reset(&envDesc);
    g.setColour(Colours::white);
    paintGraph(g);
}

void EnvelopeEditor::resized()
{
    if (envDesc.size() == 0) return;

    // resize all segments proportionally
    int oldWidth = 0;
    for (auto& seg : envDesc) oldWidth += seg.lengthSamples;

    int newWidth = getWidth();
    int totalWidth = 0;
    for (int i=0; i < (int(envDesc.size()) - 1); i++)
    {
        float proportion = float(envDesc[i].lengthSamples) / float(oldWidth);
        int newLength = int(proportion * newWidth);
        envDesc[i].lengthSamples = newLength;
        totalWidth += newLength;
    }
    envDesc[envDesc.size() - 1].lengthSamples = newWidth - totalWidth;
}

int EnvelopeEditor::getSegmentIndexFor(int sampleIndex)
{
    int segStart = 0;
    for (int i = 0; i < envDesc.size(); i++)
    {
        int segLength = envDesc[i].lengthSamples;
        int segEnd = segStart + segLength;
        if (sampleIndex >= segStart && sampleIndex <= segEnd) return i;
        segStart += segLength;
    }
    return -1;  // not found
}

int EnvelopeEditor::getControlPointIndexFor(int mx, int my, int allowance)
{
    int segStart = 0;
    for (int i = 0; i < envDesc.size(); i++)
    {
        int segLength = envDesc[i].lengthSamples;
        int segEnd = segStart + segLength;
        if (mx > segStart && (mx - segStart) < allowance)
        {
            if (abs(my - (1.0f - envDesc[i].initialValue) * getHeight()) < allowance)
                return i;
        }
        if (mx <= segEnd && (segEnd - mx) < allowance)
        {
            if (abs(my - (1.0f - envDesc[i].finalValue) * getHeight()) < allowance)
                return i + 1;
        }
        segStart += segLength;
    }
    return -1;  // not found
}

void EnvelopeEditor::getSegmentStartAndEndIndices(int segIndex, int &segStart, int&segEnd, int& prevSegStart)
{
    segStart = 0;
    prevSegStart = 0;
    for (int i = 0; i < envDesc.size(); i++)
    {
        int segLength = envDesc[i].lengthSamples;
        segEnd = segStart + segLength;
        if (i == segIndex) return;
        prevSegStart = segStart;
        segStart += segLength;
    }
    prevSegStart = segStart = segEnd = 0;
}

void EnvelopeEditor::mouseDown(const MouseEvent& evt)
{
    int mx = evt.getPosition().getX();
    int my = evt.getPosition().getY();
    float fy = 1.0f - my / float(getHeight());
    actionType = none;

    segmentIndex = getControlPointIndexFor(mx, my);
    if (segmentIndex == 0)
    {
        // hit leftmost control point
        actionType = draggingLeftmostControlPoint;
    }
    else if (segmentIndex == envDesc.size())
    {
        // hit rightmost control point
        actionType = draggingRightmostControlPoint;
    }
    else if (segmentIndex > 0)
    {
        // hit an interior control point
        if (evt.getNumberOfClicks() > 1)
        {
            // delete control point
            auto it = envDesc.begin();
            for (int i = 0; i < segmentIndex; i++) it++;

            auto seg = envDesc[segmentIndex];
            envDesc.erase(it);
            segmentIndex--;

            envDesc[segmentIndex].finalValue = seg.finalValue;
            envDesc[segmentIndex].lengthSamples += seg.lengthSamples;

            repaint();
        }
        else
        {
            // drag the control point
            actionType = draggingInteriorControlPoint;
        }
    }

    else
    {
        // didn't hit any control points
        if (evt.getNumberOfClicks() > 1)
        {
            // insert new control point
            segmentIndex = getSegmentIndexFor(mx);
            auto it = envDesc.begin();
            for (int i = 0; i < segmentIndex; i++) it++;

            auto seg = *it;
            int segStart, segEnd, prevSegStart;
            getSegmentStartAndEndIndices(segmentIndex, segStart, segEnd, prevSegStart);
            int lengthDelta = mx - segStart;
            envDesc.insert(it, { seg.initialValue, fy, 0.0, lengthDelta });

            segmentIndex++;
            envDesc[segmentIndex].initialValue = fy;
            envDesc[segmentIndex].lengthSamples -= lengthDelta;

            repaint();
        }
        else
        {
            // drag to adjust segment curvature
            segmentIndex = getSegmentIndexFor(mx);
            if (segmentIndex >= 0)
            {
                actionType = draggingSegmentBody;
                savedCurvature = envDesc[segmentIndex].curvature;
            }
        }
    }
}

void EnvelopeEditor::mouseDrag(const MouseEvent& evt)
{
    int mx = evt.getPosition().getX();
    float fy = 1.0f - evt.getPosition().getY() / float(getHeight());
    if (fy < 0.0f) fy = 0.0f;
    if (fy > 1.0f) fy = 1.0f;
    float dy, curvature;
    int segStart, segEnd, prevSegStart, lengthDelta;
    getSegmentStartAndEndIndices(segmentIndex, segStart, segEnd, prevSegStart);

    switch (actionType)
    {
        case draggingLeftmostControlPoint:
            envDesc[segmentIndex].initialValue = fy;
            break;
        case draggingRightmostControlPoint:
            envDesc[segmentIndex - 1].finalValue = fy;
            break;
        case draggingInteriorControlPoint:
            if (mx >= prevSegStart && mx <= segEnd)
            {
                lengthDelta = mx - segStart;
                envDesc[segmentIndex].lengthSamples -= lengthDelta;
                envDesc[segmentIndex - 1].lengthSamples += lengthDelta;
            }
            envDesc[segmentIndex].initialValue = fy;
            envDesc[segmentIndex - 1].finalValue = fy;
            break;
        case draggingSegmentBody:
            dy = 0.02f * evt.getDistanceFromDragStartY();
            if (envDesc[segmentIndex].finalValue < envDesc[segmentIndex].initialValue) dy = -dy;
            curvature = savedCurvature - dy * dy*dy;
            if (curvature > 50.0f) curvature = 50.0f;
            if (curvature < -50.0f) curvature = -50.0f;
            envDesc[segmentIndex].curvature = curvature;
            break;
        default:
            return;
    }
    repaint();
}
