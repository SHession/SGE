
#pragma once
#include "SGE.h"

#include <d3d11.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include <vector>
#include <iostream>
#include <fstream>
#include <string>

namespace SGD3D {

	struct Vertex{
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT2 TexUV;
	};

	struct Mesh{
		Vertex *vertices;
		WORD *indices;
		UINT numOfVertices;
		UINT numOfIndices;
		UINT startVertex;
		UINT startIndex;
	};

	void DestroyMesh(Mesh *mesh);

	class DirectXDevice : public SGE::Graphics::GraphicDevice{
		public:
			__declspec( dllexport ) DirectXDevice();
			__declspec( dllexport ) ~DirectXDevice();

			HRESULT InitializeDevice(HWND hWnd);
			HRESULT Draw();
			HRESULT DrawMesh(SGE::Graphics::Mesh);
			HRESULT DrawGameObject(SGE::Framework::GameObject);
			HRESULT Clear();

			HRESULT LoadObj(wchar_t * filename, SGE::Graphics::Mesh* mesh);				
		    HRESULT LoadTexture(wchar_t* filename, SGE::Graphics::Texture* texture);
			HRESULT LoadVShader(wchar_t* filename, char* entryPoint, SGE::Graphics::VertexShader *shader);
			HRESULT LoadPShader(wchar_t* filename, char* entryPoint, SGE::Graphics::PixelShader *shader);

		private:
			D3D_DRIVER_TYPE         driverType;
			D3D_FEATURE_LEVEL       featureLevel;
			ID3D11Device*           d3dDevice;
			ID3D11DeviceContext*    immediateContext;
			IDXGISwapChain*         swapChain;
			ID3D11RenderTargetView* renderTargetView;
			ID3D11Texture2D*		depthStencil;
			ID3D11DepthStencilView*	depthStencilView;

			std::vector<ID3D11VertexShader*> vertexShaders;
			std::vector<ID3D11PixelShader*> pixelShaders;
			std::vector<ID3D11ShaderResourceView*> textures;
			std::vector<Mesh*> meshes;

			HRESULT CleanUp();
			HRESULT CompileShader(LPCWSTR srcFile,LPCSTR entryPoint, LPCSTR profile, ID3DBlob** blob );

	};

}