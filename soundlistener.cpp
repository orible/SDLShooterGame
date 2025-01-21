#include "soundlistener.h"
#include <iostream>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <avrt.h>
#include "system.h"

#pragma comment(lib, "avrt.lib")

void SoundListener::Dispose()
{
    // Cleanup
    pAudioClient->Stop();
    pCaptureClient->Release();
    CoTaskMemFree(pWaveFormat);
    pAudioClient->Release();
    pDevice->Release();
    pEnumerator->Release();
    CoUninitialize();

    Surface::Dispose();
}

int SoundListener::Setup()
{

    CoInitialize(NULL);

    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, IID_PPV_ARGS(&pEnumerator));
    if (FAILED(hr)) {
        printf("Failed to create IMMDeviceEnumerator\n");
        return -1;
    }

    // Get the default audio endpoint (speakers)
    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    if (FAILED(hr)) {
        printf("Failed to get default audio endpoint.");
        pEnumerator->Release();
        return -1;
    }

    // Activate the audio client
    hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
    if (FAILED(hr)) {
        printf("Failed to activate IAudioClient.");
        pDevice->Release();
        pEnumerator->Release();
        return -1;
    }

    pWaveFormat = NULL;
    hr = pAudioClient->GetMixFormat(&pWaveFormat);
    if (FAILED(hr)) {
        printf("Failed to get mix format.");
        pAudioClient->Release();
        pDevice->Release();
        pEnumerator->Release();
        return -1;
    }

    // Initialize the audio client in loopback mode
    hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, 0, 0, pWaveFormat, NULL);
    if (FAILED(hr)) {
        printf("Failed to initialize IAudioClient in loopback mode.");
        CoTaskMemFree(pWaveFormat);
        pAudioClient->Release();
        pDevice->Release();
        pEnumerator->Release();
        return -1;
    }

    // Get the capture client
    hr = pAudioClient->GetService(IID_PPV_ARGS(&pCaptureClient));
    if (FAILED(hr)) {
        printf("Failed to get IAudioCaptureClient.");
        CoTaskMemFree(pWaveFormat);
        pAudioClient->Release();
        pDevice->Release();
        pEnumerator->Release();
        return -1;
    }

    // Start capturing
    hr = pAudioClient->Start();
    if (FAILED(hr)) {
        printf("Failed to start IAudioClient.");
        pCaptureClient->Release();
        CoTaskMemFree(pWaveFormat);
        pAudioClient->Release();
        pDevice->Release();
        pEnumerator->Release();
        return -1;
    }

    return 0;
}
void SoundListener::Step(double dt, Node* parent) {
    BYTE* pData;
    UINT32 packetLength = 0;
    DWORD flags;
    
    for (int i = 0; i < 100; i++) {
        HRESULT hr = pCaptureClient->GetNextPacketSize(&packetLength);
        if (FAILED(hr)) {
            std::cerr << "Failed to get packet size." << std::endl;
            return;
        }

        if (packetLength > 0) {
            hr = pCaptureClient->GetBuffer(&pData, &packetLength, &flags, NULL, NULL);
            if (SUCCEEDED(hr)) {
                // Process the audio data (pData, packetLength)
                //std::cout << "Captured " << packetLength << " frames." << std::endl;
                ProcessAudioData(pData, packetLength, pWaveFormat);
                pCaptureClient->ReleaseBuffer(packetLength);
            }
        }
    }
}
void SoundListener::ProcessAudioData(const BYTE* pData, UINT32 packetLength, WAVEFORMATEX* waveFormat) {
    // Convert raw audio samples to floating-point format
    int sampleCount = packetLength / (waveFormat->wBitsPerSample / 8);
    const float scale = 1.0f / (1 << (waveFormat->wBitsPerSample - 1)); // Normalize
    
    switch (waveFormat->wFormatTag) {
    case WAVE_FORMAT_EXTENSIBLE: {
        // Cast to WAVEFORMATEXTENSIBLE
        auto extensible = reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(waveFormat);

        // Check the SubFormat
        if (extensible->SubFormat == KSDATAFORMAT_SUBTYPE_PCM) {
            // PCM (signed integer)
            if (waveFormat->wBitsPerSample == 16) {
                const short* samples = reinterpret_cast<const short*>(pData);
                for (int i = 0; i < sampleCount; ++i) {
                    float normalizedSample = samples[i] / 32768.0f;
                    audioBuffer->push_back(normalizedSample);
                }
            }
            else if (waveFormat->wBitsPerSample == 32) {
                const int* samples = reinterpret_cast<const int*>(pData);
                for (int i = 0; i < sampleCount; ++i) {
                    float normalizedSample = samples[i] / 2147483648.0f;
                    audioBuffer->push_back(normalizedSample);
                }
            }
            else {
                std::cerr << "Unsupported PCM bit depth: " << waveFormat->wBitsPerSample << " bits" << std::endl;
            }
        }
        else if (extensible->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
            // IEEE Float
            const float* samples = reinterpret_cast<const float*>(pData);
            for (int i = 0; i < sampleCount; ++i) {
                audioBuffer->push_back(samples[i]); // Already normalized
            }
        }
        else {
            std::cerr << "Unsupported SubFormat GUID" << std::endl;
        }
    }break;
    case WAVE_FORMAT_PCM: 
        if (waveFormat->wBitsPerSample == 16) {
            const short* samples = reinterpret_cast<const short*>(pData);
            for (int i = 0; i < sampleCount; ++i) {
                float normalizedSample = samples[i] / 32768.0f; // Normalize 16-bit signed PCM
                audioBuffer->push_back(normalizedSample);
            }
        }
        else if (waveFormat->wBitsPerSample == 32) {
            const int* samples = reinterpret_cast<const int*>(pData); // 32-bit signed PCM
            for (int i = 0; i < sampleCount; ++i) {
                float normalizedSample = samples[i] / 2147483648.0f; // Normalize 32-bit signed PCM
                audioBuffer->push_back(normalizedSample);
            }
        }
        else if (waveFormat->wBitsPerSample == 8) {
            const unsigned char* samples = reinterpret_cast<const unsigned char*>(pData); // 8-bit unsigned PCM
            for (int i = 0; i < sampleCount; ++i) {
                float normalizedSample = (samples[i] - 128) / 128.0f; // Normalize 8-bit unsigned PCM
                audioBuffer->push_back(normalizedSample);
            }
        }
        else {
            std::cerr << "Unsupported PCM bit depth: " << waveFormat->wBitsPerSample << " bits" << std::endl;
        }
    case WAVE_FORMAT_IEEE_FLOAT:
        if (waveFormat->wBitsPerSample == 32) {
            const float* samples = reinterpret_cast<const float*>(pData);
            for (int i = 0; i < sampleCount; ++i) {
                audioBuffer->push_back(samples[i]); // Already normalized
            }
        }
        break;
    }

    // Limit buffer size for visualization
    if (audioBuffer->size() > 2048) { // Keep the last 2048 samples
        audioBuffer->erase(audioBuffer->begin(), audioBuffer->end() - 2048);
    }
}

