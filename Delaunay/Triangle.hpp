#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#include <Eigen/Core>
#include <limits>
#include <type_traits>

#include <iostream>

using std::cout;
using std::endl;

using namespace Eigen;

//浮点数相等：https://en.cppreference.com/w/cpp/types/numeric_limits/epsilon
template<class T>
typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
almost_equal(T x, T y, int ulp = 2) {
    // the machine epsilon has to be scaled to the magnitude of the values used
    // and multiplied by the desired precision in ULPs (units in the last place)
    return std::fabs(x - y) <= std::numeric_limits<T>::epsilon() * std::fabs(x + y) * ulp
           // unless the result is subnormal
           || std::fabs(x - y) < std::numeric_limits<T>::min();
}

bool almost_equal(const Vector2d &v1, const Vector2d &v2) {
    return (almost_equal(v1.x(), v2.x()) && almost_equal(v1.y(), v2.y()));
//    return (almost_equal(v1.x(), v2.x()) && almost_equal(v1.y(), v2.y())) ||
//           (almost_equal(v1.x(), v2.y()) && almost_equal(v1.y(), v2.x()));
}

class Edge {
private:
    Vector2d _u, _v;
    bool _bad = false;
public:
    Edge(const Vector2d &u, const Vector2d &v) : _u(u), _v(v) {}

    const Vector2d &getU() const {
        return _u;
    }

    const Vector2d &getV() const {
        return _v;
    }

    void setBad() {
        this->_bad = true;
    }

    bool isBad() {
        return this->_bad;
    }
};

bool almost_equal(const Edge &e1, const Edge &e2) {
    return (almost_equal(e1.getU(), e2.getU()) && almost_equal(e1.getV(), e2.getV())) ||
           (almost_equal(e1.getU(), e2.getV()) && almost_equal(e1.getV(), e2.getU()));
}

class Triangle {
private:
    Vector2d _a, _b, _c;
    Vector2d _center; // 外接圆圆心
    double _redis; // 外接圆半径
    bool _bad = false;

private:
//    计算外接圆：https://blog.csdn.net/MallowFlower/article/details/79919797
//    令:A1=2*(x2-x1)；
//    B1=2*(y2-y1)；
//    C1=x2^2+y2^2-x1^2-y1^2;
//    A2=2*(x3-x2)；
//    B2=2*(y3-y2)；
//    C2=x3^2+y3^2-x2^2-y2^2;
//    即:A1*x+B1y=C1;
//    A2*x+B2y=C2;
//    最后根据克拉默法则：
//    x=((C1*B2)-(C2*B1))/((A1*B2)-(A2*B1))；
//    y=((A1*C2)-(A2*C1))/((A1*B2)-(A2*B1))；
    void computeCircumCircle() {
        double x1, y1, x2, y2, x3, y3;
        x1 = this->_a.x();
        y1 = this->_a.y();
        x2 = this->_b.x();
        y2 = this->_b.y();
        x3 = this->_c.x();
        y3 = this->_c.y();
        double A1, B1, C1, A2, B2, C2;
        A1 = 2 * (x2 - x1);
        B1 = 2 * (y2 - y1);
        C1 = x2 * x2 + y2 * y2 - x1 * x1 - y1 * y1;
        A2 = 2 * (x3 - x2);
        B2 = 2 * (y3 - y2);
        C2 = x3 * x3 + y3 * y3 - x2 * x2 - y2 * y2;
        double x, y;
        x = ((C1 * B2) - (C2 * B1)) / ((A1 * B2) - (A2 * B1));
        y = ((A1 * C2) - (A2 * C1)) / ((A1 * B2) - (A2 * B1));
        this->_center = Vector2d{x, y};
        this->_redis = (this->_center - this->_a).norm();
    }

public:
    Triangle(const Vector2d &a, const Vector2d &b, const Vector2d &c) : _a(a), _b(b), _c(c) {
        this->computeCircumCircle();
    }

    const Vector2d &getA() const {
        return _a;
    }

    const Vector2d &getB() const {
        return _b;
    }

    const Vector2d &getC() const {
        return _c;
    }

    const Vector2d &getCenter() const {
        return _center;
    }

    double getRedis() const {
        return _redis;
    }

    void setBad() {
        this->_bad = true;
    }

    bool isBad() {
        return this->_bad;
    }

    friend std::ostream &operator<<(std::ostream &os, const Triangle &triangle) {
        os << "_a: " << triangle._a.transpose() << endl
           << " _b: " << triangle._b.transpose() << endl
           << " _c: " << triangle._c.transpose() << endl
           << " _center: " << triangle._center.transpose() << " _redis: " << triangle._redis;
        return os;
    }

    bool containsVertex(const Vector2d &v) const {
        return almost_equal(this->_a, v) || almost_equal(this->_b, v) || almost_equal(this->_c, v);
    }

    bool circumCircleContainsVertex(const Vector2d &v) const {
        double distance = (this->_center - v).norm();
        return distance <= this->_redis;
    }
};

bool almost_equal(const Triangle &t1, const Triangle &t2) {
    return (almost_equal(t1.getA(), t2.getA()) || almost_equal(t1.getA(), t2.getB()) ||
            almost_equal(t1.getA(), t2.getC())) &&
           (almost_equal(t1.getB(), t2.getA()) || almost_equal(t1.getB(), t2.getB()) ||
            almost_equal(t1.getB(), t2.getC())) &&
           (almost_equal(t1.getC(), t2.getA()) || almost_equal(t1.getC(), t2.getB()) ||
            almost_equal(t1.getC(), t2.getC()));
}

#endif //TRIANGLE_HPP
