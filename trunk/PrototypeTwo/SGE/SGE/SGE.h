// SGE.h

#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <stdio.h>



namespace SGEFramework {

    class Game{
	public:
		__declspec( dllexport ) Game();
		__declspec( dllexport ) ~Game();

		__declspec( dllexport ) void Run(HINSTANCE hInstance, int nCmdShow);

	private:
		HINSTANCE	hInst;
		HWND		mainWnd;


		HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
		HRESULT InitDevice();

	protected:
		void Initalize();
		void Draw();
		void Update();
		void LoadContent();


	};
}

namespace SGEGraphics {
	


}

namespace SGEInput{


}

namespace SGESound{


}