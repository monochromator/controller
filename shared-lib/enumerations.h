#pragma once

#include <cstdint>

namespace chroma {
/**
 * Controller possible states
 */
enum class ControllerState : uint32_t {
  Booting,
  PacketReceived,
  Analyse,
  Calibrate,

  Idle
};

/**
 * Header sent before common packet body
 */
enum class PacketHeader : uint32_t { Ping = 0, Analysis = 1, Calibrate = 2 };

/**
 * Header sent before analysis-specific packet body
 */
enum class AnalysisPacketHeader : uint32_t {
  InvalidArguments = 0,
  Start = 1,
  ResultsSize = 2,
  NotCalibrated = 3
};

enum class CalibratePacketHeader : uint32_t {
  InvalidArguments = 0,
  Start = 1,
  End = 2
};

} // namespace chroma