#include <chrono>
#include <type_traits>

#include "mbed.h"
#include "../shared-lib/rpc_utilities.hpp"
#include "../shared-lib/utilities.hpp"

/**
 * Mocked version of the analysis, it returns cos of (current time * current step)
 *
 * @param socket Socket connected to PC
 * @param start First step
 * @param end Last step
 * @param tick_rate Stride between steps
 * @return Results
*/
std::vector<std::pair<uint32_t, float>> analyse(BufferedSerial& socket, uint32_t start, uint32_t end, uint32_t stride) {
    // Send invalid arguments
    if (end > start) {
        chroma::rpc_send(socket, chroma::AnalysisPacketHeader::InvalidArguments);
    }

    // Notify analysis start
    chroma::rpc_send(socket, chroma::AnalysisPacketHeader::Start);

    // Run analysis
    std::vector<std::pair<uint32_t, float>> results;
    auto step = start;
    do {
        // Calculate and store result
        auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        results.push_back(std::make_pair(step, std::cosf(now * step)));

        // Move to the next step
        step += stride;
    } while (step < end);

    return results;
}

// main() runs in its own thread in the OS
int main() {
    std::atomic<chroma::ControllerState> state(chroma::ControllerState::Booting);

    // Run listening function
    Thread listening_thread;
    listening_thread.start([&](){
        chroma::listen(state, analyse);
    });

    // Boot device and stay in idle state
    chroma::to_idle(state);
}
