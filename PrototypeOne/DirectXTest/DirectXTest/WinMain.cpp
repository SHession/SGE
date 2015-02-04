# include <Windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <D3Dcompiler.h>
#include <xnamath.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

//#pragma comment (lib, "dinput8.lib")

//Vertex structure
struct SimpleVertex{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexUV;
};

//VS constant buffer
struct CB_VS_PER_OBJECT{
	XMMATRIX gWorldViewProj;
};

//.obj mesh strucure
struct ObjMesh{
	SimpleVertex *vertices;
	WORD *indices;
	UINT numOfVertices;
	UINT numOfIndices;
};

//Game object strucure
struct GameObject{
	XMFLOAT3				position;
	XMFLOAT3				rotation;	
	XMVECTOR				initVecDir;
	XMVECTOR				curVecDir;
};

//Keyboard buttons being pressed
struct Keyboard{
	bool					w,a,s,d;

};


HINSTANCE	hInst = NULL;
HWND		mainWnd = NULL;
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_HARDWARE;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           g_pd3dDevice = NULL;
ID3D11DeviceContext*    g_pImmediateContext = NULL;
IDXGISwapChain*         g_pSwapChain = NULL;
ID3D11RenderTargetView* g_pRenderTargetView = NULL;
ID3D11VertexShader*     g_pVertexShader = NULL;
ID3D11PixelShader*      g_pPixelShader = NULL;
ID3D11InputLayout*      g_pVertexLayout = NULL;
ID3D11Buffer*           g_pVertexBuffer = NULL;
ID3D11Buffer*           g_pIndexBuffer = NULL;
ID3D11Buffer*           g_pConstantBuffer = NULL;

ID3D11SamplerState*                 g_pSamplerLinear = NULL;

ID3D11Texture2D*					g_pDepthStencil = NULL;
ID3D11DepthStencilView*				g_pDepthStencilView = NULL;

ID3D11ShaderResourceView           *g_pShipTextureResourceView   = NULL;
ID3D11ShaderResourceView           *g_pStationTextureResourceView   = NULL;

//Matrix data

XMMATRIX                g_World;
XMMATRIX                g_View;
XMMATRIX                g_Projection;

UINT					g_ShipIndices;
UINT					g_StationIndices;

GameObject					g_Ship;

Keyboard				g_Keyboard;

//Declerations 
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow );
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
void Render();
ObjMesh ObjLoader(char* filename);
void HandleKeyDown(WPARAM key);
void HandleKeyUp(WPARAM key);

void RotateBy(GameObject *obj, float x, float y, float z);
void MoveForward(GameObject *obj, float speed);


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow ){
	//Create the window and init the DirectX
	if( FAILED( InitWindow( hInstance, nCmdShow ) ) )
        return 0;
	if(FAILED(InitDevice() ) )
		return 0;


	MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
           TranslateMessage( &msg );
           DispatchMessage( &msg );
		   //If keyboard presses are detected handle them
		   if(msg.message = WM_KEYDOWN){
			   HandleKeyDown(msg.wParam);
		   }
		   if(msg.message = WM_KEYUP){
			   HandleKeyUp(msg.wParam);
		   }
		   
        }
        else
        {
		//Render the application
           Render();
        }
    }

	while(true){

	}

	//Cleanup created objects
	CleanupDevice();

    return ( int )msg.wParam;
}

