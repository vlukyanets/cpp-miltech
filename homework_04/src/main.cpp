#include <iostream>
#include <iomanip>
#include <cmath>
#include <fstream>

// Robot params
namespace robot::params {
    constexpr long ticks_per_revolution = 1024;
    constexpr double wheel_radius_m = 0.3;
    constexpr double wheelbase_m = 1.0;
    // Also constant value
    constexpr double distance_per_tick = 2.0 * M_PI * wheel_radius_m / ticks_per_revolution;
}

// Struct that contains robot position and current timestamp
struct RobotPosition {
    long timestamp_ms;
    double x;
    double y;
    double theta;

    // Update robot position method
    void update(const long ts, const double d, const double dtheta) {
        timestamp_ms = ts;
        x += d * std::cos(theta + dtheta / 2.0);
        y += d * std::sin(theta + dtheta / 2.0);
        theta += dtheta;
    }
};

// Print RobotPosition to output stream
std::ostream& operator<<(std::ostream& os, const RobotPosition& rp) {
    os << rp.timestamp_ms << " " << rp.x << " " << rp.y << " " << rp.theta;
    return os;
}

// Struct that contains odometry reading and timestamp
struct RobotOdometryReading {
    long timestamp_ms;
    long fl_ticks;
    long fr_ticks;
    long bl_ticks;
    long br_ticks;
};

// Read RobotOdometryReading from input stream
std::istream& operator>>(std::istream& is, RobotOdometryReading& ror) {
    is >> ror.timestamp_ms >> ror.fl_ticks >> ror.fr_ticks >> ror.bl_ticks >> ror.br_ticks;
    return is;
}

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

    // Read first reading
    ifs >> prev_reading;

    // Read next readings until end-of-file
    while (ifs >> current_reading) {
        // Do math
        const long d_fl = current_reading.fl_ticks - prev_reading.fl_ticks;
        const long d_fr = current_reading.fr_ticks - prev_reading.fr_ticks;
        const long d_bl = current_reading.bl_ticks - prev_reading.bl_ticks;
        const long d_br = current_reading.br_ticks - prev_reading.br_ticks;

        const double d_left = (d_fl + d_bl) / 2.0;
        const double d_right = (d_fr + d_br) / 2.0;

        const double dl = d_left * robot::params::distance_per_tick;
        const double dr = d_right * robot::params::distance_per_tick;

        const double d = (dl + dr) / 2.0;
        const double dtheta = (dr - dl) / robot::params::wheelbase_m;

        // Update robot position info
        robot_position.update(current_reading.timestamp_ms, d, dtheta);

        // Output robot position info
        std::cout << std::setprecision(4) << robot_position << std::endl;

        // Current reading become previous on the next iteration of loop
        prev_reading = current_reading;
    }

    return 0;
}
