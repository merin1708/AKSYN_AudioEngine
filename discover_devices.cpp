#include <iostream>
#include <portaudio.h>

int main() {
    Pa_Initialize();
    int numDevices = Pa_GetDeviceCount();
    std::cout << "--- Available Audio Devices (with Host API) ---\n";
    for (int i = 0; i < numDevices; i++) {
        const PaDeviceInfo* dev = Pa_GetDeviceInfo(i);
        const PaHostApiInfo* api = Pa_GetHostApiInfo(dev->hostApi);
        if (dev->maxInputChannels > 0) {
            std::cout << "ID: " << i
                      << " | API: " << api->name
                      << " | Name: " << dev->name
                      << " | Inputs: " << dev->maxInputChannels << "\n";
        }
    }
    Pa_Terminate();
    return 0;
}