//Create Windows application
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow ){

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof( WNDCLASSEX );
	wcex.style = CS_OWNDC;
	wcex.lpfnWndProc = WndProc;
	 wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"DirectXTest";
    wcex.hIconSm = NULL;
    if( !RegisterClassEx( &wcex ) )
        return E_FAIL;

	hInst = hInstance;
	RECT rc = {0,0,640,480};
	AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
	mainWnd = CreateWindow( L"DirectXTest", L"Direct3D 11 DirectXTest", WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
                           NULL );
	if( !mainWnd)
		return E_FAIL;

	ShowWindow(mainWnd, nCmdShow);


	return S_OK;
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch( message )
    {
        case WM_PAINT:
            hdc = BeginPaint( hWnd, &ps );
            EndPaint( hWnd, &ps );
            break;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;

        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}

//Load a shader from a file
HRESULT CompileShaderFromFile(WCHAR *szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut){
	
	HRESULT hr = S_OK;

	    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob;
	hr = D3DX11CompileFromFile(szFileName, NULL,NULL,szEntryPoint, szShaderModel,
		dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);
	if(FAILED(hr)){
		return hr;
	}
	if( pErrorBlob ) pErrorBlob->Release();

	return S_OK;
}

//Init the direct x pipeline
HRESULT InitDevice(){

	//Create a rect for the viewport based on the window size
	RECT rc;
	GetClientRect( mainWnd, &rc );
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
	sd.OutputWindow = mainWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;


	//create swap chain/ context and view
	HRESULT hr = D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, 0, 0,D3D11_SDK_VERSION, &sd, &g_pSwapChain
												,&g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);

	if(FAILED(hr)) return hr;


	//Add a render target view to the swap chain
	ID3D11Texture2D *BackBuffer;
	hr = g_pSwapChain->GetBuffer(0, _uuidof(ID3D11Texture2D), (LPVOID*) &BackBuffer);
	if(FAILED(hr)) return hr;
	hr = g_pd3dDevice->CreateRenderTargetView(BackBuffer,0,&g_pRenderTargetView);
	if(FAILED(hr)) return hr;

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

    hr = g_pd3dDevice->CreateTexture2D( &descDepth, NULL, &g_pDepthStencil );
    if( FAILED( hr ) ) return hr;

	//Create depth stencil view 
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory( &descDSV, sizeof(descDSV) );
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;

	hr = g_pd3dDevice->CreateDepthStencilView( g_pDepthStencil, &descDSV, &g_pDepthStencilView );
    if( FAILED( hr ) ) return hr;

	//Output merger stage
	g_pImmediateContext->OMSetRenderTargets( 1, &g_pRenderTargetView, g_pDepthStencilView );

	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = width;
	vp.Height = height;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;

	//Bind an array of viewports to the rasterizer stage of the pipeline.
	g_pImmediateContext->RSSetViewports(1,&vp);

	//Compile the vertex shader
	ID3DBlob *pVSBlob = NULL;
	hr = CompileShaderFromFile( L"VertexShader.hlsl", "VS", "vs_4_0", &pVSBlob );
	if(FAILED(hr)) return hr;

	//Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShader );
	if(FAILED(hr)) return hr;

	//Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	 {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
	UINT numElements = ARRAYSIZE( layout );

	//Create the input layout
	hr = g_pd3dDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
                                          pVSBlob->GetBufferSize(), &g_pVertexLayout );
	pVSBlob->Release();
	if(FAILED(hr)) return hr;

	//Set the input layout
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

	//Compile the pixel shader
	ID3DBlob *pPSBlob = NULL;
	hr = CompileShaderFromFile( L"PixelShader.hlsl", "PS", "ps_4_0", &pPSBlob );
	if(FAILED(hr)) return hr;

	//Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader);
	if(FAILED(hr)) return hr;

	//Meshes for testing
	//ObjMesh mesh = ObjLoader("Media\\Cup\\cup.obj");
	//ObjMesh mesh = ObjLoader("Media\\Textured_triangulated_Cube\\cube.obj");


	//Meshes taken fron TurboSquid
	ObjMesh shipMesh = ObjLoader("Media\\space_frigate_6\\space_frigate_6.obj");
	ObjMesh stationMesh = ObjLoader("Media\\space_station_4\\space_station_4.obj");

	UINT numOfVertices = shipMesh.numOfVertices + stationMesh.numOfVertices;
	SimpleVertex *tempVertices = new SimpleVertex[numOfVertices];
	for (int i =0; i < shipMesh.numOfVertices; i++)
	{
		tempVertices[i] = shipMesh.vertices[i];
	}

	for (int i = shipMesh.numOfVertices; i < shipMesh.numOfVertices + stationMesh.numOfVertices; i++)
	{
		tempVertices[i] = stationMesh.vertices[i - shipMesh.numOfVertices];
	}

	UINT numOfIndices = shipMesh.numOfIndices + stationMesh.numOfIndices;
	WORD *tempIndices = new WORD[numOfIndices];
	for (int i =0; i < shipMesh.numOfIndices; i++)
	{
		tempIndices[i] = shipMesh.indices[i];
	}

	for (int i = shipMesh.numOfIndices; i < shipMesh.numOfIndices + stationMesh.numOfIndices; i++)
	{
		tempIndices[i] = stationMesh.indices[i- shipMesh.numOfIndices];
	}


	g_ShipIndices = shipMesh.numOfIndices;
	g_StationIndices = stationMesh.numOfIndices;

	//Create a vertex buffer
	D3D11_BUFFER_DESC bd;
	SecureZeroMemory(&bd, sizeof(bd));
	bd.ByteWidth = sizeof(SimpleVertex) * (numOfVertices);
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;


	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = tempVertices;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
	if(FAILED(hr)) return hr;

	//Set vertex buffer
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0,1, &g_pVertexBuffer, &stride, &offset);



	//Create an index buffer
	SecureZeroMemory(&bd, sizeof(bd));
	bd.ByteWidth = sizeof(WORD) * numOfIndices;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	InitData.pSysMem = tempIndices;
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pIndexBuffer );
	if(FAILED(hr)) return hr;

	// Set index buffer
    g_pImmediateContext->IASetIndexBuffer( g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );

	if(shipMesh.indices) delete shipMesh.indices;
	if(shipMesh.vertices) delete shipMesh.vertices;
	if(stationMesh.indices) delete stationMesh.indices;
	if(stationMesh.vertices) delete stationMesh.vertices;
	if(tempIndices) delete tempIndices;
	if(tempVertices) delete tempVertices;

	// Set primitive topology
	g_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	//Create constant buffer(s)
	SecureZeroMemory(&bd, sizeof(bd));
	bd.ByteWidth = sizeof(CB_VS_PER_OBJECT);
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;

	hr = g_pd3dDevice->CreateBuffer(&bd, NULL, &g_pConstantBuffer);
	if(FAILED(hr)) return hr;

	//Load in textures
	//Create shader view 
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"Media\\space_frigate_6\\space_frigate_6_color.png", NULL, NULL, &g_pShipTextureResourceView, NULL);
	if(FAILED(hr)) return hr;
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"Media\\space_station_4\\space_station_4_color.jpg", NULL, NULL, &g_pStationTextureResourceView, NULL);
	if(FAILED(hr)) return hr;

	 // Create the sample state
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory( &sampDesc, sizeof(sampDesc) );
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = g_pd3dDevice->CreateSamplerState( &sampDesc, &g_pSamplerLinear );
    if( FAILED( hr ) )
        return hr;
		
	//Set texture to ship resource view
	g_pImmediateContext->PSSetShaderResources( 0, 1, &g_pShipTextureResourceView );

	// Initialize the world matrix
	g_World = XMMatrixIdentity();

    // Initialize the view matrix
	XMVECTOR Eye = XMVectorSet( 0, 3, -10, 0.0f );
	XMVECTOR At = XMVectorSet( 0.0f, 0.0f, 0.0f, 0.0f );
	XMVECTOR Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	g_View = XMMatrixLookAtLH( Eye, At, Up );

    // Initialize the projection matrix
	g_Projection = XMMatrixPerspectiveFovLH( XM_PIDIV2, width / (FLOAT)height, 0.01f, 1000.0f );

	//Set up ship
	g_Ship.position = XMFLOAT3(0,0,0);
	g_Ship.initVecDir = XMVectorSet(-1,0,0,0);
	g_Ship.curVecDir = XMVectorSet(1,0,0,0);
	g_Ship.rotation = XMFLOAT3(0,0,0);

	return S_OK;
}

