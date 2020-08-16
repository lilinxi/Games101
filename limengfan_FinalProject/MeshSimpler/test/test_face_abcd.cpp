#include <Eigen/Core>
#include <Eigen/Dense>

#include <iostream>

using std::cout;
using std::endl;

using namespace Eigen;

int main() {
    Vector3d a, b, c;
    a = MatrixXd::Random(3, 1);
    b = MatrixXd::Random(3, 1);
    c = MatrixXd::Random(3, 1);
    cout << a.transpose() << endl;
    cout << b.transpose() << endl;
    cout << c.transpose() << endl;
    Vector3d p = (a - b).cross(a - c);
    cout << p.transpose() << endl;
    cout << p.transpose() * (a - b) << endl;
    cout << p.transpose() * (a - c) << endl;
    p.normalize();
    cout << p.transpose() << endl;
    double d = -(p.x() * a.x() + p.y() * a.y() + p.z() * a.z());
    cout << "d = " << d << endl;
    cout << p.transpose() * a << endl;
    cout << p.transpose() * b << endl;
    cout << p.transpose() * c << endl;

    cout << p.transpose() * (a + p) + d << " = 1" << endl;
    cout << p.transpose() * (b + p) + d << " = 1" << endl;
    cout << p.transpose() * (c + p) + d << " = 1" << endl;

    double _a = p.x();
    double _b = p.y();
    double _c = p.z();
    double _d = d;

    Matrix4d kp;
    kp
            <<
            _a * _a, _a * _b, _a * _c, _a * _d,
            _a * _b, _b * _b, _b * _c, _b * _d,
            _a * _c, _b * _c, _c * _c, _c * _d,
            _a * _d, _b * _d, _c * _d, _d * _d;
    cout << "kp=" << endl << kp << endl;

    Vector4d test{(a + p).x(), (a + p).y(), (a + p).z(), 1};
    cout << "test = " << test.transpose() << endl;
    cout << test.transpose() * kp * test << " = 1" << endl;

    Vector4d test1{(a + p * -1.5).x(), (a + p * -1.5).y(), (a + p * -1.5).z(), 1};
    cout << "test1 = " << test1.transpose() << endl;
    cout << test1.transpose() * kp * test1 << " = 2.25" << endl;
}