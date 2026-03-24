#include <iostream>
#include <winsock2.h>
#include "portaudio.h"
#include <chrono>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")

#define SAMPLE_RATE (44100)
#define FRAMES_PER_BUFFER (512)

struct AudioPacket {
    long long timestamp;
    float frames[512]; // Matches FRAMES_PER_BUFFER
};

struct SenderData {
    SOCKET socket;
    sockaddr_in addr;
};

// This function is called by PortAudio every time the Mic has data
static int recordCallback(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData) {
    SenderData *data = (SenderData*)userData;
    AudioPacket pkg;

    // 1. Capture current time
    auto now = std::chrono::high_resolution_clock::now();
    pkg.timestamp = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();

    // 2. Copy audio data
    memcpy(pkg.frames, inputBuffer, framesPerBuffer * sizeof(float));

    // 3. Send the whole struct
    sendto(data->socket, (const char*)&pkg, sizeof(AudioPacket), 0, 
           (SOCKADDR*)&data->addr, sizeof(data->addr));
           
    return paContinue;
}

int main() {
    // 1. Setup Network
    WSADATA wsa; WSAStartup(MAKEWORD(2,2), &wsa);
    SenderData sd;
    sd.socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sd.addr.sin_family = AF_INET;
    sd.addr.sin_port = htons(5001);
    sd.addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // 2. Setup PortAudio — try all WO Mic variants (WASAPI=12, DirectSound=7, MME=2)
    Pa_Initialize();
    PaStream *stream = NULL;

    int woDev[] = {12, 7, 2}; // WASAPI, DirectSound, MME
    PaError err = paInvalidDevice;
    int usedDev = -1;

    for (int d : woDev) {
        PaStreamParameters inputParams;
        inputParams.device = d;
        inputParams.channelCount = 1;
        inputParams.sampleFormat = paFloat32;
        inputParams.suggestedLatency = Pa_GetDeviceInfo(d)->defaultLowInputLatency;
        inputParams.hostApiSpecificStreamInfo = NULL;

        err = Pa_OpenStream(&stream, &inputParams, NULL, SAMPLE_RATE,
                            FRAMES_PER_BUFFER, paClipOff, recordCallback, &sd);
        if (err == paNoError) { usedDev = d; break; }
        std::cout << "  [skip] ID " << d << " ("
                  << Pa_GetDeviceInfo(d)->name << "): "
                  << Pa_GetErrorText(err) << "\n";
    }

    if (err != paNoError) {
        std::cout << "[ERROR] Could not open any WO Mic device. Is WO Mic connected?\n";
        Pa_Terminate();
        return 1;
    }

    std::cout << "[*] Sender Active. Capturing from: "
              << Pa_GetDeviceInfo(usedDev)->name << " (ID " << usedDev << ")\n";
    Pa_StartStream(stream);

    std::cout << "Press Enter to stop...\n";
    std::cin.get();

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    return 0;
}