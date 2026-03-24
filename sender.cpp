#include <iostream>
#include <winsock2.h>
#include <chrono>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET sendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sockaddr_in recvAddr;
    recvAddr.sin_family = AF_INET;
    recvAddr.sin_port = htons(5001);
    recvAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    std::cout << "[*] Node A (Sender) transmitting to Port 5001...\n";

    while (true) {
        auto now = std::chrono::high_resolution_clock::now();
        uint64_t timestamp = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();

        sendto(sendSocket, (char*)&timestamp, sizeof(timestamp), 0, (SOCKADDR*)&recvAddr, sizeof(recvAddr));
        std::this_thread::sleep_for(std::chrono::milliseconds(20)); // Simulate audio frame rate
    }
    closesocket(sendSocket);
    WSACleanup();
    return 0;
}
