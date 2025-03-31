#pragma once
#include "engine.h"
#include "soundlistener.h"
#include <iostream>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <avrt.h>
#include <aubio.h>
#include <map>
class SoundListener: public Renderable
{
	IMMDeviceEnumerator* pEnumerator = NULL;
	IMMDevice* pDevice = NULL;
	IAudioClient* pAudioClient = NULL;
	IAudioCaptureClient* pCaptureClient = NULL;
	WAVEFORMATEX* pWaveFormat = NULL;
	std::vector<float>* audioBuffer = new std::vector<float>();
	void ProcessAudioDataElements();
	void ProcessAudioData(const BYTE* pData, UINT32 packetLength, WAVEFORMATEX* waveFormat);
	void Render(SDL_Renderer* g);

	uint_t win_size = 512;      // Window size for FFT
	uint_t hop_size = win_size/2;       // Hop size for overlap
	uint_t samplerate = 44100;   // Sampling rate (Hz)
	aubio_tempo_t* tempo;
	aubio_notes_t* notes;

	int audioBufferReadPosition = 0;
	fvec_t* tempo_input;
	fvec_t* tempo_output;

	fvec_t* notes_input;
	fvec_t* notes_output;

	struct Midi {
		long start;
		long end;
		int note;
		float velocity;
	};

	std::map<int, Midi> midiMap;
	std::vector<Midi> midiBuffer;
	std::vector<float> beatBuffer;
	float lastBeat;

public:
	void Dispose();
	int Setup();
	void Step(double dt, Node* parent);
	int CaptureAudio();
	~SoundListener();
};

