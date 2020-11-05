#pragma once

#include <cstddef>
#include <functional>

#define _GRATING_REVOLUTION_STEP_COUNT 42 // TODO: TBD
#define _MOTOR_WAVELENGTH_PER_STEP 1      // TODO: TBD

#define _REFERENCE_STEP_INDEX 0
#define _REFERENCE_WAVELENGTH_INDEX 1

namespace chroma {

// Current step of the motor
static uint32_t _current_step = std::numeric_limits<uint32_t>::max();

// Reference step (step and wavelength)
static std::pair<uint32_t, uint32_t> _reference_step;

/**
 * Move grating motor by one step, if positive_rotation is true then the current
 * step is incremented and wavelength configured by grating must be greater than
 * the previous one
 *
 * @param positive_way Rotation way
 */
void rotate_grating(bool positive_way) {
  // TODO: Implement it

  // Update current step
  _current_step = (_current_step + (positive_way ? 1 : -1)) %
                  _GRATING_REVOLUTION_STEP_COUNT;
}

/**
 * Get current wavelength configured by grating
 *
 * @return Wavelength
 */
uint32_t wavelength() {
  return std::get<_REFERENCE_WAVELENGTH_INDEX>(_reference_step) -
         (std::get<_REFERENCE_STEP_INDEX>(_reference_step) - _current_step) *
             _MOTOR_WAVELENGTH_PER_STEP;
}

/**
 * Calibrate grating motor with a known wavelength
 *
 * @param wavelength Wavelength of the source installed
 */
void calibrate(uint32_t wavelength) {
  // Consider current motor state as the step 0
  _current_step = 0;

  // Find step where the photodiode reacts
  auto index = -1;
  for (auto i = 0; i < _GRATING_REVOLUTION_STEP_COUNT; i++) {
    // TODO: Break if photodiode reacts
    // auto result = PHOTODIODE_VALUE;
    // if (result > PHOTODIODE_REACTION_VALUE) {
    //   index = i;
    //   break;
    // }

    // Move motor to the next step
    rotate_grating(true);
  }

  // TODO: Crash if index = -1

  // Update reference step
  _reference_step = std::make_pair(index, wavelength);
}

/**
 * Test whether grating motor has been calibrated at least one time
 *
 * @return true if grating motor has been calibrated at least one time, false
 * otherwise
 */
bool is_calibrate() {
  return _current_step != std::numeric_limits<uint32_t>::max();
}
} // namespace chroma