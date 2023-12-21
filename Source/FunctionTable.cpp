#include "FunctionTable.h"
#ifndef _USE_MATH_DEFINES
  #define _USE_MATH_DEFINES
#endif
#include <math.h>

void FunctionTable::init(int tableLength)
{
    if (nTableSize == tableLength) return;
    nTableSize = tableLength;
    if (pWaveTable) delete[] pWaveTable;
    pWaveTable = new float[tableLength];
}
    
void FunctionTable::deinit()
{
    if (pWaveTable) delete[] pWaveTable;
    nTableSize = 0;
    pWaveTable = 0;
}
    
// Initialize a FunctionTable to an exponential shape, scaled to fit in the unit square.
// The function itself is y = -exp(-x), where x ranges from 'left' to 'right'.
// The more negative 'left' is, the more vertical the start of the rise; -5.0 yields near-vertical.
// The more positive 'right' is, the more horizontal then end of the rise; +5.0 yields near-horizontal.
void FunctionTable::exponentialCurve(float left, float right)
{
    // in case user forgot, init table to default size
    if (pWaveTable == 0) init(nTableSize);
        
    float bottom = -expf(-left);
    float top = -expf(-right);
    float vscale = 1.0f / (top - bottom);
        
    float x = left;
    float dx = (right - left) / (nTableSize - 1);
    for (int i=0; i < nTableSize; i++, x += dx)
        pWaveTable[i] = vscale * (-expf(-x) - bottom);
}

// Initialize a FunctionTable to a power-curve shape, defined in the unit square.
// The given exponent may be positive for a concave-up shape or negative for concave-down.
// Typical range of the exponent is plus or minus 4 to 5.
void FunctionTable::powerCurve(float exponent)
{
    if (pWaveTable == 0) init(nTableSize);

    float x = 0.0f;
    float dx = 1.0f / (nTableSize - 1);
    for (int i=0; i < nTableSize; i++, x += dx)
        pWaveTable[i] = powf(x, exponent);
}
