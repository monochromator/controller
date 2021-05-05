#pragma once
#include "mbed.h"

#include "PinNames.h"
#include "mbed_wait_api.h"
#include <cstddef>
#include <cstdlib>
#include <functional>

#define _GRATING_REVOLUTION_STEP_COUNT 10000
#define _MOTOR_WAVELENGTH_PER_STEP 0.3492

#define _REFERENCE_STEP_INDEX 0
#define _REFERENCE_WAVELENGTH_INDEX 1

namespace chroma {

// Current step of the motor
static uint32_t _current_step = std::numeric_limits<uint32_t>::max();

// Reference step (step and wavelength)
static std::pair<uint32_t, float> _reference_step;

/**
 * Move grating motor by one step, if positive_rotation is true then the current
 * step is incremented and wavelength configured by grating must be greater than
 * the previous one
 *
 * @param positive_way Rotation way
 */
void rotate_grating(bool positive_way) {
  // DigitalOut P1(p20), P2(p19), P3(p18), P4(p16);
  DigitalOut P1(p7), P2(p8), P3(p9), P4(p10);

  P1 = P2 = P3 = P4 = 0;
 
  int end = positive_way ? 4 : 0;
  int way = positive_way ? 1 : -1;

  for (int i = positive_way ? 0 : 4; i != end; i+= way) {
    switch (i) {
    case 0:
      P1 = 1; P2 = 0; P3 = 1; P4 = 0;
      // led1 = 1; led2 = 0; led3 = 1; led4 = 0;
      break;
    case 1:
      P1 = 0; P2 = 1; P3 = 1; P4 = 0;
      // led1 = 0; led2 = 1; led3 = 1; led4 = 0;
      break;
    case 2:
      P1 = 0; P2 = 1; P3 = 0; P4 = 1;
      // led1 = 0; led2 = 1; led3 = 0; led4 = 1;
      break;
    case 3:
      P1 = 1; P2 = 0; P3 = 0; P4 = 1;
      // led1 = 1; led2 = 0; led3 = 0; led4 = 1;
      break;
    default:
      break;
    }
    wait_us(100000);
  }

  // Update current step
  _current_step = (_current_step + (positive_way ? 1 : -1)) %
                  _GRATING_REVOLUTION_STEP_COUNT;
}

/**
 * Get current wavelength configured by grating
 *
 * @return Wavelength
 */
float wavelength() {
  return std::get<_REFERENCE_WAVELENGTH_INDEX>(_reference_step) -
         (std::get<_REFERENCE_STEP_INDEX>(_reference_step) - _current_step) *
             _MOTOR_WAVELENGTH_PER_STEP;
}

void motor_goto(float step) {

  auto curLw = wavelength();

  auto deltaToMove = step - curLw;

  auto stepNumber = std::abs(deltaToMove) / _MOTOR_WAVELENGTH_PER_STEP;

  auto way = deltaToMove > 0;

  for (int i = 0; i < stepNumber; i++) {
    rotate_grating(way);
  }
}

/**
 * Calibrate grating motor with a known wavelength
 *
 * @param wavelength Wavelength of the source installed
 */
void calibrate(float wavelength) {
  // Consider current motor state as the step 0
  _current_step = 0;

  // Loading photodiode in
  AnalogIn   photodiodePin(p15);  

  // Deteciton treshold
  const float PHOTODIODE_REACTION_VALUE = 100;

  // Find step where the photodiode reacts
  auto index = -1;
  for (auto i = 0; i < _GRATING_REVOLUTION_STEP_COUNT; i++) {
    auto PHOTODIODE_VALUE =  photodiodePin.read(); 

    // Break if photodiode reacts
    auto result = PHOTODIODE_VALUE;
    if (result > PHOTODIODE_REACTION_VALUE) {
      index = i;
      // break;
    }

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
bool is_calibrated() {
  return _current_step != std::numeric_limits<uint32_t>::max();
}
} // namespace chroma