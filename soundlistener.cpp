
#include "soundlistener.h"
#include <iostream>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <avrt.h>
#include "system.h"

#include <aubio.h>
#include "hud.h"

//#pragma comment(lib, "User32.lib")
#pragma comment(lib, "avrt.lib")
#define ONE_SECOND 1000
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


    // Create a tempo detection object
    tempo = new_aubio_tempo("default", win_size, hop_size, samplerate);
    //new_aubio_pitch("default", win_size, hop_size, samplerate);
    notes = new_aubio_notes("default", win_size, hop_size, samplerate);
    //aubio_notes_set_minioi_ms(notes, 1000);
    //aubio_notes_set_release_drop(notes, 1000);
    //aubio_notes_set_silence(notes, 1000);

    // Input and output buffers
    //tempo_input = new_fvec(hop_size);  // Input buffer (single channel)
    tempo_output = new_fvec(2);        // Output buffer (tempo results)


    //notes_input = new_fvec(hop_size);  // Input buffer (single channel)
    notes_output  = new_fvec(3);        // Output buffer (tempo results)

    return 0;
}
void SoundListener::Step(double dt, Node* parent) {
    BYTE* pData;
    UINT32 packetLength = 0;
    DWORD flags;
    
    //audioBuffer->clear();

    for (int i = 0; i < 2048; i++) {
        HRESULT hr = pCaptureClient->GetNextPacketSize(&packetLength);
        if (FAILED(hr)) {
            //std::cerr << "Failed to get packet size." << std::endl;
            break;
        }

        if (packetLength > 0) {
            hr = pCaptureClient->GetBuffer(&pData, &packetLength, &flags, NULL, NULL);
            if (SUCCEEDED(hr)) {

                // Process the audio data into audioBuffer (pData, packetLength)
                ProcessAudioData(pData, packetLength, pWaveFormat);
                
                // release memory
                pCaptureClient->ReleaseBuffer(packetLength);
                continue;
            }
            break;
        }
    }

    // process all our new packets
    this->ProcessAudioDataElements();
}
// Helper function to map frequency to MIDI note names
const char* get_note_name(int midi_note) {
    if (midi_note < 0) return "NULL";
    static const char* note_names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    int octave = (midi_note / 12) - 1;
    int noteIndex = (midi_note % 12);
    return note_names[noteIndex];
}
void SoundListener::ProcessAudioDataElements()
{
    HUD* hud = ((HUD*)this->GetNode("/DebugHUD"));
    //int sz = 0;
    //if ((sz = audioBuffer->size() - hop_size) < 0) {
    //    sz = 0;
    //}

    int time = CurTime();
    midiBuffer.erase(std::remove_if(midiBuffer.begin(), midiBuffer.end(),
       [time](const Midi &note) {
            return (time - note.start > ONE_SECOND * 5); // Remove notes older than 5 seconds
       }),
        midiBuffer.end());

    for (size_t i = audioBufferReadPosition; i < audioBuffer->size(); i += hop_size) {
        // Copy audio samples to input buffer

        tempo_input = new_fvec(hop_size);  // Input buffer (single channel)
        notes_input = new_fvec(hop_size);  // Input buffer (single channel)

        for (uint_t j = 0; j < hop_size; ++j) {
            if (i + j > audioBuffer->size()) {
                break;
            }
            tempo_input->data[j] = (i + j < audioBuffer->size()) ? (*audioBuffer)[i + j] : 0.0f;
            notes_input->data[j] = (i + j < audioBuffer->size()) ? (*audioBuffer)[i + j] : 0.0f;
            audioBufferReadPosition++;
        }

        // Process the input buffer to detect beats
        aubio_tempo_do(tempo, tempo_input, tempo_output);

        // Check if a beat was detected
        if (tempo_output->data[0] > 0) {
            int bpm = aubio_tempo_get_bpm(tempo);
            hud->trackInfo->SetTextF("BPM: %d", bpm);
        }

        aubio_notes_do(notes, notes_input, notes_output);

        // if this is not null we have a turn off request
        if (notes_output->data[2] != 0) {
            smpl_t lastmidi = notes_output->data[2];
            if (this->midiMap.find(lastmidi) == this->midiMap.end()) {
                this->midiMap[lastmidi] = Midi{ CurTime(), 0, (int)lastmidi, 0 };
            }
            this->midiMap[lastmidi].end = CurTime();
            this->midiMap[lastmidi].velocity = notes_output->data[1];
            printf("[%s] (%d) Note ended\n", get_note_name(lastmidi), (int)lastmidi);
        }
        if(notes_output->data[0] != 0) {
            smpl_t lastmidi = notes_output->data[0];
            if (this->midiMap.find(lastmidi) == this->midiMap.end()) {
                this->midiMap[lastmidi] = Midi{ CurTime(), 0, (int)lastmidi, 0 };
            }
            this->midiMap[lastmidi].end = 0;
            this->midiMap[lastmidi].start = CurTime();
            this->midiMap[lastmidi].velocity = notes_output->data[1];
            printf("[%s] (%d) Note started\n", get_note_name(lastmidi), (int)lastmidi);
        }

        del_fvec(tempo_input);
        del_fvec(notes_input);
        //printf("Note detected: %s%d (MIDI: %d) | Velocity: %.2f | Duration: %.2fs\n",
        //    get_note_name(midi_note), midi_note / 12 - 1, midi_note, velocity, duration);
        //midiBuffer.push_back(Midi{ CurTime(), 1, midi_note, velocity, duration * ONE_SECOND });


    }
    //aubio_tempo_do(tempo, input, output);
    //if (fvec_get_sample(output, 0) > 0) {
    //
    //}
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
    /*int sz = audioBuffer->size();
    if (audioBuffer->size() > 2048) { // Keep the last 2048 samples
        audioBuffer->erase(audioBuffer->begin(), audioBuffer->end() - 2048);
        int diff = sz - audioBuffer->size();
        audioBufferReadPosition -= diff;
    }*/
}
int midi_to_y(int midiNote, int screenHeight) {
    const int minMidi = 21;  // Lowest MIDI note (A0)
    const int maxMidi = 108; // Highest MIDI note (C8)
    return screenHeight - ((midiNote - minMidi) * screenHeight) / (maxMidi - minMidi);
}
void SoundListener::Render(SDL_Renderer* g) {

    System* sys = (System*)this->GetRoot();
    int SCREEN_WIDTH = sys->ScreenWidth;
    int SCREEN_HEIGHT = sys->ScreenHeight;
    int mag = 0;
    int width = 1024;
    int centerY = SCREEN_HEIGHT / 2;
    int sampleCount = audioBuffer->size();
    int start = fmax(0, sampleCount - width);


    SDL_SetRenderDrawColor(g, 0, 255, 0, 255);
    for (int i = start; i < sampleCount - 1; ++i) {
        int r = i - start;
        float mag = 1;//0000;
        float p1 = (*audioBuffer)[i] * mag;
        float p2 = (*audioBuffer)[i + 1] * mag;

        // Map samples to screen space
        int x1 = (r * SCREEN_WIDTH) / width;
        int x2 = ((r + 1) * SCREEN_WIDTH) / width;

        // scale the height by -1 / +1 * screen height /2
        int y1 = centerY - (int)(p1 * (SCREEN_HEIGHT / 2));
        int y2 = centerY - (int)(p2 * (SCREEN_HEIGHT / 2));

        // Draw a line between samples
        SDL_RenderDrawLine(g, x1, y1, x2, y2);
    }

    std::map<int, Midi>::iterator it;
    for (it = midiMap.begin(); it != midiMap.end(); it++) {
        it->first;
        Midi midi = it->second;

        int dt = CurTime() - midi.start;
        int len = midi.start - midi.end + 10;

        int x1 = SCREEN_WIDTH - (dt / (ONE_SECOND * 5.0f)) * SCREEN_WIDTH;
        int x3 = SCREEN_WIDTH - (dt + len / (ONE_SECOND * 5.0f)) * SCREEN_WIDTH;

        int x2 = x1 + (midi.end > 0)
            ? x3
            : 1000;
        int y1 = midi_to_y(midi.note, SCREEN_HEIGHT);

        SDL_RenderDrawLine(g, x1, y1, x2, y1);
    }

    /*
    for (int i = 0; i < midiBuffer.size(); i++) {
        Midi midi = midiBuffer[i];
        
        int dt = CurTime() - midi.time;
        int x1 = SCREEN_WIDTH - (dt / (ONE_SECOND * 5.0f)) * SCREEN_WIDTH;
        int x2 = x1 + 100;// midi.duration;
        int y1 = midi_to_y(midi.note, SCREEN_HEIGHT);

        SDL_RenderDrawLine(g, x1, y1, x2, y1);
    }*/
}
                
int SoundListener::CaptureAudio()
{
    return 0;
}

SoundListener::~SoundListener()
{
    Surface::Dispose();
}
