#include "stdafx.h"
#include "SGD3D.h"

using namespace SGD3D;

DirectXDevice::DirectXDevice(){
	driverType = D3D_DRIVER_TYPE_HARDWARE;
	featureLevel = D3D_FEATURE_LEVEL_11_0;
	d3dDevice = NULL;
	immediateContext = NULL;
	swapChain = NULL;

}

DirectXDevice::~DirectXDevice(){


}

HRESULT DirectXDevice::InitializeDevice(HWND hWnd){
	//Create a rect for the viewport based on the window size
	RECT rc;
	GetClientRect( hWnd, &rc );
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
	#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif

	//Describe swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount =1;
	sd.OutputWindow = hWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	//create swap chain/ context and view
	HRESULT result = D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, 0, 0,D3D11_SDK_VERSION, &sd, &swapChain
												,&d3dDevice, &featureLevel, &immediateContext);

	if (FAILED(result)) return E_FAIL;

	//Add a render target view to the swap chain
	ID3D11Texture2D *BackBuffer;
	result = swapChain->GetBuffer(0, _uuidof(ID3D11Texture2D), (LPVOID*) &BackBuffer);
	if(FAILED(result)) return result;
	result = d3dDevice->CreateRenderTargetView(BackBuffer,0,&renderTargetView);
	if(FAILED(result)) return result;

	//Add a depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;

    result = d3dDevice->CreateTexture2D( &descDepth, NULL, &depthStencil );
    if( FAILED( result ) ) return result;

	//Create depth stencil view 
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory( &descDSV, sizeof(descDSV) );
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;

	result = d3dDevice->CreateDepthStencilView( depthStencil, &descDSV, &depthStencilView );
    if( FAILED( result ) ) return result;

	//Output merger stage
	immediateContext->OMSetRenderTargets( 1, &renderTargetView, depthStencilView );

	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = width;
	vp.Height = height;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;

	//Bind an array of viewports to the rasterizer stage of the pipeline.
	immediateContext->RSSetViewports(1,&vp);


	return S_OK;
}

HRESULT DirectXDevice::Draw(){

	return S_OK;
}

HRESULT DirectXDevice::DrawGameObject(SGE::Framework::GameObject){

	return S_OK;
}

HRESULT DirectXDevice::Clear(){

	return S_OK;
}

HRESULT DirectXDevice::CleanUp(){

	if(d3dDevice) d3dDevice->Release();
	if(immediateContext) immediateContext->Release();
	if(swapChain) swapChain->Release();
	if(renderTargetView) renderTargetView->Release();
	if(depthStencil) depthStencil->Release();
	if(depthStencilView) depthStencilView->Release();

	return S_OK;
}

HRESULT DirectXDevice::CompileShader(LPCWSTR srcFile,LPCSTR entryPoint, LPCSTR profile, ID3DBlob** blob ){
	   	
	if ( !srcFile || !entryPoint || !profile || !blob )
       return E_INVALIDARG;

    *blob = nullptr;

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    flags |= D3DCOMPILE_DEBUG;
#endif

    const D3D_SHADER_MACRO defines[] = 
    {
        "EXAMPLE_DEFINE", "1",
        NULL, NULL
    };

    ID3DBlob* shaderBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    HRESULT result = D3DCompileFromFile( srcFile, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                     entryPoint, profile,
                                     flags, 0, &shaderBlob, &errorBlob );
    if ( FAILED(result) )
    {
        if ( errorBlob )
        {
            OutputDebugStringA( (char*)errorBlob->GetBufferPointer() );
            errorBlob->Release();
        }

        if ( shaderBlob )
           shaderBlob->Release();

        return result;
    }    

    *blob = shaderBlob;

    return result;

}