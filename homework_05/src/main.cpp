#include "telemetry.hpp"

#include <iostream>

int main(int argc, char** argv) {
    // The executable expects exactly one telemetry log path.
    if (argc != 2) {
        std::cerr << "usage: telemetry_check <input_path>\n";
        return 1;
    }

    Frame frames[MAX_TELEMETRY_FRAMES];
    int frame_count;
    if (!read_frames(argv[1], frames, MAX_TELEMETRY_FRAMES, frame_count))
        return 1;

    if (!validate_frames(frames, frame_count))
        return 1;

    const Summary summary = summarize(frames, frame_count);
    print_summary(summary);

    return 0;
}
