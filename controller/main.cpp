#include "PinNames.h"
#include "mbed.h"
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <utility>

#include "../shared-lib/rpc_utilities.hpp"
#include "../shared-lib/utilities.hpp"
#include "../shared-lib/motor.hpp"
// Initialize a pins to perform analog input and digital output fucntions

/**
 * Analysis, it returns pairs 
 *
 * @param socket Socket connected to PC
 * @param start First step
 * @param end Last step
 * @param tick_rate Stride between steps
 * @return Results
 */
std::vector<std::pair<float, float>> analyse(BufferedSerial& socket, float start, float end, float stride) {
    AnalogIn   photodiodePin(p15);  

    // Run analysis
    std::vector<std::pair<float, float>> results;
    auto step = start;
    do {

        // Goto step
        chroma::motor_goto(step);

        // Getting photodiode
        // Pin p15
        auto value = photodiodePin.read(); 
        results.push_back(std::make_pair(step, value));

        // Move to the next step
        step += stride;

    } while (step < end);

    return results;
}


// main() runs in its own thread in the OS
int main() {
    std::srand(std::time(nullptr)); // use current time as seed for random generator
    std::atomic<chroma::ControllerState> state(chroma::ControllerState::Booting);

    // Run listening function
    Thread listening_thread;
    listening_thread.start([&]() { chroma::listen(state, analyse, chroma::calibrate); });

    // Boot device and stay in idle state
    chroma::to_idle(state);

}
