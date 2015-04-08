
#pragma once
#include "SGE.h"
#include <dsound.h>
#include <vector>

namespace SGDS {

	class DirectSoundDevice : public SGE::Sound::SoundDevice{
		public:
			__declspec( dllexport ) DirectSoundDevice();
			__declspec( dllexport ) ~DirectSoundDevice();

			HRESULT InitializeDevice(HWND hWnd);
			HRESULT LoadWav(char* filename, SGE::Sound::Sound *sound);
			HRESULT PlaySound(SGE::Sound::Sound *sound);

		private:
			LPDIRECTSOUND8 lpds;
			std::vector<IDirectSoundBuffer8*> sounds;

			HRESULT CleanUp();
	};

}