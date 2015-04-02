
#pragma once
#include "SGE.h"

#include <dsound.h>

namespace SGDS {

	class DirectSoundDevice : public SGE::Sound::SoundDevice{
		public:
			__declspec( dllexport ) DirectSoundDevice();
			__declspec( dllexport ) ~DirectSoundDevice();

			HRESULT InitializeDevice(HWND hWnd);
			HRESULT CleanUp();

		private:
			LPDIRECTSOUND8 lpds;
				
	};

}