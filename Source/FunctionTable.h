#pragma once

#define DEFAULT_TABLE_SIZE 256

struct FunctionTable
{
    float *pWaveTable;
    int nTableSize;
        
    FunctionTable() : pWaveTable(0), nTableSize(0) {}
    ~FunctionTable() { deinit(); }
        
    void init(int tableLength=DEFAULT_TABLE_SIZE);
    void deinit();
        
    void exponentialCurve(float left, float right);
    void powerCurve(float exponent);
        
    inline float interp_bounded(float phase)
    {
        if (phase < 0) return pWaveTable[0];
        if (phase >= 1.0) return pWaveTable[nTableSize-1];
            
        float readIndex = phase * (nTableSize - 1);
        int ri = int(readIndex);
        float f = readIndex - ri;
        int rj = ri + 1;
        // if (rj >= nTableSize) rj = nTableSize - 1;
            
        float si = pWaveTable[ri];
        float sj = pWaveTable[rj];
        return (float)((1.0 - f) * si + f * sj);
    }
};
