#include "EnvelopeEditor.h"
#include "Settings.h"

EnvelopeEditor::EnvelopeEditor() : mouseEditModel(envDesc), env(MSEG_DEFAULT_PIXELS_WIDTH)
{
    mouseEditModel.addChangeListener(this);

    if (envDesc.empty())
        envDesc.push_back({ 0.5f, 0.5f, 0.0f,    MSEG_DEFAULT_PIXELS_WIDTH });
}

EnvelopeEditor::~EnvelopeEditor()
{
}

/*======================================================================================================================*/

void EnvelopeEditor::paint(Graphics& g)
{
    // this will be a child component of msegEditor with own background fill/lines.. so no need to fill background.
    env.reset(&paintingEnv);

    auto bounds = getLocalBounds().reduced(INSET_PIXELS);
    const float x0 = INSET_PIXELS;
    const float y0 = INSET_PIXELS;
    int height = bounds.getHeight();
    int width = bounds.getWidth();


    Path p;
    float fx, fy;
    bool endOfEnvelope = env.getSample(fy);
    p.startNewSubPath(x0 + 0.0f, y0 + (1.0f - fy) * height);
    int ix = 1;
    for (ix = 1; !endOfEnvelope && ix < width; ix++)
    {
        endOfEnvelope = env.getSample(fy);
        fx = float(ix);
        p.lineTo(x0 + fx, y0 + (1.0f - fy) * height);
    }


    g.setColour(Colours::grey);

    // put a circle at every segment start point
    Rectangle<float> dotRect(2 * DOT_RADIUS, 2 * DOT_RADIUS);
    int segStart = 0;
    int segEnd = 0;
    int si = 0;
    for (; si < paintingEnv.size(); si++)
    {
        fx = x0 + float(segStart);
        fy = y0 + (1.0f - paintingEnv[si].initialValue) * height;

        g.drawEllipse(dotRect.withCentre(Point<float>(fx, fy)), 2.0f);

        int segLength = paintingEnv[si].lengthSamples;
        segEnd = segStart + segLength;
        segStart += segLength;
    }

    // put a circle at the last segment end point
    fx = x0 + float(segEnd);
    fy = y0 + (1.0f - paintingEnv[si - 1].finalValue) * bounds.getHeight();

    g.drawEllipse(dotRect.withCentre(Point<float>(fx, fy)), 2.0f);


    p.lineTo(fx, fy); // the very last line section 

    // paint the graph
    g.setColour(juce::Colours::white);
    g.strokePath(p, PathStrokeType(2.0f));
}

/*======================================================================================================================*/

void EnvelopeEditor::resized()
{
    mouseEditModel.setBounds(getLocalBounds().reduced(INSET_PIXELS));

    updatePaintingEnv();
}

/*======================================================================================================================*/

void EnvelopeEditor::flipH()
{
    if (envDesc.size() == 0) return;

    envDescCopy.clear();
    envDescCopy = envDesc;

    int envDescSize = envDesc.size();

    for (int i = 0; i < envDescSize; i++)
    {
        float oldYinitial = envDescCopy[(envDescSize - 1 - i)].initialValue;
        float oldYfinal = envDescCopy[(envDescSize - 1 - i)].finalValue;
        float oldCurve = envDescCopy[(envDescSize - 1 - i)].curvature;
        int oldLengthSamples = envDescCopy[(envDescSize - 1 - i)].lengthSamples;

        envDesc.at(i).initialValue = oldYfinal;
        envDesc.at(i).finalValue = oldYinitial;
        envDesc.at(i).curvature = oldCurve * -1.0f;
        envDesc.at(i).lengthSamples = oldLengthSamples;
    }

    sendChangeMessage(); // to let external code to know when to re-read the data from MSEG
    resized();
}

/*======================================================================================================================*/

void EnvelopeEditor::flipV()
{
    if (envDesc.size() == 0) return;

    for (int i = 0; i < (int(envDesc.size())); i++)
    {
        float oldYinitial = envDesc.at(i).initialValue;
        float oldYfinal = envDesc.at(i).finalValue;

        envDesc.at(i).initialValue = (-1.0f * (oldYinitial - 0.5)) + 0.5f;
        envDesc.at(i).finalValue = (-1.0f * (oldYfinal - 0.5)) + 0.5f;
        envDesc.at(i).curvature = envDesc.at(i).curvature;
    }

    sendChangeMessage(); // to let external code to know when to re-read the data from MSEG
    resized();

}

/*======================================================================================================================*/

void EnvelopeEditor::clearSegments()
{
    envDesc.clear();
    envDesc.push_back({ 0.5f, 0.5f, 0.0f,  MSEG_DEFAULT_PIXELS_WIDTH });

    resized();
    sendChangeMessage(); // to let external code to know when to re-read the data from MSEG
}


/*======================================================================================================================*/