//Render the application
void Render(){

	    // Update our time
    static float t = 0.0f;
    if( g_driverType == D3D_DRIVER_TYPE_REFERENCE )
    {
        t += ( float )XM_PI * 0.0125f;
    }
    else
    {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = GetTickCount();
        if( dwTimeStart == 0 )
            dwTimeStart = dwTimeCur;
        t = ( dwTimeCur - dwTimeStart ) / 1000.0f;
    }

	//clear the screen and set to black/grey
	float ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f }; //red,green,blue,alpha
    g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView, ClearColor );
	g_pImmediateContext->ClearDepthStencilView( g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );

	
	MoveForward(&g_Ship, 0.01f);

	//Handle ship matrices
	XMMATRIX matShipScale = XMMatrixScaling(0.2f,0.2f,0.2f);
	XMMATRIX matShipTranslate = XMMatrixTranslation(g_Ship.position.x,g_Ship.position.y,g_Ship.position.z + 2);
	XMMATRIX matShipRotate = XMMatrixRotationRollPitchYaw(g_Ship.rotation.x, g_Ship.rotation.y, g_Ship.rotation.z);
	g_World = matShipRotate * matShipTranslate * matShipScale;

	//Update the ship direction vector
	g_Ship.curVecDir = XMVector3TransformCoord(g_Ship.initVecDir, matShipRotate);
	g_Ship.curVecDir = XMVector3Normalize(g_Ship.curVecDir);

	//Change the view matrix to look at ship
	XMVECTOR localDir = g_Ship.curVecDir;
	localDir *= -10;

	XMVECTOR Eye = XMVectorSet( 0, 3, -10, 0.0f );
	Eye = XMVectorSet(g_Ship.position.x/5 + XMVectorGetX(localDir),
		              g_Ship.position.y/5 + XMVectorGetY(localDir) + 4,
					  g_Ship.position.z/5 + XMVectorGetZ(localDir),
					  0);


	XMVECTOR At = XMVectorSet( g_Ship.position.x/5,  g_Ship.position.y/5,  g_Ship.position.z/5, 0.0f );
	XMVECTOR Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	g_View = XMMatrixLookAtLH( Eye, At, Up );


	//Update constant buffer
	CB_VS_PER_OBJECT cb;
	cb.gWorldViewProj = XMMatrixTranspose(g_World * g_View * g_Projection);
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer,0,NULL,&cb,0,0);
	g_pImmediateContext->VSSetConstantBuffers( 0, 1, &g_pConstantBuffer );
	g_pImmediateContext->PSSetShaderResources( 0, 1, &g_pShipTextureResourceView );
	g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
	g_pImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );


	// Render the ship
	g_pImmediateContext->DrawIndexed(g_ShipIndices,0,0);


	//Move the station
	XMMATRIX matStationScale = XMMatrixScaling(1,1,1);
	XMMATRIX matStationTranslate = XMMatrixTranslation(-100,2,100);
	XMMATRIX matStationRotate = XMMatrixRotationX(t/4);
	g_World =   matStationRotate * matStationTranslate * matStationScale;

	//Update constant buffer
	cb.gWorldViewProj = XMMatrixTranspose(g_World * g_View * g_Projection);
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer,0,NULL,&cb,0,0);
	g_pImmediateContext->VSSetConstantBuffers( 0, 1, &g_pConstantBuffer );
	g_pImmediateContext->PSSetShaderResources( 0, 1, &g_pStationTextureResourceView );

	//Render the station
	g_pImmediateContext->DrawIndexed(g_StationIndices,894,378);

	//Render the orbiing station
	XMMATRIX matOrbitStation;
	 matStationScale = XMMatrixScaling(0.2f,0.2f,0.2f);
	 matStationTranslate = XMMatrixTranslation(0,150,0);
	 matStationRotate = XMMatrixRotationX(t);


	 matOrbitStation =   matStationTranslate * matStationRotate  *matStationScale;
	 matOrbitStation *= g_World;

	//Update constant buffer
	cb.gWorldViewProj = XMMatrixTranspose(matOrbitStation * g_View * g_Projection);
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer,0,NULL,&cb,0,0);
	g_pImmediateContext->VSSetConstantBuffers( 0, 1, &g_pConstantBuffer );
	g_pImmediateContext->PSSetShaderResources( 0, 1, &g_pStationTextureResourceView );

	//Render the orbiting station
	g_pImmediateContext->DrawIndexed(g_StationIndices,894,378);

    // Present the information rendered to the back buffer to the front buffer (the screen)
    g_pSwapChain->Present( 0, 0 );

}

