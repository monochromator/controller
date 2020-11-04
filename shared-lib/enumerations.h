#pragma once

namespace chroma {
    /**
    * Controller possible states
    */
    enum ControllerState : uint32_t {
        Booting,
        PacketReceived,
        ExecuteSimulation,

        Idle
    };

    /**
    * Header sent before common packet body
    */
    enum PacketHeader : uint32_t {
        Ping = 0,
        Simulation = 1
    };

    /**
    * Header sent before simulation-specific packet body
    */
    enum SimulationPacketHeader : uint32_t {
        InvalidArguments = 0,
        Start = 1,
        ResultsSize = 2
    };
}