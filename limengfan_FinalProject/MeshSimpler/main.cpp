//#define ObjSimplerTimer

#include <sstream>
#include "MeshSimpler.hpp"

int main() {
    MeshSimper meshSimper;

   for (double i = 0.95; i > 0.0; i -= 0.05) {
       cout << "I: " << i << endl;
       std::stringstream ss;
       ss << i;
       std::string si;
       ss >> si;
       meshSimper.readInputObj("../models/input/bunny.obj");
       meshSimper.simpler(i);
       meshSimper.writeOutputObj("../models/output/bunny" + si + ".obj", true);
   }

    return 0;
}