//Clear up created objects
void CleanupDevice()
{
    if( g_pImmediateContext ) g_pImmediateContext->ClearState();

	 if( g_pSamplerLinear ) g_pSamplerLinear->Release();

    if( g_pVertexBuffer ) g_pVertexBuffer->Release();
    if( g_pVertexLayout ) g_pVertexLayout->Release();
    if( g_pVertexShader ) g_pVertexShader->Release();
    if( g_pPixelShader ) g_pPixelShader->Release();
    if( g_pRenderTargetView ) g_pRenderTargetView->Release();
    if( g_pSwapChain ) g_pSwapChain->Release();
    if( g_pImmediateContext ) g_pImmediateContext->Release();
    if( g_pd3dDevice ) g_pd3dDevice->Release();

	if(g_pShipTextureResourceView) g_pShipTextureResourceView->Release();
	if(g_pStationTextureResourceView) g_pStationTextureResourceView->Release();
}

//Obj Loader
ObjMesh ObjLoader(char* filename){	
	using namespace std;

	ifstream fileStream;
	string line;

	ObjMesh mesh;
	mesh.numOfIndices =0;
	mesh.numOfVertices =0;

	UINT numOfSubests = 0;

	//Vectors to store the found data
	vector<XMFLOAT3> vertices;
	vector<XMFLOAT2> uvs;
	vector<XMFLOAT3> normals;
	vector<XMFLOAT4> faces;

	//UINTs to store the size of the vectors
	UINT numOfVertices = 0;
	UINT numOfUVs = 0;
	UINT numOfNormals = 0;
	UINT numOfFaces = 0;

	//Vector of created vertices
	UINT numOfUVertices = 0;
	vector<SimpleVertex> uniqueVertices;
	//Vector of indices
	UINT numOfIndices = 0;
	vector<WORD> indices;

	//Open the file
	fileStream.open(filename);

	if(fileStream.is_open()){
		while(getline(fileStream, line)){

			//If a group is found
			if(line.compare(0,2,"g ") == 0){
				numOfSubests++;
				//if(numOfSubests >1)
				//	fileStream.close();
			}
			//If a vertex is found
			else if(line.compare(0,2,"v " ) == 0){

				//find all spaces
				size_t spaces[3];
				spaces[0]  = line.find(" ");
				spaces[1] = line.find(" ", spaces[0] + 2);
				spaces[2] = line.find(" ", spaces[1] + 2);

				//break up in to substrings using .substr
				string floats[3];
				floats[0] = line.substr(spaces[0], spaces[1] - spaces[0]);
				floats[1] = line.substr(spaces[1], spaces[2]  - spaces[1]);
				floats[2] = line.substr(spaces[2]);

				//Store the x,y,z coordinates of the vertex 
				XMFLOAT3 tempVertex = XMFLOAT3( stof(floats[0]),  stof(floats[1]), stof(floats[2]));
				vertices.push_back(tempVertex);
				numOfVertices ++;

			}
			//If a Texture UV is found
			else if(line.compare(0,2,"vt") == 0){

				//find all spaces
				size_t spaces[2];
				spaces[0]  = line.find(" ");
				spaces[1] = line.find(" ", spaces[0] + 2);

				//Break into substrings
				string floats[2];
				floats[0] = line.substr(spaces[0] , spaces[1] - spaces[0]);
				floats[1] = line.substr(spaces[1]);

				//Store the u,v coordinates
				XMFLOAT2 tempFloat2 = XMFLOAT2(stof(floats[0]),  stof(floats[1]));
				uvs.push_back(tempFloat2);
				numOfUVs++;

			}
			//If a normal is found
			else if(line.compare(0,2,"vn") == 0){
				//find all spaces
				size_t spaces[3];
				spaces[0]  = line.find(" ");
				spaces[1] = line.find(" ", spaces[0] + 2);
				spaces[2] = line.find(" ", spaces[1] + 2);

				//break up in to substrings usesing .substr
				string floats[3];
				floats[0] = line.substr(spaces[0], spaces[1] - spaces[0]);
				floats[1] = line.substr(spaces[1], spaces[2]  - spaces[1]);
				floats[2] = line.substr(spaces[2]);

				//Store the x,y,z of the normal
				XMFLOAT3 tempNormal = XMFLOAT3( stof(floats[0]),  stof(floats[1]), stof(floats[2]));
				normals.push_back(tempNormal);			
				numOfNormals++;

			}
			//If a row of faces is found
			else if(line.compare(0,2,"f ") == 0){
				//find all spaces
				size_t spaces[3];
				spaces[0]  = line.find(" ");
				spaces[1] = line.find(" ", spaces[0] + 2);
				spaces[2] = line.find(" ", spaces[1] + 2);

				//break up in to substrings usesing .substr
				string floats[3];
				floats[0] = line.substr(spaces[0] , spaces[1] - spaces[0]);
				floats[1] = line.substr(spaces[1] , spaces[2]  - spaces[1]);
				floats[2] = line.substr(spaces[2]);

				//For each element of the face definition (Assumes Tris)
				for(int i =0; i < 3; i++){
						//Find slashes 
						size_t slashes[2];
						slashes[0] = floats[i].find("/");
						slashes[1] = floats[i].find("/", slashes[0] + 1);

						XMFLOAT4 tempFace;

						//Check for file format (Supported V, V/VT, V/VT/VN)
						if(slashes[0] == string::npos)
						{
							 tempFace = XMFLOAT4 (stof(floats[i]), -1,-1, -1);
						}
						else if(slashes[1] == string::npos)
						{
							 tempFace = XMFLOAT4 (stof(floats[i].substr(0,slashes[0]))- 1, stof(floats[i].substr(slashes[0] + 1))-1,-1,-1);
						}
						else
						{
							 tempFace = XMFLOAT4 (stof(floats[i].substr(0,slashes[0]))- 1, stof(floats[i].substr(slashes[0] + 1, (slashes[1] - slashes[0]) - 1)) - 1, stof(floats[i].substr(slashes[1] + 1)) - 1, -1);
						}

						bool found = false;
						int temp = 0;

						//Check to see if the face if there is a vertex with that definition
						for(int i =0; i < numOfFaces; i++){
							if(faces[i].x == tempFace.x && faces[i].y == tempFace.y && faces[i].z == tempFace.z){
								//If found store its W value
								found = true;
								temp = faces[i].w;	
							}

						}

						//Push the face to the stored faces
						faces.push_back(tempFace);
						numOfFaces++;
						
						//If found push the index of where it was found 
						if(found){
							indices.push_back(temp);
							faces[numOfFaces - 1].w = temp; //Store the index of where it was found in the .w 
							numOfIndices++;

						}
						//If not found create a new unique vertex 
						else if (!found){
							SimpleVertex tempVertex;
							tempVertex.Pos = vertices[faces[numOfFaces - 1].x];

							if(faces[numOfFaces-1].y != -1){
								tempVertex.TexUV = //uvs[faces[numOfFaces - 1].y];
								XMFLOAT2( uvs[faces[numOfFaces - 1].y].x , 1 - uvs[faces[numOfFaces - 1].y].y);
							}

							if(faces[numOfFaces-1].z != -1){
								tempVertex.Normal = normals[faces[numOfFaces - 1].z];
							}

							uniqueVertices.push_back(tempVertex);
							numOfUVertices++;

							//Push the index of the new vertex and associate it with the face
							indices.push_back(numOfUVertices -1);
							faces[numOfFaces-1].w = numOfUVertices -1;		
							numOfIndices++;
						}
					}
				}
			}		
		}


	mesh.numOfVertices = numOfUVertices;
	mesh.numOfIndices = numOfFaces;

	//Create an array to store the vertices and indices
	mesh.vertices = new SimpleVertex[mesh.numOfVertices];
	mesh.indices = new WORD[mesh.numOfIndices];


	//Copy the values into the mesh
	for(int i= 0; i < numOfUVertices; i++){
		mesh.vertices[i] = uniqueVertices[i];
	}

	for(int i= 0; i < numOfFaces; i++){
		mesh.indices[i]= faces[i].w;
	}

	//Close the file and return
	fileStream.close();
	return mesh;

}

