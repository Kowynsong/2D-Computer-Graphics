#include "GMatrix.h"
#include "GPoint.h"

///////////////////////////////////////////////////////////////////////////////////////////////
// These methods must be implemented by the student.
GMatrix::GMatrix() {
    this->fMat[0] = 1;  this->fMat[1] = 0;  this->fMat[2] = 0;
    this->fMat[3] = 0;  this->fMat[4] = 1;  this->fMat[5] = 0;
}      
GMatrix GMatrix::Translate(float tx, float ty) {
    GMatrix translated = GMatrix();
    translated.fMat[TX] = tx;
    translated.fMat[TY] = ty;
    return translated;
}
GMatrix GMatrix::Scale(float sx, float sy) {
    GMatrix scaled = GMatrix();
    scaled.fMat[SX] = sx;
    scaled.fMat[SY] = sy;
    return scaled;
}
GMatrix GMatrix::Rotate(float radians) {
    GMatrix rotated = GMatrix(
        cos(radians), 0-sin(radians), 0,
        sin(radians), cos(radians), 0
    );
    return rotated;
}
GMatrix GMatrix::Concat(const GMatrix& a, const GMatrix& b) {
    /**
     * a[0] * b[0] + a[1] * b[3] + a[2] * 0,
     * a[0] * b[1] + a[1] * b[4] + a[2] * 0, 
     * a[0] * b[2] + a[1] * b[5] + a[2] * 1,
     * a[3] * b[0] + a[4] * b[3] + a[5] * 0, 
     * a[3] * b[1] + a[4] * b[4] + a[5] * 0, 
     * a[3] * b[2] + a[4] * b[5] + a[5] * 1
     * ignoring values mult by 0
     * keeping values mult by 1
     */
    GMatrix concatenated = GMatrix(
        a[0] * b[0] + a[1] * b[3],        
        a[0] * b[1] + a[1] * b[4],         
        a[0] * b[2] + a[1] * b[5] + a[2],
        a[3] * b[0] + a[4] * b[3], 
        a[3] * b[1] + a[4] * b[4], 
        a[3] * b[2] + a[4] * b[5] + a[5]
    );
    return concatenated;
}

/*
    *  If this matrix is invertible, return true and (if not null) set the inverse parameter.
    *  If this matrix is not invertible, return false and ignore the inverse parameter.

    *inverse = (this)^-1
    */

static double dcross (double a, double b, double c, double d) {
    return a * b - c * d;
}
bool GMatrix::invert(GMatrix* inverse) const {
    // // similar to concat, ignore mult by 0 & keep mult by 1 for determinant
    // // square matrices are invertible if det != 0
    // float values[6] = {
    //     this->fMat[0],
    //     this->fMat[1],
    //     this->fMat[2],
    //     this->fMat[3],
    //     this->fMat[4],
    //     this->fMat[5]
    // };
    // float det = values[0]*values[4] - values[1]*values[3];
    // if (det == 0) {
    //     return false;
    // }
    // /**
    //  * https://math.stackexchange.com/questions/21533/shortcut-for-finding-a-inverse-of-matrix
    //  * A                   A inverse
    //  * | a b c |           | (e1-f0) -(b1-c0)  (bf-ce) |
    //  * | d e f | --> 1/det |-(d1-f0)  (a1-c0) -(af-cd) |
    //  * | g h i |           | (d0-e0) -(a0-b0)  (ae-bd) |
    //  * e = [4]
    //  * -b = -[1]
    //  * bf-ce = [1]*[5]-[2]*[4]
    //  * -d = -[3]
    //  * a = [0]
    //  * -(af-cd) = cd-af = [2]*[3] - [0]*[5]
    //  */
    // float oneOverDet = 1 / det;
    // inverse->fMat[0] = oneOverDet*values[4];
    // inverse->fMat[1] = oneOverDet*(0-values[1]);
    // inverse->fMat[2] = oneOverDet*(values[1]*values[5] - values[2]-values[4]);
    // inverse->fMat[3] = oneOverDet*(0-values[3]);
    // inverse->fMat[4] = oneOverDet*(values[0]);
    // inverse->fMat[5] = oneOverDet*(values[2]*values[3] - values[0]*values[5]);
    // return true;
    double det = dcross(fMat[SX], fMat[SY], fMat[KY], fMat[KX]);
    if (0==det) {
        return false;
    }
    double oneOverDet= 1/det;
    float a = fMat[SY] * oneOverDet;
    float b = -fMat[KX] * oneOverDet;
    float c = dcross(fMat[KX], fMat[TY], fMat[SY], fMat[TX]) * oneOverDet;
    float d = -fMat[KY]*oneOverDet;
    float e = fMat[SX]*oneOverDet;
    float f = dcross(fMat[KY], fMat[TX], fMat[SX], fMat[TY]) * oneOverDet;
    *inverse = GMatrix(a, b, c, d, e, f);
    return true;
}

/**
 *  Transform the set of points in src, storing the resulting points in dst, by applying this
 *  matrix. It is the caller's responsibility to allocate dst to be at least as large as src.
 *
 *  [ a  b  c ] [ x ]     x' = ax + by + c
 *  [ d  e  f ] [ y ]     y' = dx + ey + f
 *  [ 0  0  1 ] [ 1 ]
 *
 *  Note: It is legal for src and dst to point to the same memory (however, they may not
 *  partially overlap). Thus the following is supported.
 *
 *  GPoint pts[] = { ... };
 *  matrix.mapPoints(pts, pts, count);
 */
void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const {
    for (int i = 0; i < count; i++) {
        GPoint source = src[i];
        float x = this->fMat[0]*source.fX + this->fMat[1]*source.fY + this->fMat[2];
        float y = this->fMat[3]*source.fX + this->fMat[4]*source.fY + this->fMat[5];
        dst[i].set(x,y);
    }
}