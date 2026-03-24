#include <iostream>
#include <winsock2.h>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET recvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    sockaddr_in recvAddr;
    recvAddr.sin_family = AF_INET;
    recvAddr.sin_port = htons(5001);
    recvAddr.sin_addr.s_addr = INADDR_ANY;

    bind(recvSocket, (SOCKADDR*)&recvAddr, sizeof(recvAddr));
    std::cout << "[*] Node B (Receiver) listening on Port 5001...\n";

    while (true) {
        uint64_t sent_timestamp = 0;
        sockaddr_in senderAddr;
        int senderAddrSize = sizeof(senderAddr);

        int bytesReceived = recvfrom(recvSocket, (char*)&sent_timestamp, sizeof(sent_timestamp), 0, (SOCKADDR*)&senderAddr, &senderAddrSize);

        if (bytesReceived > 0) {
            auto now = std::chrono::high_resolution_clock::now();
            uint64_t current_timestamp = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();

            double delay_ms = (current_timestamp - sent_timestamp) / 1000.0;
            std::cout << "Packet received | Network Delay: " << delay_ms << " ms\n";
        }
    }
    closesocket(recvSocket);
    WSACleanup();
    return 0;
}