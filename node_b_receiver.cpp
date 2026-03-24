#include <iostream>
#include <chrono>
#include <boost/asio.hpp>

using boost::asio::ip::udp;

int main() {
    try {
        boost::asio::io_context io_context;
        // Bind to port 5001
        udp::socket socket(io_context, udp::endpoint(udp::v4(), 5001));

        std::cout << "[*] Node B (Receiver) listening on Port 5001...\n";

        while (true) {
            uint64_t sent_timestamp = 0;
            udp::endpoint sender_endpoint;

            // Block and wait for incoming UDP packet
            size_t length = socket.receive_from(
                boost::asio::buffer(&sent_timestamp, sizeof(sent_timestamp)), sender_endpoint);

            if (length == sizeof(sent_timestamp)) {
                // Instantly grab the arrival time
                auto now = std::chrono::high_resolution_clock::now();
                auto duration = now.time_since_epoch();
                uint64_t current_timestamp = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

                // Calculate actual delay (Current Time - Sent Time)
                double delay_ms = (current_timestamp - sent_timestamp) / 1000.0;

                std::cout << "Packet from Node A | Actual Delay: " << delay_ms << " ms\n";
            }
        }
    } catch (std::exception& e) {
        std::cerr << "Network Error: " << e.what() << "\n";
    }
    return 0;
}