#include <iostream>
#include <chrono>
#include <thread>
#include <boost/asio.hpp>

using boost::asio::ip::udp;

int main() {
    try {
        boost::asio::io_context io_context;
        udp::socket socket(io_context);
        socket.open(udp::v4());

        // Target: Localhost, Port 5001
        udp::endpoint receiver_endpoint(boost::asio::ip::make_address("127.0.0.1"), 5001);

        std::cout << "[*] Node A (Sender) transmitting to Port 5001...\n";

        while (true) {
            // Get precise current time
            auto now = std::chrono::high_resolution_clock::now();
            auto duration = now.time_since_epoch();
            uint64_t timestamp = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

            // Fire the UDP Datagram (Simulating an audio packet payload)
            boost::system::error_code err;
            socket.send_to(boost::asio::buffer(&timestamp, sizeof(timestamp)), receiver_endpoint, 0, err);

            // Wait 20ms to simulate the Packetization Delay (D_pack)
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    } catch (std::exception& e) {
        std::cerr << "Network Error: " << e.what() << "\n";
    }
    return 0;
}