int EnvelopeEditor::MouseEditModel::getSegmentIndexFor(int sampleIndex)
{
    int segStart = 0;
    for (int i = 0; i < envDesc.size(); i++)
    {
        int segLength = envDesc.at(i).lengthSamples;
        int segEnd = segStart + segLength;
        if (sampleIndex >= segStart && sampleIndex <= segEnd)
        {
            return i;
        }
        segStart += segLength;
    }
    return -1;  // not found
}

/*======================================================================================================================*/

int EnvelopeEditor::MouseEditModel::getControlPointIndexFor(int mx, int my, int xAllowance, int yAllowance)
{
    juce::Rectangle<int> bounds = juce::Rectangle<int>{ 0,0,  MSEG_DEFAULT_PIXELS_WIDTH, MSEG_DEFAULT_PIXELS_HEIGHT };

    int segStart = 0;
    for (int i = 0; i < envDesc.size(); i++)
    {
        int segLength = envDesc.at(i).lengthSamples;
        int segEnd = segStart + segLength;
        //if ((mx < 0 || mx >= segStart) && (mx - segStart) < allowance)
        if (abs(mx - segStart) < xAllowance)
        {
            int yAbs = abs(my - ((1.0f - envDesc.at(i).initialValue) * MSEG_DEFAULT_PIXELS_HEIGHT));

            if (yAbs < yAllowance)
                return i;
        }
        //if ((segEnd - mx) < allowance)
        if (abs(mx - segEnd) < xAllowance)
        {
            int yAbs = abs(my - ((1.0f - envDesc.at(i).finalValue) * MSEG_DEFAULT_PIXELS_HEIGHT));

            if (yAbs < yAllowance)
                return i + 1;
        }
        segStart += segLength;
    }
    return -1;  // not found
}

/*======================================================================================================================*/

void EnvelopeEditor::MouseEditModel::getSegmentStartAndEndIndices(int segIndex, int& segStart, int& segEnd, int& prevSegStart)
{
    segStart = 0;
    prevSegStart = 0;
    for (int i = 0; i < envDesc.size(); i++)
    {
        int segLength = envDesc.at(i).lengthSamples;
        segEnd = segStart + segLength;
        if (i == segIndex) return;
        prevSegStart = segStart;
        segStart += segLength;
    }
    prevSegStart = segStart = segEnd = 0;
}

/*======================================================================================================================*/

void EnvelopeEditor::MouseEditModel::mouseDown(float mx, float my, int numberofclicks)
{
    // convert mouse down X and Y to the virtual fixed buffer size we use for storing data/modulating 
    // this prevents rescaling error build ups by not needing to resize the data that describes the MSEG... MVC...

    juce::Rectangle<int> descriptorBounds = juce::Rectangle<int>{ 0,0,  MSEG_DEFAULT_PIXELS_WIDTH, MSEG_DEFAULT_PIXELS_HEIGHT };

    int screenBoundsWidth = screenBounds.getWidth();
    int screenBoundsHeight = screenBounds.getHeight();

    float mouseXscale = (float)descriptorBounds.getWidth() / (float)screenBoundsWidth;
    float mouseYscale = (float)descriptorBounds.getHeight() / (float)screenBoundsHeight;

    mx = mx * mouseXscale;
    my = my * mouseYscale;

    float fy = 1.0f - (float)(my / (float)descriptorBounds.getHeight());

    actionType = none;

    segmentIndex = getControlPointIndexFor(mx, my, (2 * DOT_RADIUS * mouseXscale), (2 * DOT_RADIUS * mouseYscale));
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
        if (numberofclicks > 1)
        {
            // delete control point
            auto it = envDesc.begin();
            for (int i = 0; i < segmentIndex; i++) it++;

            auto seg = envDesc.at(segmentIndex);
            envDesc.erase(it);
            segmentIndex--;

            envDesc.at(segmentIndex).finalValue = seg.finalValue;
            envDesc.at(segmentIndex).lengthSamples += seg.lengthSamples;

            sendChangeMessage();
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
        if (numberofclicks > 1)
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
            envDesc.at(segmentIndex).initialValue = fy;
            envDesc.at(segmentIndex).lengthSamples -= lengthDelta;

            sendChangeMessage();
        }
        else
        {
            // drag to adjust segment curvature
            segmentIndex = getSegmentIndexFor(mx);
            if (segmentIndex >= 0)
            {
                actionType = draggingSegmentBody;
                savedCurvature = envDesc.at(segmentIndex).curvature;
            }
        }
    }
}

/*======================================================================================================================*/