//Handle key down messages
void HandleKeyDown(WPARAM key){
	//If w pressed
	if(key == 0x57)
	{
		g_Keyboard.w = true;
		MoveForward(&g_Ship, 1 );

	}
	//If a pressed
	else if(key == 0x41)
	{
		g_Keyboard.a = true;	
		RotateBy(&g_Ship,0,-0.05f,0);
	}
	//If s pressed
	else if(key == 0x53)
	{
		g_Keyboard.s = true;

	}
	//If d pressed
	else if(key == 0x44)
	{
		g_Keyboard.d = true;
		RotateBy(&g_Ship,0,0.05f,0);
	}
	//If q pressed
	else if(key == 0x51)
	{
		RotateBy(&g_Ship,-0.05f,0,0);

	}
	//If e pressed
	else if(key == 0x45)
	{
		RotateBy(&g_Ship,0.05f,0,0);

	}

}

//Handle Key up messages
void HandleKeyUp(WPARAM key){
	//If w released
	if(key == 0x57)
	{
		g_Keyboard.w = false;
	}
	//If a released
	else if(key == 0x41)
	{
		g_Keyboard.a = false;

	}
	//If s released
	else if(key == 0x53)
	{
		g_Keyboard.s = false;

	}
	//If d released
	else if(key == 0x44)
	{
		g_Keyboard.d = false;

	}
}

