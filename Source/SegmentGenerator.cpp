#include "SegmentGenerator.h"
#include "Settings.h"


void SegmentGenerator::reset(float initialValue, float finalValue, float curvature, int segmentLengthSamples)
{
    firstValue = initialValue;
    lastValue = finalValue;
    isHorizontal = finalValue == initialValue;
    segLength = segmentLengthSamples;

    isLinear = (curvature == 0.0f);

#ifdef EXPONENTIAL_CURVES
    if (!isLinear) table.exponentialCurve(-curvature, curvature);
#else
    if (!isLinear) table.powerCurve(powf(2.0f, -curvature));
#endif

    start();
}

void SegmentGenerator::start()
{
    tcount = 0;
    x = 0.0f;
    dx = 1.0f / segLength;
}


void MultiSegmentEnvelopeGenerator::setupCurSeg()
{
    SegmentDescriptor& seg = (*segments)[curSegIndex];
    SegmentGenerator::reset(seg.initialValue, seg.finalValue, seg.curvature, seg.lengthSamples);
}

void MultiSegmentEnvelopeGenerator::setupCurSeg(float initValue)
{
    SegmentDescriptor& seg = (*segments)[curSegIndex];
    SegmentGenerator::reset(initValue, seg.finalValue, seg.curvature, seg.lengthSamples);
}

void MultiSegmentEnvelopeGenerator::reset(Descriptor* pDesc, int initialSegmentIndex)
{
    segments = pDesc;
    curSegIndex = initialSegmentIndex;
    setupCurSeg();
}

void MultiSegmentEnvelopeGenerator::advanceToSegment(int segIndex)
{
    float startValue;
    getSample(startValue);
    curSegIndex = segIndex;
    setupCurSeg(startValue);
}
