// SGE.h

#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <D3Dcompiler.h>
#include <xnamath.h>


using namespace System;


namespace SGEFramework {

	public ref class Game
	{
	public:

	private:


	protected:
		void Run();
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