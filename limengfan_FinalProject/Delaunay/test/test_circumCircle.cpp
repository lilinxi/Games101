#include <random>
#include "../Triangle.hpp"

int main() {
    std::default_random_engine eng(std::random_device{}());
    std::uniform_real_distribution<double> dist(0, 100);

    Vector2d a{dist(eng), dist(eng)};
    Vector2d b{dist(eng), dist(eng)};
    Vector2d c{dist(eng), dist(eng)};

    Triangle t(a, b, c);

    cout << t << endl;

    cout << "a: " << (t.getCenter() - a).norm()<<", "<<t.circumCircleContainsVertex(a) << endl;
    cout << "b: " << (t.getCenter() - b).norm() <<", "<<t.circumCircleContainsVertex(b) << endl;
    cout << "c: " << (t.getCenter() - c).norm() <<", "<<t.circumCircleContainsVertex(c) << endl;

    Vector2d d{dist(eng), dist(eng)};
    cout << "d: " << d.transpose() << endl;
    cout << "d: " << (t.getCenter() - d).norm() <<", "<<t.circumCircleContainsVertex(d) << endl;


    return 0;
}