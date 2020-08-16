#ifndef MESH_SIMPER_HPP
#define MESH_SIMPER_HPP

#include <string>
#include <fstream>

#include "ObjSimpler.hpp"

class MeshSimper {
private:
    int _face;
    ObjSimpler *_obj;

public:
    MeshSimper() {}

    void readInputObj(std::string filename) {
        this->_face = 0;
        this->_obj = new ObjSimpler();
        char flag;
        // 第一遍读取所有的数据，并初始化为 ObjSimpler 中的 _PointMap
        std::ifstream fin(filename.c_str());
        while (!fin.eof()) {
            fin >> flag;
            if (flag == '#') {
                std::string s;
                getline(fin, s);
                continue;
            }
            //verticals
            if (flag == 'v' || flag == 'V') {
                double tmp[3];
                fin >> tmp[0] >> tmp[1] >> tmp[2];
                Point *p = new Point({tmp[0], tmp[1], tmp[2]});
                this->_obj->addPoint(p);
            }
            //faces
            if (flag == 'f' || flag == 'F') {
                this->_face++;
                int tmp[3];
                fin >> tmp[0] >> tmp[1] >> tmp[2];
                Face *f = new Face(
                        this->_obj->getPoint(tmp[0]),
                        this->_obj->getPoint(tmp[1]),
                        this->_obj->getPoint(tmp[2])
                );
                this->_obj->getPoint(tmp[0])->addFace(f);
                this->_obj->getPoint(tmp[1])->addFace(f);
                this->_obj->getPoint(tmp[2])->addFace(f);
                this->_obj->getPoint(tmp[0])
                        ->addLinkPoint(this->_obj->getPoint(tmp[1]))
                        ->addLinkPoint(this->_obj->getPoint(tmp[2]));
                this->_obj->getPoint(tmp[1])
                        ->addLinkPoint(this->_obj->getPoint(tmp[0]))
                        ->addLinkPoint(this->_obj->getPoint(tmp[2]));
                this->_obj->getPoint(tmp[2])
                        ->addLinkPoint(this->_obj->getPoint(tmp[0]))
                        ->addLinkPoint(this->_obj->getPoint(tmp[1]));
            }
        }
        fin.close();
        // 第二遍读取所有的数据，并初始化为 ObjSimpler 中的 _EdgeHeap（需要在初始化所有的面片之后再初试化边）
        std::ifstream fin2(filename.c_str());
        while (!fin2.eof()) {
            fin2 >> flag;
            if (flag == '#') {
                std::string s;
                getline(fin2, s);
                continue;
            }
            //verticals
            if (flag == 'v' || flag == 'V') {
                std::string s;
                getline(fin2, s);
                continue;
            }
            //faces
            if (flag == 'f' || flag == 'F') {
                int tmp[3];
                fin2 >> tmp[0] >> tmp[1] >> tmp[2];
                this->_obj->addEdge(new Edge(
                        this->_obj->getPoint(tmp[0]),
                        this->_obj->getPoint(tmp[1])
                ));
                this->_obj->addEdge(new Edge(
                        this->_obj->getPoint(tmp[1]),
                        this->_obj->getPoint(tmp[2])
                ));
                this->_obj->addEdge(new Edge(
                        this->_obj->getPoint(tmp[2]),
                        this->_obj->getPoint(tmp[0])
                ));
            }
        }
        fin2.close();
    }

    void writeOutputObj(std::string filename, bool out = false) {
        std::ofstream fout(filename.c_str());
        std::unordered_map<int, Point *> pointMap = this->_obj->getPointMap();
        int id = 1;
        int faces = 0;
        std::unordered_map<int, int> idMap; // id 映射 map，将 id 映射到 1 ~ n
        std::unordered_map<int, int> idMapRev; // id 映射 map，将 id 映射到 1 ~ n
        for (auto iter:pointMap) {
            idMap[id] = iter.first;
            idMapRev[iter.first] = id;
            id++;
        }
        for (int i = 1; i < id; i++) {
            Vector3d pos = pointMap[idMap[i]]->getPos();
            fout << "v " << pos.x() << " " << pos.y() << " " << pos.z() << endl;
        }
        for (int i = 1; i < id; i++) {
            Point *point = pointMap[idMap[i]];
            for (auto iter:point->getFaceList()) {
//                每个面片只与一个id最大的主点绑定
                if (iter->mainPoint(point) && !iter->deleted()) {
                    faces += 1;
#ifdef MeshSimplerDebug
                    if (idMapRev.find(iter->getU()->getId()) == idMapRev.end()) {
                        cout << "U" << endl;
                        cout << point << endl;
                        cout << iter->getU() << endl;
                        cout << iter->getV() << endl;
                        cout << iter->getW() << endl;
                        cout << pointMap[iter->getU()->getId()] << endl;
                    }
                    if (idMapRev.find(iter->getV()->getId()) == idMapRev.end()) {
                        cout << "V" << endl;
                        cout << point << endl;
                        cout << iter->getU() << endl;
                        cout << iter->getV() << endl;
                        cout << iter->getW() << endl;
                        cout << pointMap[iter->getV()->getId()] << endl;
                    }
                    if (idMapRev.find(iter->getW()->getId()) == idMapRev.end()) {
                        cout << "W" << endl;
                        cout << point << endl;
                        cout << iter->getU() << endl;
                        cout << iter->getV() << endl;
                        cout << iter->getW() << endl;
                        cout << pointMap[iter->getW()->getId()] << endl;
                    }
#endif
                    fout << "f " << idMapRev[iter->getU()->getId()] << " "
                         << idMapRev[iter->getV()->getId()] << " "
                         << idMapRev[iter->getW()->getId()] << endl;
                }
            }
        }
        fout.close();
        if (out) {
            cout << "finish: ====================" << endl;
            cout << "# vertex count = " << id - 1 << endl;
            cout << "# face count = " << faces << endl;
        }
    }

    void simpler(double alpha) {
        int i = this->_face;
        while (i > this->_face * alpha) {
            this->shrink();
            i = i - 2;
        }
    }

    void simpler(int counts) {
        for (int i = 0; i < counts; i++) {
            this->shrink();
        }
    }

private:

    void shrink() {
        this->_obj->deleteOneEdge();
    }
};

#endif //MESH_SIMPER_HPP
