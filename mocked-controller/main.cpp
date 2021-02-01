#include <chrono>
#include <cstdint>
#include <type_traits>
#include <utility>

#include "../shared-lib/rpc_utilities.hpp"
#include "../shared-lib/utilities.hpp"
#include "mbed.h"

/**
 * Mocked version of the analysis, it returns cos of (current time * current step)
 *
 * @param socket Socket connected to PC
 * @param start First step
 * @param end Last step
 * @param tick_rate Stride between steps
 * @return Results
 */
std::vector<std::pair<float, float>> analyse(BufferedSerial& socket, float start, float end, float stride) {
    // Run analysis
    std::vector<std::pair<float, float>> results;
    auto step = start;
    do {
        // Calculate and store result
        auto now = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        results.push_back(std::make_pair(step, std::cosf(now * step) + 1.f));

        // Move to the next step
        step += stride;

        ThisThread::sleep_for(50ms);
    } while (step < end);

    return results;
}

/**
 * Mocked version of the calibration
 *
 * @param wavelength Reference wavelength
 */
void calibrate(float wavelength) {
    ThisThread::sleep_for(2s);
    chroma::_current_step = 0;
}

// main() runs in its own thread in the OS
int main() {
    std::atomic<chroma::ControllerState> state(chroma::ControllerState::Booting);

    // Run listening function
    Thread listening_thread;
    listening_thread.start([&]() { chroma::listen(state, analyse, calibrate); });

    // Boot device and stay in idle state
    chroma::to_idle(state);
}
