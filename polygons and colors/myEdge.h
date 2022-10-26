#ifndef myEdges_DEFINED
#define myEdges_DEFINED

#include <vector>
#include "GRect.h"
#include "GPoint.h"


struct Edge {
    int minY, maxY;
    float m, b, x;
};

Edge* makeEdge(GPoint p0, GPoint p1) {
    /** 
     * for each pair of p0 and p1, round their y values
     * because you need to know if a ray shooting across
     * the middle of a pixel intersects b/w 
     * the span of the Y for p0 and p1.
     * 
     * with the rounded Y values you know the
     * number of rows of pixels
     * you have to blit across.
     * where # of rows = p1.y-p0.y
     * 
     * re: sep10 clipping notes
     */
    
    int minY = GRoundToInt(std::min(p0.fY, p1.fY));
    int maxY = GRoundToInt(std::max(p0.fY, p1.fY));
    // int maxY = GRoundToInt(p1.fY) - 1;
    
    /**
     * since we are calculating the X coordinate
     * of intersection b/w the blit row and the edges
     * we should calculate X as a function of Y.
     * x = my + b where m = deltaX/deltaY
     * AND deltaY is never 0 since minY == maxY was
     * checked, so deltaY will never be 0, letting
     * us calculate m in such a manner
     */
    float m = ((p1.fX - p0.fX) / (p1.fY-p0.fY));
    // x = m*y + b --> b = x- m*y
    // float b = p0.x() - m*p0.y();
    float b = 0.1234;
    if (minY == GRoundToInt(p0.fY)) {
        b = p0.fX + m * (minY - p0.fY + 0.5);
    } else {
        b = p1.fX + m * (minY - p1.fY + 0.5);
    }

    Edge* edge = new Edge();
    edge->b = b;
    edge->m = m;
    edge->maxY = maxY;
    edge->minY = minY;
    // edge->x = (((minY+0.5) * m) + b);

    return edge;
}

bool legalY (int y, Edge edge) {
    return y>=edge.minY && y<edge.maxY;
}

/** 
 * sort edges such that
 * startY --> firstX --> slope
 * re: sep10 clipping notes
 * need to sort bc we need to know
 * which edge is on the left and
 * which edge is on the right
 */
bool compareEdges(Edge a, Edge b) {
    if (a.minY < b.minY) {
        return true;
    } 
    if (a.minY > b.minY) {
        return false;
    }
    // compare initial x value******
    if (GRoundToInt(a.b) < GRoundToInt(b.b)) {
        return true;
    } 
    if (GRoundToInt(a.b) > GRoundToInt(b.b)) {
        return false;
    }
    return a.m <= b.m;
}
bool compareEdgesX(Edge a, Edge b) {
    // compare initial x value******
    if (GRoundToInt(a.b) < GRoundToInt(b.b)) {
        return true;
    } 
    if (GRoundToInt(a.b) > GRoundToInt(b.b)) {
        return false;
    }
    return a.m <= b.m;
}
void clip(GRect bounds, GPoint p0, GPoint p1, std::vector<Edge>& edges) {
    // remember top left is (0,0)***
    /** 
     * if p0.y == p1.y
     * it is a horizontal line: legal
     * if p0.y < p1.y,
     * it is normal
     * if p0.y > p1.y,
     * flip the points
     */
    if (p0.fY > p1.fY) {
        std::swap(p0, p1);
    }

    // vertical pass
    /** 
     * if p0 is within bounds, return since it's legal
     *     if p1 was out of bounds it'll be passed in as p0 in the next
     *     for loop iteration
     * if p1 is within bounds, return because p0 is below p1
     *      if p0 was below the bounds then it'll be checked 
     *      in the last iteration of the for loop since
     *      points[i+1 % count] = points[0] when i = count 
     * 
     * since p0 is above p1, if:
     * p0 is below the bounds or p1 is above the bounds,
     * we can just ignore it since for either cases, if
     * one is true, then the other is automatically also
     * out of bounsd
     */

    // segment eliminated
    if (p0.fY >= bounds.bottom() || p1.fY <= bounds.top()) {
        return;
    }

    // segment chopped at top bound
    if (p0.fY < bounds.top()) {
        p0.set(
            p0.fX + (p1.fX - p0.fX) * (bounds.top() - p0.fY) / (p1.fY - p0.fY), 
            bounds.top()
        );
    }
    // segment chopped at bottom bound
    if (p1.fY > bounds.bottom()) {
        p1.set(
            p1.fX - (p1.fX - p0.fX) * (p1.fY - bounds.bottom()) / (p1.fY - p0.fY), 
            bounds.bottom()
        );
    }

    // confirm no horizontal edge before continuing
    // if (p0.y() == p1.y()) {
    //     return clippedEdges;
    // }
    
    // horizontal pass
    if (p0.fX > p1.fX) {
        std::swap(p0, p1);
    }

    // project to the left
    if (p1.fX <= bounds.left()) {
        edges.push_back(*makeEdge(
            GPoint::Make(bounds.left(), p0.fY),
            GPoint::Make(bounds.left(), p1.fY))
        );
        return;
    }

    // project to the right
    if (p0.fX >= bounds.right()) {
        edges.push_back(*makeEdge(
            GPoint::Make(bounds.right(), p0.fY),
            GPoint::Make(bounds.right(), p1.fY))
        );
        return;
    }

    // need to calculate new y values for projection
    // project and clip to the left
    if (p0.fX < bounds.left()) {
        /** 
         * new Y value = 
         * initial y + diff X for bounds & p0 * difference in y / difference in X for p1 & p0
         * y = b     + deltaX                 * deltaY          / deltaX
         * y = b     + deltaY
         * y = y int + deltaY
         */
        edges.push_back(*makeEdge(
            GPoint::Make(bounds.left(), p0.fY),
            GPoint::Make(bounds.left(), p0.fY + (bounds.left() - p0.fX) * (p1.fY - p0.fY)/(p1.fX - p0.fX)))
        );
        p0.set(bounds.left(), p0.fY + (bounds.left() - p0.fX) * (p1.fY - p0.fY)/(p1.fX - p0.fX));
    }
    // project and clip to the right
    if (p1.fX > bounds.right()) {
        /** 
         * new Y value = 
         * initial y - diff X for bounds & p1 * difference in y / difference in X for p1 & p0
         * y = b     - deltaX                 * deltaY          / deltaX
         * y = b     + deltaY
         */
        edges.push_back(*makeEdge(
            GPoint::Make(bounds.right(), p1.fY - (p1.fX - bounds.right()) * (p1.fY - p0.fY)/(p1.fX - p0.fX)),
            GPoint::Make(bounds.right(), p1.fY))
        );
        p1.set(bounds.right(), p1.fY - (p1.fX - bounds.right()) * (p1.fY - p0.fY)/(p1.fX - p0.fX));
    }

    // quick check to confirm # of rows is not 0.
    /** 
     * horizontal edges also go away bc our diagonal edges
     * will run out of Y at the same time
     * re: sep08 convex polygon notes
     */
    if (GRoundToInt(p0.y()) != GRoundToInt(p1.y())) {
        edges.push_back(*makeEdge(p0, p1));
    }
}

#endif