// SGE.h

#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <wincodec.h>
#include <stdio.h>
#include <ctime>
#include <dsound.h>


namespace SGEFramework {


	struct SimpleVertex{
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT2 TexUV;
	};

	//VS constant buffer
	struct CB_VS_PER_OBJECT{
		DirectX::XMMATRIX gWorldViewProj;
	};

	struct Mesh{
		SimpleVertex *vertices;
		WORD *indices;
		UINT numOfVertices;
		UINT numOfIndices;
		ID3D11ShaderResourceView* textureResourceView;

		UINT startVertex;
		UINT startIndex;
	};


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

   class Game{
	public:
		__declspec( dllexport ) Game();
		__declspec( dllexport ) ~Game();

		__declspec( dllexport ) void Run(HINSTANCE hInstance, int nCmdShow);

		  

	private:
		HINSTANCE	hInst;
		HWND		mainWnd;
		D3D_DRIVER_TYPE         g_driverType;
		D3D_FEATURE_LEVEL       g_featureLevel;
		ID3D11Device*           g_pd3dDevice;
		ID3D11DeviceContext*    g_pImmediateContext;
		IDXGISwapChain*         g_pSwapChain;
		ID3D11RenderTargetView* g_pRenderTargetView;
		ID3D11VertexShader*     g_pVertexShader;
		ID3D11PixelShader*      g_pPixelShader;
		ID3D11InputLayout*      g_pVertexLayout;
		ID3D11Buffer*           g_pVertexBuffer;
		ID3D11Buffer*           g_pIndexBuffer;
		ID3D11Buffer*           g_pConstantBuffer;
		ID3D11Texture2D*		g_pDepthStencil;
		ID3D11DepthStencilView*	g_pDepthStencilView;
		ID3D11SamplerState*                 g_pSamplerLinear;

		IWICImagingFactory* g_pFactory;
		IWICBitmapDecoder* g_pDecoder;

		LPDIRECTSOUND8 lpds;
		IDirectSoundBuffer8 * g_pSoundBuffer;

		DirectX::XMMATRIX                g_World;
		DirectX::XMMATRIX                g_Projection;

		GameTime		gameTime;

		HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
		HRESULT InitDevice();
		HRESULT HandleInputs(MSG msg);

		static HRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );

	protected:
		//.obj mesh strucure

		struct Keyboard{
			int w,a,s,d,space;
			bool x;
		};

		Keyboard keyboard;
		DirectX::XMMATRIX                Camera;

		__declspec( dllexport ) virtual void Initalize();
		__declspec( dllexport ) virtual void Draw();
	    __declspec( dllexport ) virtual void Update(GameTime gameTime);
		__declspec( dllexport ) virtual void LoadContent();
		__declspec( dllexport ) virtual void CleanUp();


		__declspec( dllexport ) virtual void DrawMesh(Mesh*mesh, DirectX::XMMATRIX *world) final;
		__declspec( dllexport ) virtual void Clear() final;
		__declspec( dllexport ) virtual HRESULT CreateVertexAndIndexBuffer(Mesh *meshes[], int numOfMeshes) final;

		__declspec( dllexport ) virtual HRESULT LoadObj(wchar_t * filename, Mesh* mesh) final;
		__declspec( dllexport ) virtual HRESULT LoadTexture(wchar_t* filename, Mesh* mesh) final;
		__declspec( dllexport ) virtual HRESULT LoadWave(char* filename) final;

	};

	 __declspec( dllexport ) void DestroyMesh(Mesh *mesh);


}

namespace SGEGraphics {
	
	

}

namespace SGEInput{


}

namespace SGESound{


}