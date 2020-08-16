#ifndef OBJ_SIMPLER_HPP
#define OBJ_SIMPLER_HPP

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <utility>
#include <vector>
#include <queue>
#include <unordered_map>

#include <iostream>

using std::cout;
using std::endl;

using namespace Eigen;

template<typename _Matrix_Type_>
_Matrix_Type_ pseudoInverse(const _Matrix_Type_ &a, double epsilon =
std::numeric_limits<double>::epsilon()) {
    Eigen::JacobiSVD<_Matrix_Type_> svd(a, Eigen::ComputeThinU | Eigen::ComputeThinV);
    double tolerance = epsilon * std::max(a.cols(), a.rows()) * svd.singularValues().array().abs()(0);
    return svd.matrixV() *
           (svd.singularValues().array().abs() > tolerance)
                   .select(svd.singularValues().array().inverse(), 0).matrix().asDiagonal() *
           svd.matrixU().adjoint();
}

class Face;

class Point {
private:
    int _id;
    Vector3d _pos;
    std::vector<Face *> _FaceList;
    std::vector<Point *> _LinkList;

    static int ID;

public:
    friend std::ostream &operator<<(std::ostream &os, const Point &point) {
        os << "_id: " << point._id << " _pos: " << point._pos.transpose();
        return os;
    }

    friend std::ostream &operator<<(std::ostream &os, const Point *point) {
        os << "_id: " << point->_id << " _pos: " << point->_pos.transpose();
        return os;
    }

    static void ResetId() {
        Point::ID = 1;
    }

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    Point(Vector3d pos) : _pos(std::move(pos)) {
        this->_id = Point::ID++;
    }

    int getId() const {
        return this->_id;
    }

    const Vector3d &getPos() const {
        return _pos;
    }

    std::vector<Face *> getFaceList() const {
        return this->_FaceList;
    }

    void addFace(Face *face) {
        this->_FaceList.push_back(face);
    }

    std::vector<Point *> getLinkList() const {
        return this->_LinkList;
    }

    Point *addLinkPoint(Point *point) {
        this->_LinkList.push_back(point);
        return this;
    }

};

int Point::ID = 1;

class Face {
private:
    Point *_u, *_v, *_w; // 三个顶点的坐标
//    _abc * xyz + d = 0            三个顶点
//    _abc * xyz + d = d'           其他顶点到面的距离（计算距离需要除以的根号下a2+b2+c2=1）
//    (xyz,1) * kp * (xyz,1) = d2   计算距离的平方
    Vector3d _abc;
    double _a, _b, _c, _d;
    Matrix4d _kp;
    int _main_id = -1;
    bool _deleted = false;

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    Face(Point *u, Point *v, Point *w) : _u(u), _v(v), _w(w) {
        this->_main_id = std::max(this->_main_id, u->getId());
        this->_main_id = std::max(this->_main_id, v->getId());
        this->_main_id = std::max(this->_main_id, w->getId());
        this->computeKp();
    }

    Point *getU() const {
        return _u;
    }

    Point *getV() const {
        return _v;
    }

    Point *getW() const {
        return _w;
    }

    bool hasPointId(int id) {
        return this->_u->getId() == id || this->_v->getId() == id || this->_w->getId() == id;
    }

    void swapPoint(Point *a, Point *b) {
        if (a->getId() == this->_u->getId()) {
            this->_u = b;
        } else if (a->getId() == this->_v->getId()) {
            this->_v = b;
        } else if (a->getId() == this->_w->getId()) {
            this->_w = b;
        } else {
            cout << "swap point not found: " << a->getId() << endl;
            exit(-1);
        }
//        更换点需要计算主点和Kp
        this->_main_id = std::max(this->_main_id, b->getId());
        this->computeKp();
    }

//    一个面至于一个点绑定关系，id 最大的点，防止重复输出边
    bool mainPoint(Point *p) {
        return this->_main_id == p->getId();
    }

    Matrix4d getKp() {
        return this->_kp;
    }

    bool deleted() {
        return this->_deleted;
    }

