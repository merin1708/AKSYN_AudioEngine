#include <iostream>
#include <winsock2.h>
#include "portaudio.h"
#include <vector>
#include <queue>
#include <mutex>
#include <chrono>
#include <fstream>

#pragma comment(lib, "ws2_32.lib")

#define SAMPLE_RATE (44100)
#define FRAMES_PER_BUFFER (512)
#define BUFFER_THRESHOLD 5

struct AudioPacket {
    long long timestamp;
    float frames[512];
};

struct ReceiverData {
    SOCKET socket;
    std::queue<std::vector<float>> audioQueue;
    std::mutex queueMutex;
    bool starting;
};

static int playbackCallback(const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData) {
    ReceiverData *data = (ReceiverData*)userData;
    float *out = (float*)outputBuffer;

    std::lock_guard<std::mutex> lock(data->queueMutex);

    if (data->audioQueue.size() < BUFFER_THRESHOLD && data->starting) {
        for(unsigned int i=0; i<framesPerBuffer; i++) out[i] = 0;
        return paContinue;
    }

    data->starting = false;

    if (!data->audioQueue.empty()) {
        std::vector<float> topPacket = data->audioQueue.front();
        for(unsigned int i=0; i<framesPerBuffer; i++) out[i] = topPacket[i];
        data->audioQueue.pop();
    } else {
        for(unsigned int i=0; i<framesPerBuffer; i++) out[i] = 0;
    }

    return paContinue;
}

int main() {
    WSADATA wsa; WSAStartup(MAKEWORD(2,2), &wsa);
    ReceiverData rd;
    rd.socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    rd.starting = true;

    sockaddr_in recvAddr;
    recvAddr.sin_family = AF_INET;
    recvAddr.sin_port = htons(5001);
    recvAddr.sin_addr.s_addr = INADDR_ANY;
    bind(rd.socket, (SOCKADDR*)&recvAddr, sizeof(recvAddr));

    Pa_Initialize();
    PaStream *stream;
    Pa_OpenDefaultStream(&stream, 0, 1, paFloat32, SAMPLE_RATE,
                         FRAMES_PER_BUFFER, playbackCallback, &rd);
    Pa_StartStream(stream);

    // Open CSV log file
    std::ofstream logFile("latency_log.csv");
    logFile << "packet,latency_ms\n";
    int packetCount = 0;

    std::cout << "[*] Jitter Buffer Active. Listening...\n";
    std::cout << "[*] Logging latency to latency_log.csv\n";

    while (true) {
        AudioPacket pkg;
        int bytesReceived = recv(rd.socket, (char*)&pkg, sizeof(AudioPacket), 0);

        if (bytesReceived > 0) {
            auto now = std::chrono::high_resolution_clock::now();
            long long arrival_time = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();

            double delay_ms = (arrival_time - pkg.timestamp) / 1000.0;

            std::cout << "\r[LATENCY] " << delay_ms << " ms    " << std::flush;

            // Log to CSV
            packetCount++;
            logFile << packetCount << "," << delay_ms << "\n";
            logFile.flush();

            std::vector<float> audioData(pkg.frames, pkg.frames + 512);
            std::lock_guard<std::mutex> lock(rd.queueMutex);
            rd.audioQueue.push(audioData);

            if(rd.audioQueue.size() > 20) rd.audioQueue.pop();
        }
    }

    logFile.close();
    return 0;
}