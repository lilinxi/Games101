//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_BOUNDS3_H
#define RAYTRACING_BOUNDS3_H

#include "Ray.hpp"
#include "Vector.hpp"
#include <limits>
#include <array>

//Bounds3.hpp: 包围盒类，每个包围盒可由 pMin 和 pMax 两点描述
class Bounds3 {
public:
    Vector3f pMin, pMax; // two points to specify the bounding box
    Bounds3() {
        double minNum = std::numeric_limits<double>::lowest();
        double maxNum = std::numeric_limits<double>::max();
        pMax = Vector3f(minNum, minNum, minNum);
        pMin = Vector3f(maxNum, maxNum, maxNum);
    }

    Bounds3(const Vector3f p) : pMin(p), pMax(p) {}

    Bounds3(const Vector3f p1, const Vector3f p2) {
        pMin = Vector3f(fmin(p1.x, p2.x), fmin(p1.y, p2.y), fmin(p1.z, p2.z));
        pMax = Vector3f(fmax(p1.x, p2.x), fmax(p1.y, p2.y), fmax(p1.z, p2.z));
    }

//    对角线
    Vector3f Diagonal() const { return pMax - pMin; }

//    最长度的维度
    int maxExtent() const {
        Vector3f d = Diagonal();
        if (d.x > d.y && d.x > d.z)
            return 0;
        else if (d.y > d.z)
            return 1;
        else
            return 2;
    }

//    表面积
    double SurfaceArea() const {
        Vector3f d = Diagonal();
        return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
    }

//    质心
    Vector3f Centroid() { return 0.5 * pMin + 0.5 * pMax; }

//    相交的包围盒
    Bounds3 Intersect(const Bounds3 &b) {
        return Bounds3(
                Vector3f(
                        fmax(pMin.x, b.pMin.x),
                        fmax(pMin.y, b.pMin.y),
                        fmax(pMin.z, b.pMin.z)),
                Vector3f(
                        fmin(pMax.x, b.pMax.x),
                        fmin(pMax.y, b.pMax.y),
                        fmin(pMax.z, b.pMax.z)));
    }

//    以包围盒内的坐标为0~1标准坐标系计算出来的点坐标
    Vector3f Offset(const Vector3f &p) const {
        Vector3f o = p - pMin;
        if (pMax.x > pMin.x)
            o.x /= pMax.x - pMin.x;
        if (pMax.y > pMin.y)
            o.y /= pMax.y - pMin.y;
        if (pMax.z > pMin.z)
            o.z /= pMax.z - pMin.z;
        return o;
    }

//    包围盒b1是否包含包围盒b2
    bool Overlaps(const Bounds3 &b1, const Bounds3 &b2) {
        bool x = (b1.pMax.x >= b2.pMin.x) && (b1.pMin.x <= b2.pMax.x);
        bool y = (b1.pMax.y >= b2.pMin.y) && (b1.pMin.y <= b2.pMax.y);
        bool z = (b1.pMax.z >= b2.pMin.z) && (b1.pMin.z <= b2.pMax.z);
        return (x && y && z);
    }

//    点是否在包围盒内
    bool Inside(const Vector3f &p, const Bounds3 &b) {
        return (p.x >= b.pMin.x && p.x <= b.pMax.x && p.y >= b.pMin.y &&
                p.y <= b.pMax.y && p.z >= b.pMin.z && p.z <= b.pMax.z);
    }

    inline const Vector3f &operator[](int i) const {
        return (i == 0) ? pMin : pMax;
    }

    inline bool IntersectP(const Ray &ray, const Vector3f &invDir,
                           const std::array<int, 3> &dirisNeg) const;
};


inline bool Bounds3::IntersectP(const Ray &ray, const Vector3f &invDir,
                                const std::array<int, 3> &dirIsNeg) const {
    // invDir: ray direction(x,y,z), invDir=(1.0/x,1.0/y,1.0/z), use this because Multiply is faster that Division
    // dirIsNeg: ray direction(x,y,z), dirIsNeg=[int(x>0),int(y>0),int(z>0)], use this to simplify your logic
    // TODO test if ray bound intersects
// 光线：（x + t·dx，y + t·dy，z + t·dy）
    // Z 面：x-y-zMin ~ x-y-zMax
    double tzMin = (this->pMin.z - ray.origin.z) * ray.direction_inv.z;
    double tzMax = (this->pMax.z - ray.origin.z) * ray.direction_inv.z;
    if (!dirIsNeg[2]) {
        double tmp = tzMin;
        tzMin = tzMax;
        tzMax = tmp;
    }

    // Y 面：x-yMin-z ~ x-yMax-z
    double tyMin = (this->pMin.y - ray.origin.y) * ray.direction_inv.y;
    double tyMax = (this->pMax.y - ray.origin.y) * ray.direction_inv.y;
    if (!dirIsNeg[1]) {
        double tmp = tyMin;
        tyMin = tyMax;
        tyMax = tmp;
    }

    // X 面：xMin-y-z ~ xMax-y-z
    double txMin = (this->pMin.x - ray.origin.x) * ray.direction_inv.x;
    double txMax = (this->pMax.x - ray.origin.x) * ray.direction_inv.x;
    if (!dirIsNeg[0]) {
        double tmp = txMin;
        txMin = txMax;
        txMax = tmp;
    }

    double tEnter = fmax(fmax(txMin, tyMin), tzMin);
    double tExit = fmin(fmin(txMax, tyMax), tzMax);
    return tEnter < tExit && tExit >= 0;
}

// 两个包围盒合并
inline Bounds3 Union(const Bounds3 &b1, const Bounds3 &b2) {
    Bounds3 ret;
    ret.pMin = Vector3f::Min(b1.pMin, b2.pMin);
    ret.pMax = Vector3f::Max(b1.pMax, b2.pMax);
    return ret;
}

//包围盒包括了新点变成了一个更大的包围盒
inline Bounds3 Union(const Bounds3 &b, const Vector3f &p) {
    Bounds3 ret;
    ret.pMin = Vector3f::Min(b.pMin, p);
    ret.pMax = Vector3f::Max(b.pMax, p);
    return ret;
}

#endif // RAYTRACING_BOUNDS3_H
