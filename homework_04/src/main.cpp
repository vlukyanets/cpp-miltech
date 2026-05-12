#include <iostream>
#include <fstream>


int main(int argc, char** argv) {
    // The program expects exactly one argument: a path to telemetry samples.
    if (argc != 2) {
        std::cerr << "usage: ugv_odometry <input_path>\n";
        return 1;
    }

    // The program should open input file by path from the argument
    std::ifstream ifs{argv[1]};
    if (!ifs.is_open()) {
        std::cerr << "cannot open file " << argv[1] << "\n";
        return 2;
    }


    return 0;
}