void EnvelopeEditor::MouseEditModel::mouseDrag(float mx, float my, int dragYdistance)
{
    // convert mouse down X and Y to the virtual fixed buffer size we use for storing data/modulating - this prevents rescaling error build ups by not needing to 
    // resize the data that describes the MSEG... MVC...

    juce::Rectangle<int> descriptorBounds = juce::Rectangle<int>{ 0,0,  MSEG_DEFAULT_PIXELS_WIDTH, MSEG_DEFAULT_PIXELS_HEIGHT };


    int screenBoundsWidth = screenBounds.getWidth();
    int screenBoundsHeight = screenBounds.getHeight();

    float mouseXscale = (float)descriptorBounds.getWidth() / (float)screenBoundsWidth;
    float mouseYscale = (float)descriptorBounds.getHeight() / (float)screenBoundsHeight;


    mx = mx * mouseXscale;
    my = my * mouseYscale;


    float fy = 1.0f - (float)(my / (float)descriptorBounds.getHeight());


    if (fy < 0.0f) fy = 0.0f;
    if (fy > 1.0f) fy = 1.0f;
    float dy, curvature;
    int segStart, segEnd, prevSegStart, lengthDelta;
    getSegmentStartAndEndIndices(segmentIndex, segStart, segEnd, prevSegStart);

    switch (actionType)
    {
    case draggingLeftmostControlPoint:
        envDesc.at(segmentIndex).initialValue = fy;
        break;
    case draggingRightmostControlPoint:
        envDesc.at(segmentIndex - 1).finalValue = fy;
        break;
    case draggingInteriorControlPoint:
        if ((mx) >= prevSegStart && (mx) <= segEnd)
        {
            lengthDelta = mx - segStart;
            envDesc.at(segmentIndex).lengthSamples -= lengthDelta;
            envDesc.at(segmentIndex - 1).lengthSamples += lengthDelta;
        }
        envDesc.at(segmentIndex).initialValue = fy;
        envDesc.at(segmentIndex - 1).finalValue = fy;
        break;
    case draggingSegmentBody:
        dy = 0.02f * dragYdistance;
        if (envDesc.at(segmentIndex).finalValue < envDesc.at(segmentIndex).initialValue) dy = -dy;
        curvature = savedCurvature - dy * dy * dy;
        if (curvature > MAX_CURVATURE) curvature = MAX_CURVATURE;
        if (curvature < MIN_CURVATURE) curvature = MIN_CURVATURE;
        envDesc.at(segmentIndex).curvature = curvature;
        break;
    default:
        return;
    }
    sendChangeMessage();
}

/*======================================================================================================================*/

void EnvelopeEditor::fillModulationBuffer(std::vector<float>& buffer)
{
    // the pixel resolution of the edit envelope and the saved/modulation data buffer will be the same.. we only re-scale for drawing the output.

    jassert(buffer.size() == MSEG_DEFAULT_PIXELS_WIDTH);

    int width = buffer.size();

    env.reset(&envDesc, 0);
    float fy;
    bool endOfEnvelope = false;

    for (int ix = 0; !endOfEnvelope && ix < width-1; ix++)
    {
        endOfEnvelope = env.getSample(fy);
        buffer[ix] = fy; // bipolar values by default.
    }

    buffer[width - 1] = envDesc.at(envDesc.size() - 1).finalValue;
}

/*======================================================================================================================*/

void EnvelopeEditor::mouseDown(const MouseEvent& event)
{
    mouseEditModel.mouseDown(event.x - INSET_PIXELS, event.y - INSET_PIXELS, event.getNumberOfClicks());
}

/*======================================================================================================================*/

void EnvelopeEditor::mouseDrag(const MouseEvent& event)
{
    mouseEditModel.mouseDrag(event.x - INSET_PIXELS, event.y - INSET_PIXELS, event.getDistanceFromDragStartY());
}

/*======================================================================================================================*/

void EnvelopeEditor::changeListenerCallback(ChangeBroadcaster* source)
{
    if (source == &mouseEditModel)
    {
        updatePaintingEnv();
        sendChangeMessage(); // to let outside code know to re-read the model for other drawing updates based on it (e.g. to trigger call to 'fillModulationBuffer')
    }
}

/*======================================================================================================================*/

void EnvelopeEditor::updatePaintingEnv()
{
    if (envDesc.size() == 0) return;

    paintingEnv.clear();
    paintingEnv = envDesc;

    // resize all segments proportionally
    auto bounds = getLocalBounds().reduced(INSET_PIXELS);

    if (bounds.getWidth() > 0) // bounds has not yet been set.. so don't modify the data
    {
        int oldWidth = 0;

        for (auto seg : paintingEnv)
        {
            oldWidth += seg.lengthSamples;
        }

        int newWidth = bounds.getWidth();
        int totalWidth = 0;
        for (int i = 0; i < (int(paintingEnv.size()) - 1); i++)
        {
            float proportion = float(paintingEnv[i].lengthSamples) / float(oldWidth);
            int newLength = int(proportion * newWidth);
            paintingEnv[i].lengthSamples = newLength;
            totalWidth += newLength;
        }
        paintingEnv[paintingEnv.size() - 1].lengthSamples = newWidth - totalWidth;

        repaint();
    }
}
