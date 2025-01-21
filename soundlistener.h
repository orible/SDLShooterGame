#pragma once
#include "engine.h"
#include "soundlistener.h"
#include <iostream>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <avrt.h>

class SoundListener: public Surface
{
	IMMDeviceEnumerator* pEnumerator = NULL;
	IMMDevice* pDevice = NULL;
	IAudioClient* pAudioClient = NULL;
	IAudioCaptureClient* pCaptureClient = NULL;
	WAVEFORMATEX* pWaveFormat = NULL;
	std::vector<float>* audioBuffer = new std::vector<float>();
	void ProcessAudioData(const BYTE* pData, UINT32 packetLength, WAVEFORMATEX* waveFormat);
	void Render(SDL_Renderer* g);
public:
	void Dispose();
	int Setup();
	void Step(double dt, Node* parent);
	int CaptureAudio();
	~SoundListener();
};

