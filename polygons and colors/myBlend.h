#include "utils.h"
#include "GPaint.h"

#ifndef myBlend_DEFINED
#define myBlend_DEFINED

//     kClear,    //!<     0
static inline GPixel kClear(GPixel src, GPixel dst) {
    return 0;
}
//     kSrc,      //!<     S
static inline GPixel kSrc(GPixel src, GPixel dst) {
    return src;
}
//     kDst,      //!<     D
static inline GPixel kDst(GPixel src, GPixel dst) {
    return dst;
}
//     kSrcOver,  //!<     S + (1 - Sa)*D
static inline GPixel kSrcOver(GPixel src, GPixel dst) {
    // source
    int srcA = GPixel_GetA(src);
    int srcR = GPixel_GetR(src);
    int srcG = GPixel_GetG(src);
    int srcB = GPixel_GetB(src);
    // destination
    int dstA = GPixel_GetA(dst);
    int dstR = GPixel_GetR(dst);
    int dstG = GPixel_GetG(dst);
    int dstB = GPixel_GetB(dst);
    // final
    return GPixel_PackARGB(
        srcA + div255( (255-srcA) * dstA), 
        srcR + div255( (255-srcA) * dstR), 
        srcG + div255( (255-srcA) * dstG), 
        srcB + div255( (255-srcA) * dstB)
    );
}
//     kDstOver,  //!<     D + (1 - Da)*S
static inline GPixel kDstOver(GPixel src, GPixel dst) {
    return kSrcOver(dst, src);
}
//     kSrcIn,    //!<     Da * S
static inline GPixel kSrcIn(GPixel src, GPixel dst) {
    // source
    int srcA = GPixel_GetA(src);
    int srcR = GPixel_GetR(src);
    int srcG = GPixel_GetG(src);
    int srcB = GPixel_GetB(src);
    // destination
    int dstA = GPixel_GetA(dst);
    int dstR = GPixel_GetR(dst);
    int dstG = GPixel_GetG(dst);
    int dstB = GPixel_GetB(dst);
    // final
    return GPixel_PackARGB(
        div255( (srcA * dstA) ), 
        div255( (srcR * dstA) ), 
        div255( (srcG * dstA) ), 
        div255( (srcB * dstA) )
    );
}
//     kDstIn,    //!<     Sa * D
static inline GPixel kDstIn(GPixel src, GPixel dst) {
    // final
    return kSrcIn(dst, src);
}
//     kSrcOut,   //!<     (1 - Da)*S
static inline GPixel kSrcOut(GPixel src, GPixel dst) {
    // source
    int srcA = GPixel_GetA(src);
    int srcR = GPixel_GetR(src);
    int srcG = GPixel_GetG(src);
    int srcB = GPixel_GetB(src);
    // destination
    int dstA = GPixel_GetA(dst);
    int dstR = GPixel_GetR(dst);
    int dstG = GPixel_GetG(dst);
    int dstB = GPixel_GetB(dst);

    return GPixel_PackARGB(
        div255( (255 - dstA) * srcA ),
        div255( (255 - dstA) * srcR ),
        div255( (255 - dstA) * srcG ),
        div255( (255 - dstA) * srcB )
    );
}
//     kDstOut,   //!<     (1 - Sa)*D
static inline GPixel kDstOut(GPixel src, GPixel dst) {
    return kSrcOut(dst, src);
}
//     kSrcATop,  //!<     Da*S + (1 - Sa)*D
static inline GPixel kSrcATop(GPixel src, GPixel dst) {
    // source
    int srcA = GPixel_GetA(src);
    int srcR = GPixel_GetR(src);
    int srcG = GPixel_GetG(src);
    int srcB = GPixel_GetB(src);
    // destination
    int dstA = GPixel_GetA(dst);
    int dstR = GPixel_GetR(dst);
    int dstG = GPixel_GetG(dst);
    int dstB = GPixel_GetB(dst);
    // final
    return GPixel_PackARGB(
        dstA, 
        div255( dstA * srcR ) + div255( (255 - srcA) * dstR ), 
        div255( dstA * srcG ) + div255( (255 - srcA) * dstG ), 
        div255( dstA * srcB ) + div255( (255 - srcA) * dstB )
    );
}
//     kDstATop,  //!<     Sa*D + (1 - Da)*S
static inline GPixel kDstATop(GPixel src, GPixel dst) {
    return kSrcATop(dst, src);
}
//     kXor,      //!<     (1 - Sa)*D + (1 - Da)*S
static inline GPixel kXor(GPixel src, GPixel dst) {
    // source
    int srcA = GPixel_GetA(src);
    int srcR = GPixel_GetR(src);
    int srcG = GPixel_GetG(src);
    int srcB = GPixel_GetB(src);
    // destination
    int dstA = GPixel_GetA(dst);
    int dstR = GPixel_GetR(dst);
    int dstG = GPixel_GetG(dst);
    int dstB = GPixel_GetB(dst);
    // final
    return GPixel_PackARGB(
        srcA + dstA - (2 * (div255( (srcA*dstA) ))), 
        div255( dstR * (255 - srcA) ) + div255( srcR * (255 - dstA) ), 
        div255( dstG * (255 - srcA) ) + div255( srcG * (255 - dstA) ), 
        div255( dstB * (255 - srcA) ) + div255( srcB * (255 - dstA) ) 
    );
}

typedef GPixel (*BlendMode)(GPixel, GPixel);

// BlendModes[0] = kClear, BlendModes[1] = kSrc, ... etc
BlendMode BlendModes[] = {
    kClear,
    kSrc,
    kDst,
    kSrcOver,
    kDstOver,
    kSrcIn,
    kDstIn,
    kSrcOut,
    kDstOut,
    kSrcATop,
    kDstATop,
    kXor
};

static inline BlendMode getBM(const GBlendMode mode) {
    return BlendModes[static_cast<int>(mode)];
}
// takes in a mode which is changed to an int, 
// which retrieves the blendmode from the array
static inline BlendMode getBM(const GBlendMode mode, const GPixel src) {
    return BlendModes[static_cast<int>(mode)];
}

#endif