void SoundListener::Render(SDL_Renderer* g) {
    System* sys = (System*)this->GetRoot();
    int SCREEN_WIDTH = sys->ScreenWidth;
    int SCREEN_HEIGHT = sys->ScreenHeight;
    int mag = 0;

    int centerY = SCREEN_HEIGHT / 2;
    int sampleCount = audioBuffer->size();

    SDL_SetRenderDrawColor(g, 0, 255, 0, 255);
    for (int i = 0; i < sampleCount - 1; ++i) {
        float mag = 1;//0000;
        float p1 = (*audioBuffer)[i] * mag;
        float p2 = (*audioBuffer)[i + 1] * mag;
        // Map samples to screen space
        int x1 = (i * SCREEN_WIDTH) / sampleCount;
        int x2 = ((i + 1) * SCREEN_WIDTH) / sampleCount;
        //printf("%f\n", p1);

        // scale the height by -1 / +1 * screen height /2
        int y1 = centerY - (int)(p1 * (SCREEN_HEIGHT / 2));
        int y2 = centerY - (int)(p2 * (SCREEN_HEIGHT / 2));

        // Draw a line between samples
        SDL_RenderDrawLine(g, x1, y1, x2, y2);
    }
}
                
int SoundListener::CaptureAudio()
{
    return 0;
}

SoundListener::~SoundListener()
{
    Surface::Dispose();
}
