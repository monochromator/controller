#pragma once

#include <cstdint>

namespace chroma {
/**
 * Controller possible states
 */
enum ControllerState : uint32_t {
  Booting,
  PacketReceived,
  Analyse,

  Idle
};

/**
 * Header sent before common packet body
 */
enum PacketHeader : uint32_t { Ping = 0, Analysis = 1 };

/**
 * Header sent before analysis-specific packet body
 */
enum AnalysisPacketHeader : uint32_t {
  InvalidArguments = 0,
  Start = 1,
  ResultsSize = 2
  // TODO: Add not calibrated
};
} // namespace chroma