    void deleteFace() {
        this->_deleted = true;
    }

//    点面绑定
    void bindPoint(Point *point) {
        if (this->_u->getId() != point->getId()) {
            point->addLinkPoint(this->_u);
        }
        if (this->_v->getId() != point->getId()) {
            point->addLinkPoint(this->_v);
        }
        if (this->_w->getId() != point->getId()) {
            point->addLinkPoint(this->_w);
        }
    }

private:
    void computeKp() {
        Vector3d a, b, c;
        a = this->_u->getPos();
        b = this->_v->getPos();
        c = this->_w->getPos();
        Vector3d p = (a - b).cross(a - c);
        p.normalize();
        double d = -(p.x() * a.x() + p.y() * a.y() + p.z() * a.z());
        this->_abc = p;
        this->_a = this->_abc.x();
        this->_b = this->_abc.y();
        this->_c = this->_abc.z();
        this->_d = d;
        Matrix4d kp;
        kp
                <<
                _a * _a, _a * _b, _a * _c, _a * _d,
                _a * _b, _b * _b, _b * _c, _b * _d,
                _a * _c, _b * _c, _c * _c, _c * _d,
                _a * _d, _b * _d, _c * _d, _d * _d;
        this->_kp = kp;
    }
};

class Edge {
private:
    double _cost;
    Point *_u, *_v;
    Point *_point;
    MatrixXd _Q; // pseudoInverse 的参数必须是 MatrixXd

public:
    friend std::ostream &operator<<(std::ostream &os, const Edge &edge) {
        os << "====================" << endl;
        os << "_cost: " << edge._cost << endl
           << " _u: " << edge._u << endl
           << " _v: " << edge._v << endl
           << " _point: " << edge._point << endl
           << " _Q: " << endl << edge._Q;
        os << endl << "====================";
        return os;
    }

    friend std::ostream &operator<<(std::ostream &os, const Edge *edge) {
        os << "====================" << endl;
        os << "_cost: " << edge->_cost << endl
           << " _u: " << edge->_u << endl
           << " _v: " << edge->_v << endl
           << " _point: " << edge->_point << endl
           << " _Q: " << endl << edge->_Q;
        os << endl << "====================";
        return os;
    }

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    Edge(Point *u, Point *v) : _u(u), _v(v) {
#ifdef ObjSimplerTimer
        auto start = std::chrono::system_clock::now();
#endif
        this->computeQ();
#ifdef ObjSimplerTimer
        auto stop = std::chrono::system_clock::now();
        std::cout << "compute Q: " << std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count()
                  << " microseconds\n";
        start = std::chrono::system_clock::now();
#endif
        this->computePointAndCost();
#ifdef ObjSimplerTimer
        stop = std::chrono::system_clock::now();
        std::cout << "compute Cost: " << std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count()
                  << " microseconds\n";
#endif
    }

    int getU() const {
        return this->_u->getId();
    }

    int getV() const {
        return this->_v->getId();
    }

    Point *getPoint() const {
        return this->_point;
    }

    double getCost() {
        return this->_cost;
    }

private:
//    矩阵 A*b + C*b = (A+C)*b
    void computeQ() {
        this->_Q = Matrix4d::Zero();
        for (auto iter:this->_u->getFaceList()) {
            this->_Q += iter->getKp();
        }
        for (auto iter:this->_v->getFaceList()) {
            this->_Q += iter->getKp();
        }
    }

    void computePointAndCost() {
        Vector4d expect{0, 0, 0, 1};
        Vector4d best = pseudoInverse(this->_Q) * expect;
        best /= best.w();
//        TODO：cost 太小误差很大，放大 cost
//        this->_cost = (best.transpose() * 100) * (this->_Q * 100) * (best * 100);
        this->_cost = best.transpose() * this->_Q * best;
        this->_point = new Point({best.x(), best.y(), best.z()});
    }
};

struct EdgeCompare {
    bool operator()(Edge *a, Edge *b) {
        return a->getCost() > b->getCost();      //与greater是等价的
    }
};

