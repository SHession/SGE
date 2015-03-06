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



namespace SGEFramework {

    __declspec( dllexport ) class Game{
	public:
		__declspec( dllexport ) Game();
		__declspec( dllexport ) ~Game();

		__declspec( dllexport ) void Run(HINSTANCE hInstance, int nCmdShow);

		  

	private:
		//Vertex structure
		struct SimpleVertex{
			DirectX::XMFLOAT3 Pos;
			DirectX::XMFLOAT3 Normal;
			DirectX::XMFLOAT2 TexUV;
		};

		//VS constant buffer
		struct CB_VS_PER_OBJECT{
			DirectX::XMMATRIX gWorldViewProj;
		};


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

		DirectX::XMMATRIX                g_World;
		DirectX::XMMATRIX                g_View;
		DirectX::XMMATRIX                g_Projection;

		clock_t time;

		float gameTime;
		float lastTime;
		float deltaTime;

		HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
		HRESULT InitDevice();

		

	protected:
		//.obj mesh strucure
		struct ObjMesh{
			SimpleVertex *vertices;
			WORD *indices;
			UINT numOfVertices;
			UINT numOfIndices;
			ID3D11ShaderResourceView* textureResourceView;
		};
		__declspec( dllexport ) virtual void DestroyObjMesh(ObjMesh *mesh) final;


		__declspec( dllexport ) virtual void Initalize();
		__declspec( dllexport ) virtual void Draw();
	    __declspec( dllexport ) virtual void Update(float deltaTime);
		__declspec( dllexport ) virtual void LoadContent();
		__declspec( dllexport ) virtual void CleanUp();


		__declspec( dllexport ) virtual void DrawMesh(ObjMesh*mesh) final;
		__declspec( dllexport ) virtual void Clear() final;
		__declspec( dllexport ) virtual HRESULT CreateVertexAndIndexBuffer(ObjMesh *mesh) final;

		__declspec( dllexport ) virtual HRESULT LoadObj(char* filename, ObjMesh* mesh) final;
		__declspec( dllexport ) virtual HRESULT LoadTexture(wchar_t* filename, ObjMesh* mesh) final;


	};
}

namespace SGEGraphics {
	
	

}

namespace SGEInput{


}

namespace SGESound{


}