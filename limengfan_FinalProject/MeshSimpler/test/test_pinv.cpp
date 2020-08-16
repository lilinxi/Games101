#include <Eigen/Core>
#include "../ObjSimpler.hpp"

#include <iostream>

using std::cout;
using std::endl;

using namespace Eigen;

int main() {
    Matrix4d kp_tmp = Matrix4d::Random();
    MatrixXd kp = kp_tmp;
    cout << "kp=" << endl << kp << endl;
    cout << "kp'=" << endl << pseudoInverse(kp) << endl;
    cout << "kp'=" << endl << kp.inverse() << endl;
}