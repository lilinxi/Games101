//#define ObjSimplerTimer

#include <sstream>
#include "MeshSimpler.hpp"

int main() {
    MeshSimper meshSimper;
    meshSimper.readInputObj("../models/input/bunny.obj");
    auto start = std::chrono::system_clock::now();
    meshSimper.simpler(0.05);
    auto stop = std::chrono::system_clock::now();
    std::cout << "costs: " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count()
              << " seconds\n";
    meshSimper.writeOutputObj("../models/output/bunny0.05.obj", true);

//    for (double i = 0.95; i > 0.0; i -= 0.05) {
//        cout << "I: " << i << endl;
//        std::stringstream ss;
//        ss << i;
//        std::string si;
//        ss >> si;
//        meshSimper.readInputObj("../models/input/bunny.obj");
//        meshSimper.simpler(i);
//        meshSimper.writeOutputObj("../models/output/bunny" + si + ".obj", true);
//    }

//    for (double i = 0.1; i > 0.0; i -= 0.01) {
//        cout << "I: " << i << endl;
//        std::stringstream ss;
//        ss << i;
//        std::string si;
//        ss >> si;
//        meshSimper.readInputObj("../models/input/bunny.obj");
//        meshSimper.simpler(i);
//        meshSimper.writeOutputObj("../models/output/bunny" + si + ".obj", true);
//    }

//    for (double i = 0.95; i > 0.0; i -= 0.05) {
//        cout << "I: " << i << endl;
//        std::stringstream ss;
//        ss << i;
//        std::string si;
//        ss >> si;
//        meshSimper.readInputObj("../models/input/armadillo.obj");
//        meshSimper.simpler(i);
//        meshSimper.writeOutputObj("../models/output/armadillo" + si + ".obj", true);
//    }

//    for (int i = 1000; i <= 2000; i += 10) {
//        cout << "I: " << i << endl;
//        std::stringstream ss;
//        ss << i;
//        std::string si;
//        ss >> si;
//        meshSimper.readInputObj("../models/input/bunny.obj");
//        meshSimper.simpler(i);
//        meshSimper.writeOutputObj("../models/output/bunny_" + si + ".obj", true);
//    }

    return 0;
}
