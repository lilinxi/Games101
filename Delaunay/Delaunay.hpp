#ifndef DELAUNAY_HPP
#define DELAUNAY_HPP

#include <vector>
#include "Triangle.hpp"

using std::vector;

class Delaunay {
private:
    vector<Vector2d> _vertices;
    vector<Edge> _edges;
    vector<Triangle> _triangles;

public:
    Delaunay(vector<Vector2d> &vertices) {
        this->_vertices = vertices;
        this->triangulate();
    }

    const vector<Vector2d> &getVertices() const {
        return _vertices;
    }

    const vector<Edge> &getEdges() const {
        return _edges;
    }

    const vector<Triangle> &getTriangles() const {
        return _triangles;
    }

private:

    void triangulate() {
//        step0. Determinate the super triangle
        double minX = this->_vertices[0].x();
        double minY = this->_vertices[0].y();
        double maxX = minX;
        double maxY = minY;

        for (auto &v : this->_vertices) {
            if (v.x() < minX) minX = v.x();
            if (v.y() < minY) minY = v.y();
            if (v.x() > maxX) maxX = v.x();
            if (v.y() > maxY) maxY = v.y();
        }

        const double dx = maxX - minX;
        const double dy = maxY - minY;
        const double deltaMax = std::max(dx, dy);
        const double midx = (minX + maxX) / 2;
        const double midy = (minY + maxY) / 2;

        const Vector2d p1(midx - 20 * deltaMax, midy - deltaMax);
        const Vector2d p2(midx, midy + 20 * deltaMax);
        const Vector2d p3(midx + 20 * deltaMax, midy - deltaMax);

//        step1. Create a list of triangles, and add the super triangle in it
        this->_triangles.emplace_back(p1, p2, p3);

//        step2. loop for _vertices
        for (auto &v : this->_vertices) {
            std::vector<Edge> polygon;

//            step2.1. set bad and release bad edges
            for (auto &t : this->_triangles) {
                if (t.circumCircleContainsVertex(v)) {
                    t.setBad();
                    polygon.emplace_back(t.getA(), t.getB());
                    polygon.emplace_back(t.getB(), t.getC());
                    polygon.emplace_back(t.getC(), t.getA());
                }
            }

//            step2.2. remove bad triangles
            this->_triangles.erase(
                    std::remove_if(
                            begin(this->_triangles),
                            end(this->_triangles),
                            [](Triangle &t) {
                                return t.isBad();
                            }),
                    end(this->_triangles));

//            step2.3.1. set edges bad when shared by both bad triangles
            for (auto e1 = polygon.begin(); e1 != polygon.end(); e1++) {
                for (auto e2 = e1 + 1; e2 != polygon.end(); e2++) {
                    if (almost_equal(*e1, *e2)) {
                        e1->setBad();
                        e2->setBad();
                    }
                }
            }

//            step2.3.2. remove bad edges
            polygon.erase(
                    std::remove_if(
                            begin(polygon),
                            end(polygon),
                            [](Edge &e) {
                                return e.isBad();
                            }),
                    end(polygon));

//            step2.4. add triangle that per edge and loop vertices
            for (const auto &e : polygon) {
                this->_triangles.emplace_back(e.getU(), e.getV(), v);
            }
        }

//        step3. loop for _triangles remove _triangles contains super points
        this->_triangles.erase(
                std::remove_if(
                        begin(this->_triangles),
                        end(this->_triangles),
                        [p1, p2, p3](Triangle &t) {
                            return t.containsVertex(p1) ||
                                   t.containsVertex(p2) ||
                                   t.containsVertex(p3);
                        }),
                end(this->_triangles));

//        step4. trans _triangles to _edges
        for (const auto &t : this->_triangles) {
            this->_edges.emplace_back(t.getA(), t.getB());
            this->_edges.emplace_back(t.getB(), t.getC());
            this->_edges.emplace_back(t.getC(), t.getA());
        }
    }

};

#endif //DELAUNAY_HPP
