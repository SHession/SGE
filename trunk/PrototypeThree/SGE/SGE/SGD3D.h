
#pragma once
#include "SGE.h"

#include <d3d11.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <wincodec.h>
#include <dxgiformat.h>
#include <assert.h>

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>

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

		void Destroy(){
			if(indices) delete[] indices;
			if(vertices) delete[] vertices;
		}
	};

	struct CB_VS_PER_OBJECT{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX worldViewProj;
	};

	

	class DirectXDevice : public SGE::Graphics::GraphicDevice{
		public:
			__declspec( dllexport ) DirectXDevice();
			__declspec( dllexport ) ~DirectXDevice();

			 HRESULT InitializeDevice(SGE::Framework::GameDescription *gameDescription, HWND hWnd);
			 HRESULT ProcessContent();
			 HRESULT PositionCamera(SGE::Vector4 position, SGE::Vector4 at);
			 HRESULT Draw();
			 HRESULT DrawMesh(SGE::Graphics::Mesh*, SGE::Graphics::Texture* texture, SGE::Vector4 position, SGE::Vector4 scale, SGE::Vector4 rotation);
			 HRESULT DrawGameObject(SGE::Framework::GameObject*);
			 HRESULT Clear();
			 HRESULT Clear(float[4]);
			 HRESULT CleanUp();

			 HRESULT LoadObj(wchar_t * filename, SGE::Graphics::Mesh* mesh);				
		     HRESULT LoadTexture(wchar_t* filename, SGE::Graphics::Texture* texture);
			 HRESULT LoadVShader(wchar_t* filename, char* entryPoint, SGE::Graphics::VertexShader *shader);
			 HRESULT LoadPShader(wchar_t* filename, char* entryPoint, SGE::Graphics::PixelShader *shader);

			 HRESULT SetVShader(SGE::Graphics::VertexShader *shader);
			 HRESULT SetPShader(SGE::Graphics::PixelShader *shader);

			 HRESULT CreateConstantBuffer(size_t byteWidth, SGE::Graphics::ConstantBuffer *buffer);
			 HRESULT UpdateConstantBuffer(SGE::Graphics::CB * data, SGE::Graphics::ConstantBuffer *buffer );

		private:
			D3D_DRIVER_TYPE         driverType;
			D3D_FEATURE_LEVEL       featureLevel;
			ID3D11Device*           d3dDevice;
			ID3D11DeviceContext*    immediateContext;
			IDXGISwapChain*         swapChain;
			ID3D11RenderTargetView* renderTargetView;
			ID3D11Texture2D*		depthStencil;
			ID3D11DepthStencilState*depthStencilState;
			ID3D11DepthStencilState*disabledDepthStencilState;
			ID3D11DepthStencilView*	depthStencilView;
			ID3D11InputLayout*      inputLayout;
			ID3D11Buffer*           vertexBuffer;
			ID3D11Buffer*           indexBuffer;
			ID3D11Buffer*           constantBuffer;
			ID3D11SamplerState*     samplerLinear;
			IWICImagingFactory*		imagingFactory;
			IWICBitmapDecoder*		bitmapDecoder;

			std::vector<ID3D11VertexShader*> vertexShaders;
			std::vector<ID3D11PixelShader*> pixelShaders;
			std::vector<ID3D11Buffer*> constantBuffers;
			std::vector<ID3D11ShaderResourceView*> textures;
			std::vector<Mesh*> meshes;

			DirectX::XMFLOAT4X4 camera;
			DirectX::XMFLOAT4X4 projection;

			HRESULT CompileShader(LPCWSTR srcFile,LPCSTR entryPoint, LPCSTR profile, ID3DBlob** blob );
			DXGI_FORMAT _WICToDXGI( const GUID& guid );
			size_t _WICBitsPerPixel( REFGUID targetGuid , IWICImagingFactory *factory);

	};

}