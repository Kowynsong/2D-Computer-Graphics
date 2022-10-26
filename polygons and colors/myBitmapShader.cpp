#include "GBitmap.h"
#include "GMatrix.h"
#include "GShader.h"


class BitmapShader : public GShader {
public:
    BitmapShader(const GBitmap& bitmap, const GMatrix& localMatrix)
        : fDevice(bitmap)
        , fLocalMatrix(localMatrix) {}
    bool isOpaque() override {
        return false;
    }
    // fInverse becomes the inverse of concat of ctm + local matrix
    bool setContext(const GMatrix& ctm) override {
        return (ctm * fLocalMatrix).invert(&fInverse);
    }
    void shadeRow(int x, int y, int count, GPixel row[]) override {
        GPoint map[1] {x, y};
        fInverse.mapPoints(map, map, 1);
        GPoint local = map[0];

        for (int i = 0; i < count; ++i) {
            x = std::max(0, std::min(fDevice.width() - 1, GRoundToInt(local.fX)));
            y = std::max(0, std::min(fDevice.height() - 1, GRoundToInt(local.fY)));

            row[i] = *fDevice.getAddr(x, y);

            local.fX += fInverse[GMatrix::SX];
            local.fY += fInverse[GMatrix::KY];
        }
    }

private:
    GBitmap fDevice;
    GMatrix fInverse;
    GMatrix fLocalMatrix;
};

std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap& bitmap, const GMatrix& localMatrix) {
    if (!bitmap.pixels()) {
        return nullptr;
    }

    return std::unique_ptr<GShader>(new BitmapShader(bitmap, localMatrix));
}
