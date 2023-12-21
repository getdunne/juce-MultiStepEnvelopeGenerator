#pragma once
#include "JuceHeader.h"
#include "FunctionTable.h"

class SegmentGenerator
{


public:

    SegmentGenerator(int tableSize_) : table(tableSize_)
    {}
    ~SegmentGenerator(){}

    void reset(float initialValue, float finalValue, float curvature, int segmentLengthSamples);

    void start();

    inline bool getSample(float& out)
    {
        float fx = float(x);
        if (isLinear)
        {
            out = firstValue + fx * (lastValue - firstValue);
        }
        else if (isHorizontal)
        {
            out = firstValue;
            if (segLength < 0) return false;        // non-timed "sustain"segment
        }
        else
        {
            out = firstValue + table.interp_bounded(fx) * (lastValue - firstValue);
        }

        x += dx;
        return (++tcount >= segLength);
    }



protected:
    float firstValue, lastValue;
    bool isHorizontal, isLinear;
    int tcount, segLength;
    double x, dx;

    FunctionTable table;
};

class MultiSegmentEnvelopeGenerator : public SegmentGenerator
{
public:

    MultiSegmentEnvelopeGenerator(int tableSize_) : SegmentGenerator(tableSize_) {}

    ~MultiSegmentEnvelopeGenerator() {}


    struct SegmentDescriptor
    {
        float initialValue;
        float finalValue;
        float curvature;
        int lengthSamples;
    };
    typedef std::vector<SegmentDescriptor> Descriptor;

    void reset(Descriptor* pDesc, int initialSegmentIndex = 0);
    void advanceToSegment(int segIndex);

    inline bool getSample(float& out)
    {
        if (SegmentGenerator::getSample(out))
        {
            if (++curSegIndex >= segments->size())
            {
                reset(segments);
                return true;
            }
            else
            {
                setupCurSeg();
            }
        }
        return false;
    }

    int getCurrentSegmentIndex() { return curSegIndex; }

protected:
    Descriptor* segments;
    int curSegIndex;

    void setupCurSeg();
    void setupCurSeg(float initValue);
};
