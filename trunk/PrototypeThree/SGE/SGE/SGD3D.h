
#pragma once
#include "SGE.h"

#include <d3d11.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

namespace SGD3D {

	class DirectXDevice : public SGE::Graphics::GraphicDevice{
		public:
			__declspec( dllexport ) DirectXDevice();
			__declspec( dllexport ) ~DirectXDevice();

			HRESULT InitializeDevice(HWND hWnd);
			HRESULT Draw();
			HRESULT DrawGameObject(SGE::Framework::GameObject);
			HRESULT Clear();
			HRESULT CleanUp();
		private:
			D3D_DRIVER_TYPE         driverType;
			D3D_FEATURE_LEVEL       featureLevel;
			ID3D11Device*           d3dDevice;
			ID3D11DeviceContext*    immediateContext;
			IDXGISwapChain*         swapChain;
			ID3D11RenderTargetView* renderTargetView;
			ID3D11Texture2D*		depthStencil;
			ID3D11DepthStencilView*	depthStencilView;

			HRESULT CompileShader(LPCWSTR srcFile,LPCSTR entryPoint, LPCSTR profile, ID3DBlob** blob );

	};

}