class ObjSimpler {
private:
    std::unordered_map<int, Point *> _PointMap;
    std::priority_queue<Edge *, std::vector<Edge *>, EdgeCompare> _EdgeHeap;

private:
    Point *deletePoint(int id) {
        auto iter = this->_PointMap.find(id);
        if (iter != this->_PointMap.end()) {
            this->_PointMap.erase(iter);
            return iter->second;
        }
//        return nullptr;
        cout << "delete point not found: " << id << endl;
        exit(-1);
    }

//    边为无效边，说明边的某一个顶点已被删除
    bool validEdge(Edge *edge) {
        if (this->_PointMap.find(edge->getU()) == this->_PointMap.end()) {
            return false;
        }
        if (this->_PointMap.find(edge->getV()) == this->_PointMap.end()) {
            return false;
        }
        return true;
    }

public:
    ObjSimpler() {
        Point::ResetId();
    }

    void addPoint(Point *point) {
        this->_PointMap[point->getId()] = point;
//        cout << point << endl;
    }

    void addEdge(Edge *edge) {
        this->_EdgeHeap.push(edge);
//        cout << edge->getCost() << endl;
    }

    const std::unordered_map<int, Point *> &getPointMap() const {
        return _PointMap;
    }

    Point *getPoint(int id) {
        if (this->_PointMap.find(id) == this->_PointMap.end()) {
            cout << "error id: " << id << endl;
            exit(-1);
        }
        return this->_PointMap[id];
    }

    void deleteOneEdge() {
//        step1. 删除边
#ifdef ObjSimplerTimer
        auto start = std::chrono::system_clock::now();
#endif
        Edge *leastEdge = this->_EdgeHeap.top();
//        只删除了点，没有删除相应边，所以可能会有已删除的边
        int count = 0;
        while (!validEdge(leastEdge)) {
            if (this->_EdgeHeap.empty()) {
                cout << "edge heap empty" << endl;
                exit(-1);
            }
            this->_EdgeHeap.pop();
            leastEdge = this->_EdgeHeap.top();
        }
        this->_EdgeHeap.pop();

#ifdef ObjSimplerDebug
        cout << leastEdge << endl;
#endif

        Point *u = this->deletePoint(leastEdge->getU());
        Point *v = this->deletePoint(leastEdge->getV());

        Point *newPoint = leastEdge->getPoint();

#ifdef ObjSimplerTimer
        auto stop = std::chrono::system_clock::now();
        std::cout << "search edge: " << std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count()
                  << " microseconds\n";
#endif

//        step2. 更新面
#ifdef ObjSimplerTimer
        start = std::chrono::system_clock::now();
#endif
        for (auto &iter : u->getFaceList()) {
            if (iter->hasPointId(v->getId()) || iter->deleted()) { // 包含u，v的面为即将被删除的面
//                需要从除了uv之外的第三点删除面，只删除一次即可
                iter->deleteFace();
                continue;
            } else {
                iter->swapPoint(u, newPoint);
                newPoint->addFace(iter);
                iter->bindPoint(newPoint);
            }
        }

        for (auto &iter : v->getFaceList()) {
            if (iter->hasPointId(u->getId()) || iter->deleted()) { // 包含u，v的面为即将被删除的面
                continue;
            } else {
                iter->swapPoint(v, newPoint);
                newPoint->addFace(iter);
                iter->bindPoint(newPoint);
            }
        }

#ifdef ObjSimplerTimer
        stop = std::chrono::system_clock::now();
        std::cout << "update faces: " << std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count()
                  << " microseconds\n";
#endif

#ifdef ObjSimplerDebug
        cout << u->getFaceList().size() << " + " << v->getFaceList().size() << " = "
             << newPoint->getFaceList().size() << " -4" << endl;
#endif

//        step3. 添加新点和新边
#ifdef ObjSimplerTimer
        start = std::chrono::system_clock::now();
#endif

        this->_PointMap[newPoint->getId()] = newPoint;

//        添加新边，并没有删除旧边，对边做了有效性的判定
        for (auto &iter : u->getLinkList()) {
            Edge *e = new Edge(iter, newPoint);
            this->_EdgeHeap.push(e);
        }
        for (auto &iter : v->getLinkList()) {
            Edge *e = new Edge(iter, newPoint);
            this->_EdgeHeap.push(e);
        }

#ifdef ObjSimplerTimer
        stop = std::chrono::system_clock::now();
        std::cout << "add point & edge: " << std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count()
                  << " microseconds\n";
#endif
    }
};

#endif //OBJ_SIMPLER_HPP
