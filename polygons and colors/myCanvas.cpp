#include <stack>
#include "GBitmap.h"
#include "GCanvas.h"
#include "GPoint.h"
#include "GRect.h"
#include "GMatrix.h"
#include "GShader.h"

#include "myBlend.h"
#include "myEdge.h"

class EmptyCanvas : public GCanvas {
public:
    
    /**
     * : is an initializer list 
     * (const GBitmap& device) : fDevice(device)
     * ---------- is identical to ----------
     * EmptyCanvas(const GBitmap& device) {fDevice = device;}
     */
    EmptyCanvas(const GBitmap& device) : fDevice(device) {
        // initial state
        ctm.push(GMatrix());
    }

    void save() {
        //dupe the top of stack
        GMatrix saved = ctm.top();
        ctm.push(GMatrix(saved[0], saved[1], saved[2], saved[3], saved[4], saved[5]));
    }

    void restore() {
        ctm.pop();
    }

    void concat(const GMatrix& matrix) {
        ctm.top().preConcat(matrix);
    }

    void drawPaint(const GPaint& paint) {
        // GPixel pixel = convert(paint.getColor());
        // for (int i = 0; i < fDevice.height; i++) {

        // }
        
        // convert paint to color to pixel
        GPixel clearPixel = convert(paint.getColor().pinToUnit());
        // "clear canvas" by coloring each pixel with the given bitmap's pixels
        for (int height = 0; height < fDevice.height(); height++) {
            for (int width = 0; width < fDevice.width(); width++) {
                GPixel* dst = fDevice.getAddr(width, height);
                *dst = clearPixel;
            }
        }
    } 

    void drawRect(const GRect& rect, const GPaint& paint) {
        /* 
        * turning GRect to GIRect by rounding 
        * (canvas is defined in floats so i need to convert
        * rect dimensions to ints by rounding to clip the area
        * that goes over the boundaries of the canvas)
        */
        // GIRect rounded = rect.round();
        // // clipping area
        // rounded.fLeft = std::max(rounded.fLeft, 0);
        // rounded.fTop = std::max(rounded.fTop, 0);
        // rounded.fRight = std::min(rounded.fRight, fDevice.width());
        // rounded.fBottom = std::min(rounded.fBottom, fDevice.height());
        // // convert paint to color to pixel
        // GPixel fill = convert(paint.getColor().pinToUnit());
        // BlendMode blender = getBM(paint.getBlendMode(), fill);
        // // "paint" or rather, fill pixel by pixel using source-over-destination
        // // convention 1: top left corner is (0,0) so paint from Left Top to Right Bottom (LTRB)
        // for (int y = rounded.fTop; y < rounded.fBottom; y++) {
        //     for (int x = rounded.fLeft; x < rounded.fRight; x++) {
        //         // read the dst pixel
        //         GPixel* dst = fDevice.getAddr(x, y);
        //         // blend the src color with it & write the new color/pixel
        //         *dst = blender(fill, *dst);
        //     }
        // }
        GPoint rectPoints[4] = {
            GPoint::Make(rect.left(), rect.top()),
            GPoint::Make(rect.right(), rect.top()),
            GPoint::Make(rect.right(), rect.bottom()),
            GPoint::Make(rect.left(), rect.bottom())
        };
        drawConvexPolygon(rectPoints, 4, paint);
    }

    void drawConvexPolygon(const GPoint points[], int count, const GPaint& paint) {
        GRect bounds = GRect::MakeWH(fDevice.width(), fDevice.height());
        std::vector<Edge> edges;
        GPoint matrixPoints[count];
        ctm.top().mapPoints(matrixPoints, points, count);

        for (int i = 0; i < count; i++) {
            GPoint p0 = matrixPoints[i];
            GPoint p1 = matrixPoints[(i + 1) % count];

            clip(bounds, p0, p1, edges);
        }

        std::sort(edges.begin(), edges.end(), compareEdges);

        // quick check to see if any edges remain
        if (edges.size() == 0) {
            return;
        } 

        Edge left = edges.at(0);
        Edge right = edges.at(1);
        int index = 2;

        for (int y = left.minY; y < edges.back().maxY; y++) {
            // assert(legalY(curY, left));
            bool edgeUpdated = false;
            if (y >= left.maxY) {
                left = edges.at(index);
                index++;
                edgeUpdated = true;
            }
            if (y >= right.maxY) {
                right = edges.at(index);
                index++;
                edgeUpdated = true;
            }
            // to solve issues when edges appear equal bc they have the same y & slope
            if (edgeUpdated && !(compareEdgesX(left, right))) {
                std::swap(left, right);
            }

            float leftX = left.b;
            float rightX = right.b;

            // GRect blitter = GRect::MakeLTRB(GRoundToInt(leftX), y, GRoundToInt(rightX), y+1);
            // drawRect(blitter, paint);
            blit(y, GRoundToInt(leftX), GRoundToInt(rightX), paint);
            left.b+=left.m;
            right.b+=right.m; 
        }
    }
    void clear(const GColor& color) {
        GPaint paint(color);
        paint.setBlendMode(GBlendMode::kSrc);
        this->drawPaint(paint);
    }

    void fillRect(const GRect& rect, const GColor& color) {
        this->drawRect(rect, GPaint(color));
    }
    void blit(int y, int leftX, int rightX, const GPaint& paint) {

        BlendMode blender = getBM(paint.getBlendMode());
        GShader* shader = paint.getShader();

        if (shader != nullptr && shader->setContext(ctm.top())) {
            int count = rightX - leftX;
            GPixel row[count];
            shader->shadeRow(leftX, y, count, row);

            for (int x = leftX; x < rightX; x++) {
                GPixel* addr = fDevice.getAddr(x, y);
                *addr = blender(row[x - leftX], *addr);
            }
        } else {
            GPixel source = convert(paint.getColor().pinToUnit());

            for (int x = leftX; x < rightX; x++) {
                GPixel* addr = fDevice.getAddr(x, y);
                *addr = blender(source, *addr);
            }
            return;
        }
    }
private:
    const GBitmap fDevice;
    std::stack<GMatrix> ctm;
};

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device) {
    if (!device.pixels()) {
        return nullptr;
    }
    return std::unique_ptr<GCanvas>(new EmptyCanvas(device));
}
static void draw_bitmap(GCanvas* canvas, const GRect& r, const GBitmap& bm, GBlendMode mode) {
    GPaint paint;
    paint.setBlendMode(mode);
    
    GMatrix m = GMatrix::Translate(r.left(), r.top())
              * GMatrix::Scale(r.width() / bm.width(), r.height() / bm.height());
    auto sh = GCreateBitmapShader(bm, m);
    paint.setShader(sh.get());
    canvas->drawRect(r, paint);
}
std::string GDrawSomething(GCanvas* canvas, GISize dim) {
    GBitmap background, foreground;
    
    background.readFromFile("apps/black.png");
    foreground.readFromFile("apps/logo.png");


    const GRect bounds = GRect::MakeWH(254, 254);
    draw_bitmap(canvas, bounds, background, GBlendMode::kSrc);
    draw_bitmap(canvas, bounds, foreground, GBlendMode::kSrcOver);
    
    return "the worst app";
}