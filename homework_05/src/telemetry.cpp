#include "telemetry.hpp"

#include <limits>
#include <cmath>
#include <cstdlib>
#include <cerrno>
#include <fstream>
#include <iostream>

const int EXPECTED_FIELD_COUNT = 7;
const int MAX_LINE_LENGTH = 256;

int split_line(char line[], char* fields[], int max_fields) {
    int count = 0;
    char* cursor = line;

    while (*cursor != '\0' && count < max_fields) {
        while (*cursor == ' ' || *cursor == '\t' || *cursor == '\n' || *cursor == '\r') {
            *cursor = '\0';
            ++cursor;
        }

        if (*cursor == '\0') {
            break;
        }

        fields[count] = cursor;
        ++count;

        while (*cursor != '\0' && *cursor != ' ' && *cursor != '\t' && *cursor != '\n' &&
               *cursor != '\r') {
            ++cursor;
        }
    }

    return count;
}

bool parse_long(const char* text, long& result) {
    if (!text || *text == '\0') return false;

    char* end = nullptr;
    errno = 0;
    const long val = std::strtol(text, &end, 10);

    if ((val == std::numeric_limits<long>::min() || val == std::numeric_limits<long>::max()) && errno == ERANGE)
        return false;

    if (text == end)
        return false;

    if (*end != '\0')
        return false;

    result = val;
    return true;
}

int parse_int(const char* text, int& result) {
    long res;
    bool parse_result = parse_long(text, res);
    result = static_cast<int>(res);
    return parse_result;
}

bool parse_double(const char* text, double& result) {
    if (!text || *text == '\0') return false;

    char* end = nullptr;
    errno = 0;

    const double val = std::strtod(text, &end);

    if (errno == ERANGE || text == end || *end != '\0' || std::isinf(val) || std::isnan(val))
        return false;

    result = val;
    return true;
}

bool parse_frame(char line[], Frame& frame) {
    char* fields[EXPECTED_FIELD_COUNT] = {};
    int field_count = split_line(line, fields, EXPECTED_FIELD_COUNT);
    if (field_count != EXPECTED_FIELD_COUNT) {
        std::cerr << "Expected " << EXPECTED_FIELD_COUNT << " fields but got " << field_count << '\n';
        return false;
    }

    if (!parse_long(fields[0], frame.timestamp_ms)) {
        std::cerr << "Failed to read timestamp_ms field\n";
        return false;
    }
    if (!parse_int(fields[1], frame.seq)) {
        std::cerr << "Failed to read seq field\n";
        return false;
    }
    if (!parse_double(fields[2], frame.voltage_v)) {
        std::cerr << "Failed to read voltage_v field\n";
        return false;
    }
    if (!parse_double(fields[3], frame.current_a)) {
        std::cerr << "Failed to read current_a field\n";
        return false;
    }
    if (!parse_double(fields[4], frame.temperature_c)) {
        std::cerr << "Failed to read temperature_c field\n";
        return false;
    }
    if (!parse_int(fields[5], frame.gps_fix)) {
        std::cerr << "Failed to read gps_fix field\n";
        return false;
    }
    if (!parse_int(fields[6], frame.satellites)) {
        std::cerr << "Failed to read satellites field\n";
        return false;
    }

    return true;
}

double compute_frame_rate_hz(const Frame frames[], int frame_count) {
    const long elapsed_ms = frame_count > 0 ? frames[frame_count - 1].timestamp_ms - frames[0].timestamp_ms : 0;

    return elapsed_ms == 0 ? std::numeric_limits<double>::quiet_NaN() : static_cast<double>((frame_count - 1) * 1000 / elapsed_ms);
}

bool read_frames(const char* path, Frame frames[], int max_frames, int& frame_count) {
    std::ifstream input{path};
    if (!input) {
        std::cerr << "error: failed to open input file: " << path << '\n';
        return 0;
    }

    frame_count = 0;
    char line[MAX_LINE_LENGTH];

    while (input.getline(line, MAX_LINE_LENGTH)) {
        if (line[0] == '\0') {
            continue;
        }

        if (frame_count < max_frames) {
            bool parsed = parse_frame(line, frames[frame_count++]);
            if (!parsed) {
                std::cerr << "Error appeared on parsing line " << frame_count << " (1-based index)" << std::endl;
                return false;
            }
        }
    }

    return frame_count > 0;
}

