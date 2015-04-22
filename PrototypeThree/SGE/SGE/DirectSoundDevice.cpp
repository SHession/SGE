#include "stdafx.h"
#include "SGDS.h"


using namespace SGDS;

DirectSoundDevice::DirectSoundDevice(){

}

DirectSoundDevice::~DirectSoundDevice(){
	CleanUp();
}

HRESULT DirectSoundDevice::InitializeDevice(HWND hWnd){

	HRESULT result = DirectSoundCreate8(NULL, &lpds,NULL);
	if(FAILED(result)) return result;

	lpds->SetCooperativeLevel(hWnd, DSSCL_PRIORITY);
	if(FAILED(result)) return result;

	return S_OK;
}

HRESULT DirectSoundDevice::LoadWav(char* filename, SGE::Sound::Sound *sound){
	//This method is based of an example featured on http://www.rastertek.com/dx11tut14.html
	struct WaveHeaderType
	{
		char chunkId[4];
		unsigned long chunkSize;
		char format[4];
		char subChunkId[4];
		unsigned long subChunkSize;
		unsigned short audioFormat;
		unsigned short numChannels;
		unsigned long sampleRate;
		unsigned long bytesPerSecond;
		unsigned short blockAlign;
		unsigned short bitsPerSample;
		char dataChunkId[4];
		unsigned long dataSize;
	};

	sound->index = -1;

	int error;
	FILE* filePtr = NULL;
	unsigned int count;
	WaveHeaderType waveFileHeader;
	WAVEFORMATEX waveFormat;
	DSBUFFERDESC bufferDesc;
	HRESULT result;
	IDirectSoundBuffer* tempBuffer = NULL;
	unsigned char* waveData = NULL;
	unsigned char *bufferPtr = NULL;
	unsigned long bufferSize = NULL;

	if(fopen_s(&filePtr, filename, "rb") != 0)
		return E_FAIL;

	count = fread(&waveFileHeader, sizeof(waveFileHeader), 1, filePtr);

	if(count != 1)
	{
		return E_FAIL;
	}

	// Check that the chunk ID is the RIFF format.
	if((waveFileHeader.chunkId[0] != 'R') || (waveFileHeader.chunkId[1] != 'I') || 
	   (waveFileHeader.chunkId[2] != 'F') || (waveFileHeader.chunkId[3] != 'F'))
	{
		return E_FAIL;
	}
 
	// Check that the file format is the WAVE format.
	if((waveFileHeader.format[0] != 'W') || (waveFileHeader.format[1] != 'A') ||
	   (waveFileHeader.format[2] != 'V') || (waveFileHeader.format[3] != 'E'))
	{
		return E_FAIL;
	}
 
	// Check that the sub chunk ID is the fmt format.
	if((waveFileHeader.subChunkId[0] != 'f') || (waveFileHeader.subChunkId[1] != 'm') ||
	   (waveFileHeader.subChunkId[2] != 't') || (waveFileHeader.subChunkId[3] != ' '))
	{
		return E_FAIL;
	}
 

	// Check for the data chunk header.
	if((waveFileHeader.dataChunkId[0] != 'd') || (waveFileHeader.dataChunkId[1] != 'a') ||
	   (waveFileHeader.dataChunkId[2] != 't') || (waveFileHeader.dataChunkId[3] != 'a'))
	{
		return E_FAIL;
	}

	// Set the wave format of secondary buffer that this wave file will be loaded onto.
	waveFormat.wFormatTag = waveFileHeader.audioFormat;
	waveFormat.nSamplesPerSec = waveFileHeader.sampleRate;
	waveFormat.wBitsPerSample = waveFileHeader.bitsPerSample;
	waveFormat.nChannels = waveFileHeader.numChannels;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;
 
	// Set the buffer description of the secondary sound buffer that the wave file will be loaded onto.
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME;
	bufferDesc.dwBufferBytes = waveFileHeader.dataSize;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = &waveFormat;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	IDirectSoundBuffer8 * soundBuffer = NULL;

	result = lpds->CreateSoundBuffer(&bufferDesc, &tempBuffer, NULL);
	if(FAILED(result)) return result;
	result = tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&soundBuffer);
	if(FAILED(result)) return result;

	tempBuffer->Release();
	tempBuffer = 0;

	fseek(filePtr, sizeof(WaveHeaderType), SEEK_SET);

	waveData = new unsigned char[waveFileHeader.dataSize];

	count = fread(waveData, 1, waveFileHeader.dataSize, filePtr);

	error = fclose(filePtr);

	result = (soundBuffer)->Lock(0, waveFileHeader.dataSize, (void**)&bufferPtr, (DWORD*)&bufferSize, NULL, 0, 0);
	if(FAILED(result)) return result;

	memcpy(bufferPtr, waveData, waveFileHeader.dataSize);

	result = (soundBuffer)->Unlock((void*)bufferPtr, bufferSize, NULL, 0);
	if(FAILED(result)) return result;

	// Release the wave data since it was copied into the secondary buffer.
	delete [] waveData;
	waveData = 0;
 
	sounds.push_back(soundBuffer);

	sound->volume = 100;
	sound->index = sounds.size() - 1;

	return S_OK;
}

HRESULT DirectSoundDevice::Play(SGE::Sound::Sound *sound){
	if(sound->index == -1 || sound->index >= sounds.size())
		return E_FAIL;
	sounds[sound->index]->SetCurrentPosition(0);
	sounds[sound->index]->SetVolume((LONG)sound->volume);
	sounds[sound->index]->Play(0,0,0);
	return S_OK;
}

HRESULT DirectSoundDevice::Loop(SGE::Sound::Sound *sound){
	if(sound->index == -1 || sound->index >= sounds.size())
		return E_FAIL;
	sounds[sound->index]->SetCurrentPosition(0);
	sounds[sound->index]->SetVolume((LONG)sound->volume);
	sounds[sound->index]->Play(0,0,DSBPLAY_LOOPING);
	return S_OK;
}

HRESULT DirectSoundDevice::Stop(SGE::Sound::Sound *sound){
	if(sound->index == -1 || sound->index >= sounds.size())
		return E_FAIL;
	sounds[sound->index]->SetCurrentPosition(0);
	sounds[sound->index]->SetVolume((LONG)sound->volume);
	sounds[sound->index]->Stop();
	return S_OK;
}

HRESULT DirectSoundDevice::SetVolume(SGE::Sound::Sound *sound){
	if(sound->index == -1 || sound->index >= sounds.size())
		return E_FAIL;
	sounds[sound->index]->SetVolume((LONG)(sound->volume - 100) * 100);
	return S_OK;
	
}

HRESULT DirectSoundDevice::CleanUp(){
	for(UINT i =0; i < sounds.size(); i++){
		if(sounds[i]) sounds[i]->Release();
	}

	if(lpds) lpds->Release();
	return S_OK;
}
