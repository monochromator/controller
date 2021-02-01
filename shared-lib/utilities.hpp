#pragma once

#include <array>
#include <atomic>
#include <functional>
#include <vector>

#include "constants.h"
#include "enumerations.h"
#include "mbed.h"
#include "motor.hpp"
#include "rpc_utilities.hpp"

namespace chroma {
/**
 * Pass current thread into idle state and notify the current state thanks to
 * LEDs
 *
 * @param state Controller state
 */
void to_idle(std::atomic<ControllerState> &state) {
  std::array<DigitalOut, 4> leds = {DigitalOut(LED1), DigitalOut(LED2),
                                    DigitalOut(LED3), DigitalOut(LED4)};
  const int IDLE_LED = 0;
  const int ANALYSIS_LED = 1;
  const int CALIBRATE_LED = 2;
  const int PACKET_LED = 3;

  while (true) {
    // Get current state
    ControllerState currentState(state);

    switch (currentState) {
    case ControllerState::Booting: {
      // Turn off LEDs
      for (auto &led : leds) {
        led = 0;
      }

      // Turn on LEDs one after another
      for (auto i = 0; i < leds.size() * 2; i++) {
        // Turn off previous LED
        auto previous_index = (i - 1) % leds.size();
        if (previous_index >= 0) {
          leds[previous_index] = 0;
        }

        // Turn on current LED
        leds[i % leds.size()] = 1;

        ThisThread::sleep_for(10ms);
      }

      // Turn off LEDs
      for (auto &led : leds) {
        led = 0;
      }

      // Update state
      state.store(ControllerState::Idle);
      break;
    }

    // Toggle analysis LED
    case ControllerState::Analyse:
      for (auto i = 0; i < leds.size(); i++) {
        leds[i] = i == ANALYSIS_LED ? !leds[ANALYSIS_LED] : 0;
      }
      break;

    // Toggle calibration LED
    case ControllerState::Calibrate:
      for (auto i = 0; i < leds.size(); i++) {
        leds[i] = i == CALIBRATE_LED ? !leds[CALIBRATE_LED] : 0;
      }
      break;

    // Turn on packet LED and update status
    case ControllerState::PacketReceived:
      leds[PACKET_LED] = 1;
      state.store(ControllerState::Idle);
      break;

    // Toggle idle LED
    case ControllerState::Idle:
    default:
      for (auto i = 0; i < leds.size(); i++) {
        leds[i] = i == IDLE_LED ? !leds[IDLE_LED] : 0;
      }
      break;
    }

    // Sleep thread to reduce CPU usage
    ThisThread::sleep_for(50ms);
  }
}

/**
 * Listen for incoming requests from PC
 *
 * @param controller_state Controller state
 * @param analysis_func Function that runs analysis (arguments: PC connection,
 * range start, range end, range stride)
 * @param calibration_func Function that callibrates the device
 */
void listen(std::atomic<ControllerState> &controller_state,
            std::function<std::vector<std::pair<float, float>>(
                BufferedSerial &, float, float, float)>
                analysis_func,
            std::function<void(float)> calibration_func) {
  BufferedSerial usb_conn(USBTX, USBRX);

  while (true) {
    // Read a header
    auto header = rpc_receive<BufferedSerial, PacketHeader>(usb_conn);

    // Notify packet receive
    controller_state.store(ControllerState::PacketReceived);

    // Treat header
    switch (header) {
    case PacketHeader::Ping:
      rpc_send(usb_conn, PING_RESPONSE);
      break;

    case PacketHeader::Calibrate: {
      // Receive arguments
      auto wavelength = rpc_receive<BufferedSerial, float>(usb_conn);

      // Send invalid arguments
      if (wavelength <= 0) {
        chroma::rpc_send(usb_conn,
                         chroma::CalibratePacketHeader::InvalidArguments);
        break;
      }

      // Notify calibration start
      chroma::rpc_send(usb_conn, chroma::CalibratePacketHeader::Start);
      controller_state.store(ControllerState::Calibrate);

      // Calibrate device
      calibration_func(wavelength);

      // Notify calibration end
      chroma::rpc_send(usb_conn, chroma::CalibratePacketHeader::End);
      controller_state.store(ControllerState::Idle);
      break;
    }

    case PacketHeader::Analysis: {
      // Receive arguments
      auto range = rpc_receive<BufferedSerial, float, 3>(usb_conn);

      // Send invalid arguments
      if (range[0] > range[1]) {
        chroma::rpc_send(usb_conn,
                         chroma::AnalysisPacketHeader::InvalidArguments);
        break;
      }

      // Send not calibrated
      if (!chroma::is_calibrated()) {
        chroma::rpc_send(usb_conn, chroma::AnalysisPacketHeader::NotCalibrated);
        break;
      }

      // Notify simuation start
      chroma::rpc_send(usb_conn, chroma::AnalysisPacketHeader::Start);
      controller_state.store(ControllerState::Analyse);

      // Run analysis
      auto results = analysis_func(usb_conn, range[0], range[1], range[2]);

      // Send results
      chroma::rpc_send(usb_conn, chroma::AnalysisPacketHeader::ResultsSize);
      chroma::rpc_send(usb_conn, results.size());
      for (auto &result : results) {
        chroma::rpc_send(usb_conn, std::get<0>(result));
        chroma::rpc_send(usb_conn, std::get<1>(result));
      }

      // Notify analysis end
      controller_state.store(ControllerState::Idle);
      break;
    }

    default:
      // Unknown header, skip it
      break;
    }
  }
}
} // namespace chroma