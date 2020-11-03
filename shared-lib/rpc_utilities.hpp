#pragma once

namespace chroma {
    /**
    * Receive a variable through socket
    * 
    * @param socket Socket connected to PC
    * @param S Socket type
    * @param D Data type
    * @return Data received
    */
    template<typename S, typename D>
    D rpc_receive(S& socket) {
        size_t received = 0;
        D data;

        // Loop until we received all data
        do {
            received = socket.read(&data, sizeof(D));
        } while(received != sizeof(D) && received != -1);

        return data;
    }

    /**
    * Receive an array through socket
    *
    * @param socket Socket
    * @param S Socket type
    * @param D Data type
    * @param L Array length
    * @return Array received
    */
    template<typename S, typename D, uint32_t L>
    std::array<D, L> rpc_receive(S& socket) {
        std::array<D, L> data;
        size_t received = 0;

        do {
            auto chunk_received = socket.read(reinterpret_cast<uint8_t*>(data.data()) + received, sizeof(D) * L - received);
            received += chunk_received;

            // Exit on error
            if (chunk_received == -1) {
                break;
            }
        } while (received != sizeof(D) * L && received != -1);

        return data;
    }

    /**
    * Send a variable through socket
    * 
    * @param socket Socket connected to PC
    * @param data Data to send
    * @param S Socket type
    * @param D Data type
    */
    template<typename S, typename D>
    void rpc_send(S& socket, D const& data) {
        size_t sent = 0;

        // Loop until we send all data
        do {
            sent = socket.write(&data, sizeof(D));
        } while(sent != sizeof(D) && sent != -1);
    }
}