bool validate_frames(const Frame frames[], int frame_count) {
    bool validated = true;

    for (int i = 0; i < frame_count; ++i) {
        if (i > 0) {
            if (frames[i].timestamp_ms <= frames[i - 1].timestamp_ms) {
                std::cerr << "Timestamp at frame " << i << " is not bigger than timestamp at frame " << i - 1 << ": "
                          << frames[i].timestamp_ms << " and " << frames[i - 1].timestamp_ms << '\n';
                validated = false;
            }
            if (frames[i].seq != frames[i - 1].seq + 1) {
                std::cerr << "Seq values at frames " << i - 1 << " and " << i << " are not consecutive: "
                          << frames[i - 1].seq << " and " << frames[i].seq << '\n';
                validated = false;
            }
        }
        if (frames[i].voltage_v <= 0) {
            std::cerr << "Voltage at frame " << i << " is not valud: " << frames[i].voltage_v << '\n';
            validated = false;
        }
        if (frames[i].temperature_c < -40 || frames[i].temperature_c > 120) {
            std::cerr << "Temperature at frame " << i << " is not valid: " << frames[i].temperature_c << '\n';
            validated = false;
        }
        if (frames[i].gps_fix != 0 && frames[i].gps_fix != 1) {
            std::cerr << "GPS fix at frame " << i << " should be 0 or 1 but it is: " << frames[i].gps_fix << '\n';
            validated = false;
        }
        if (frames[i].satellites < 0) {
            std::cerr << "Satellites value at frame " << i << " should be >= 0 but it is: " << frames[i].satellites << '\n';
            validated = false;
        }
    }

    return validated;
}

Summary summarize(const Frame frames[], int frame_count) {
    Summary summary{};
    summary.frames_total = frame_count;
    summary.frames_valid = frame_count;
    summary.low_voltage_frames = 0;
    if (frame_count > 0) {
        summary.voltage_min = frames[0].voltage_v;
        summary.voltage_max = frames[0].voltage_v;
    }

    double temperature_sum = 0.0;

    for (int i = 0; i < frame_count; ++i) {
        if (frames[i].voltage_v < summary.voltage_min) {
            summary.voltage_min = frames[i].voltage_v;
        }

        if (frames[i].voltage_v > summary.voltage_max) {
            summary.voltage_max = frames[i].voltage_v;
        }

        temperature_sum += frames[i].temperature_c;

        if (frames[i].voltage_v < 22.0) {
            ++summary.low_voltage_frames;
        }
    }

    if (frame_count > 0) {
        const int temperature_tenths = static_cast<int>(temperature_sum * 10.0) / frame_count;
        summary.temperature_avg = static_cast<double>(temperature_tenths) / 10.0;
    }
    summary.frame_rate_hz = compute_frame_rate_hz(frames, frame_count);

    return summary;
}

void print_summary(const Summary& summary) {
    if (summary.frames_total <= 0) {
        std::cerr << "No frames - cannot generate summary\n";
        return;
    }

    if (std::isnan(summary.frame_rate_hz)) {
        std::cerr << "No time difference in frames - cannot calculate frame rate Hz\n";
        return;
    }

    std::cout << "frames_total " << summary.frames_total << '\n';
    std::cout << "frames_valid " << summary.frames_valid << '\n';
    std::cout << "voltage_min " << summary.voltage_min << '\n';
    std::cout << "voltage_max " << summary.voltage_max << '\n';
    std::cout << "temperature_avg " << summary.temperature_avg << '\n';
    std::cout << "low_voltage_frames " << summary.low_voltage_frames << '\n';
    std::cout << "frame_rate_hz " << summary.frame_rate_hz << '\n';
}
