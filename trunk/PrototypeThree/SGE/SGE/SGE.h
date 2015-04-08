// SGE.h

#pragma once
#include <Windows.h>
#include <ctime>


namespace SGE {
	//Forward Declertaions
	namespace Framework {
		class GameObject;
	}

	namespace Graphics {
		struct VertexShader{
			int index;
		};

		struct PixelShader{
			int index;
		};

		struct Mesh{
			int index;
		};

		struct Texture{
			int index;
		};

		//Abstract class for a graphics device
		class GraphicDevice{
			public:
				virtual HRESULT InitializeDevice(HWND hWnd) = 0;
				virtual HRESULT Draw() = 0;
				virtual HRESULT DrawMesh(Graphics::Mesh) = 0;
				virtual HRESULT DrawGameObject(Framework::GameObject) = 0;
				virtual HRESULT Clear() = 0;
				virtual HRESULT CleanUp() = 0;

				
				virtual HRESULT LoadObj(wchar_t * filename, Mesh* mesh) = 0;				
				virtual HRESULT LoadTexture(wchar_t* filename, Texture* texture) = 0;
				virtual HRESULT LoadVShader(wchar_t* filename, char* entryPoint, VertexShader *shader) = 0;
				virtual HRESULT LoadPShader(wchar_t* filename, char* entryPoint, PixelShader *shader) = 0;
		};
	}

	namespace Sound{
		//Abstract class for a sound device
		struct Sound{
			int index;
			float volume;
		};

		class SoundDevice{
			public:
				virtual HRESULT InitializeDevice(HWND hWnd) = 0;
				virtual HRESULT LoadWav(char* filename, Sound *sound) = 0;
				virtual HRESULT PlaySound(Sound *sound) = 0;
				virtual HRESULT CleanUp() = 0;
		};

	}

	namespace Framework {
		//Tracks the time stats of a game
		class GameTime{
			public:
				GameTime();
				void Update();
				__declspec( dllexport ) double GetDeltaTime();
				__declspec( dllexport ) double GetElapsedTime();

			private:
				clock_t elapsedTime;
				clock_t deltaTime;
				clock_t lastTime;
		};

		//Tracks keyboard inputs
		class GameInputs{
			public:
				GameInputs();
				void HandleInput(MSG msg);
		};

		//Game object class can be used for objects within a game
		class GameObject{
			public:
				__declspec( dllexport ) GameObject();
				__declspec( dllexport ) ~GameObject();

				__declspec( dllexport ) virtual void Update(GameTime gameTime);
			private:

			protected:

		};

		//Game class provides the structure of a game similar to XNA 
		class Game{
			public:
				__declspec( dllexport ) Game();
				__declspec( dllexport ) ~Game();
				__declspec( dllexport ) HRESULT Run(HINSTANCE hInstance, int nCmdShow);
			private:
				HINSTANCE						hInst;
				HWND							mainWnd;
				GameTime						gameTime;

				HRESULT InitializeWindow(HINSTANCE hInstance, int nCmdShow);
				static HRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
			protected:
				Graphics::GraphicDevice *		graphics;
				Sound::SoundDevice *			sound;

				__declspec( dllexport ) virtual void LoadContent();
				__declspec( dllexport ) virtual void Initialize();
				__declspec( dllexport ) virtual void Update(GameTime gameTime);
				__declspec( dllexport ) virtual void Draw();
				__declspec( dllexport ) virtual void CleanUp();
		};
	}
}