#include <iostream>
#include <fstream>

// Struct that contains robot position and current timestamp
struct RobotPosition {
    long timestamp_ms;
    double x;
    double y;
    double theta;
};

// Struct that contains odometry reading and timestamp
struct RobotOdometryReading {
    long timestamp_ms;
    long fl_ticks;
    long fr_ticks;
    long bl_ticks;
    long br_ticks;
};

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

    // Initialize with zeroes robot position
    RobotPosition robot_position{};
    // Use pair of consecutive odometry readings for wheel odometry math
    RobotOdometryReading prev_reading, current_reading;

    return 0;
}