//Changes the rotation values of an object
void RotateBy(GameObject *obj, float x, float y, float z){
	obj->rotation.x += x;
	obj->rotation.y += y;
	obj->rotation.z += z;

	if(obj->rotation.x > XM_PI*2)
		obj->rotation.x = 0;
	if(obj->rotation.y > XM_PI*2)
		obj->rotation.y = 0;
	if(obj->rotation.z > XM_PI*2)
		obj->rotation.z = 0;

	if(obj->rotation.x < 0)
		obj->rotation.x = XM_PI*2;
	if(obj->rotation.y < 0)
		obj->rotation.y = XM_PI*2;
	if(obj->rotation.z < 0)
		obj->rotation.z = XM_PI*2;

	obj->curVecDir = XMVectorSetX(obj->curVecDir, obj->rotation.x);
	obj->curVecDir = XMVectorSetY(obj->curVecDir, obj->rotation.y);
	obj->curVecDir = XMVectorSetZ(obj->curVecDir, obj->rotation.z);


};

//Moves the object forward based on its rotation vector
void MoveForward(GameObject *obj, float speed){
	obj->position.x += XMVectorGetX(obj->curVecDir) * speed; //* -g_Ship.speed;
	obj->position.y += XMVectorGetY(obj->curVecDir) * speed;// * g_Ship.speed;
	obj->position.z += XMVectorGetZ(obj->curVecDir) * speed; //* g_Ship.speed;
};
