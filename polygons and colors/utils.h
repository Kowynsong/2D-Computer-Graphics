#ifndef utils_DEFINED
#define utils_DEFINED

#include "GColor.h"
#include "GPixel.h"

GPixel convert(const GColor& color) {
        /* 
    * GColor to GPixel
    * 1. pin the colors [0...1]
    * 2. premultiply & "convert" to GPixel by changing it to bytes
    * 3. pack the ARGB values
    */
    GPinToUnit(color.fA);
    int a = (int)floor( (color.fA * 255) + 0.5);

    GPinToUnit(color.fA * color.fR);
    int r = (int)floor(( (color.fA * color.fR) * 255) + 0.5);

    GPinToUnit(color.fA * color.fG);
    int g = (int)floor(( (color.fA * color.fG) * 255) + 0.5);
    
    GPinToUnit(color.fA * color.fB);
    int b = (int)floor(( (color.fA * color.fB) * 255) + 0.5);

    return GPixel_PackARGB(a, r, g, b);
}

int div255 (int x) {
    // return (x + 127) / 255;
    return ((x + 128) * 257) >> 16;
}

#endif