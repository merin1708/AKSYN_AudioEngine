#include <iostream>
#include <winsock2.h>
#include "portaudio.h"
#include <vector>
#include <queue>
#include <mutex>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")

#define SAMPLE_RATE (44100)
#define FRAMES_PER_BUFFER (512)
#define BUFFER_THRESHOLD 5 // How many packets to "wait" for before playing

struct AudioPacket {
    long long timestamp;
    float frames[512]; // Matches FRAMES_PER_BUFFER
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

    // If we don't have enough packets yet, play silence to build the buffer
    if (data->audioQueue.size() < BUFFER_THRESHOLD && data->starting) {
        for(unsigned int i=0; i<framesPerBuffer; i++) out[i] = 0;
        return paContinue;
    }

    data->starting = false; // Buffer is primed, start playing

    if (!data->audioQueue.empty()) {
        std::vector<float> topPacket = data->audioQueue.front();
        for(unsigned int i=0; i<framesPerBuffer; i++) {
            out[i] = topPacket[i];
        }
        data->audioQueue.pop();
    } else {
        // Underflow: fill with silence if network is too slow
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

    std::cout << "[*] Jitter Buffer Active. Listening...\n";

    // Main loop: Constantly receive from network and push to queue
    while (true) {
        AudioPacket pkg;
        int bytesReceived = recv(rd.socket, (char*)&pkg, sizeof(AudioPacket), 0);
        
        if (bytesReceived > 0) {
            // 1. Get current arrival time
            auto now = std::chrono::high_resolution_clock::now();
            long long arrival_time = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();

            // 2. Calculate Latency (Arrival - Sent)
            double delay_ms = (arrival_time - pkg.timestamp) / 1000.0;

            // 3. Print it (Clear the line so it doesn't flood the screen)
            std::cout << "\r[LATENCY] " << delay_ms << " ms    " << std::flush;

            // 4. Push only the audio frames to the Jitter Buffer
            std::vector<float> audioData(pkg.frames, pkg.frames + 512);
            std::lock_guard<std::mutex> lock(rd.queueMutex);
            rd.audioQueue.push(audioData);
            
            // Prevent memory explosion if sender is too fast
            if(rd.audioQueue.size() > 20) rd.audioQueue.pop(); 
        }
    }

    return 0;
}