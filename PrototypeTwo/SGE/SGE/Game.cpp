// This is the main DLL file.

#include "stdafx.h"
#include "SGE.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <dxgiformat.h>
#include <assert.h>
#include <memory>

using namespace SGEFramework;

//Forward Declerations
HRESULT CompileShader( _In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob );
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
size_t _WICBitsPerPixel( REFGUID targetGuid , IWICImagingFactory *factory);
DXGI_FORMAT _WICToDXGI( const GUID& guid );


Game::Game(){
	

}

Game::~Game(){
	CleanUp();
}

void Game::Run(HINSTANCE hInstance, int nCmdShow){
	InitWindow(hInstance, nCmdShow);
	InitDevice();
	LoadContent();
	Initalize();

	MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
           TranslateMessage( &msg );
           DispatchMessage( &msg );		   

		   if(msg.message == WM_KEYDOWN || msg.message == WM_KEYUP || msg.message == WM_CHAR)
		   {
			   OutputDebugStringW(L"\n Handling Inputs");
			   HandleInputs(msg);		
		   }
        }

		//Render the application
		Update(gameTime);
		Draw();		
    }
}

HRESULT Game::InitWindow( HINSTANCE hInstance, int nCmdShow ){


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

HRESULT Game::InitDevice(){
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
	hr = CompileShader( L"VertexShader.hlsl", "VS", "vs_4_0", &pVSBlob );
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
	hr = CompileShader( L"PixelShader.hlsl", "PS", "ps_4_0", &pPSBlob );
	if(FAILED(hr)) return hr;

	//Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader);
	if(FAILED(hr)) return hr;

	//Create constant buffer(s)
	D3D11_BUFFER_DESC bd;
	SecureZeroMemory(&bd, sizeof(bd));
	bd.ByteWidth = sizeof(CB_VS_PER_OBJECT);
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;

	hr = g_pd3dDevice->CreateBuffer(&bd, NULL, &g_pConstantBuffer);
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


	g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
	g_pImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );

	g_pImmediateContext->PSSetSamplers(0,1,&g_pSamplerLinear);

	using namespace DirectX;

	DirectSoundCreate8(NULL, &lpds,NULL);
	lpds->SetCooperativeLevel(mainWnd, DSSCL_NORMAL);


	//Handle ship matrices
	g_World =   XMMatrixIdentity();


	    // Initialize the view matrix
	XMVECTOR Eye = XMVectorSet( 0, 3, -10, 0.0f );
	XMVECTOR At = XMVectorSet( 0.0f, 0.0f, 0.0f, 0.0f );
	XMVECTOR Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	Camera = XMMatrixLookAtLH( Eye, At, Up );

    // Initialize the projection matrix
	g_Projection = XMMatrixPerspectiveFovLH( XM_PIDIV2, width / (FLOAT)height, 0.01f, 1000.0f );

		//Update constant buffer
	CB_VS_PER_OBJECT cb;
	cb.gWorld = XMMatrixTranspose(g_World);
	cb.gWorldViewProj = XMMatrixTranspose(g_World * Camera * g_Projection);
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer,0,NULL,&cb,0,0);

	keyboard.w =0;
	keyboard.a =0;
	keyboard.s =0;
	keyboard.d =0;
	keyboard.space=0;

	return S_OK;
}

void Game::Initalize(){


}

void Game::Draw(){
	CB_VS_PER_OBJECT cb;
	cb.gWorld = XMMatrixTranspose(g_World);
	cb.gWorldViewProj = XMMatrixTranspose(g_World * Camera * g_Projection);
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer,0,NULL,&cb,0,0);
	g_pImmediateContext->VSSetConstantBuffers( 0, 1, &g_pConstantBuffer );

	g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
	g_pImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );
	
	gameTime.Update();
	g_pSwapChain->Present( 1, 0 );
}

void Game::Update(GameTime gameTime){
	puts("Running");
}

void Game::LoadContent(){

}

void Game::CleanUp(){
	if( g_pImmediateContext ) g_pImmediateContext->ClearState();
    if( g_pVertexBuffer ) g_pVertexBuffer->Release();
    if( g_pVertexLayout ) g_pVertexLayout->Release();
    if( g_pVertexShader ) g_pVertexShader->Release();
    if( g_pPixelShader ) g_pPixelShader->Release();
    if( g_pRenderTargetView ) g_pRenderTargetView->Release();
    if( g_pSwapChain ) g_pSwapChain->Release();
    if( g_pImmediateContext ) g_pImmediateContext->Release();
    if( g_pd3dDevice ) g_pd3dDevice->Release();
	if(g_pDepthStencil) g_pDepthStencil->Release();
	if(g_pDepthStencilView) g_pDepthStencilView->Release();

	if(g_pSamplerLinear) g_pSamplerLinear->Release();

	if(g_pFactory) g_pFactory->Release();
	if(g_pDecoder) g_pDecoder->Release();

}

void Game::DrawMesh(Mesh*mesh, DirectX::XMMATRIX *world){
	if(mesh){
		CB_VS_PER_OBJECT cb;
		cb.gWorld = XMMatrixTranspose(g_World);
		cb.gWorldViewProj = XMMatrixTranspose((*world) * Camera * g_Projection);
		g_pImmediateContext->UpdateSubresource(g_pConstantBuffer,0,NULL,&cb,0,0);

		g_pImmediateContext->PSSetShaderResources( 0, 1, &mesh->textureResourceView );
		g_pImmediateContext->DrawIndexed(mesh->numOfIndices,mesh->startIndex,mesh->startVertex);
	}
}

void Game::Clear(){
	float ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f }; //red,green,blue,alpha
    g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView, ClearColor );
	g_pImmediateContext->ClearDepthStencilView( g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );
}

HRESULT Game::CreateVertexAndIndexBuffer(Mesh *meshes[], int numOfMeshes){

	SimpleVertex *vertices;
	WORD * indices;

	int numOfVertices = 0;
	int numOfIndices = 0;

	int totalVertices = 0;
	int totalIndices = 0;

	for(int i=0; i <numOfMeshes; i++){
		totalVertices += meshes[i]->numOfVertices;
		totalIndices += meshes[i]->numOfIndices;
	}

	vertices = new SimpleVertex[totalVertices];
	indices = new WORD[totalIndices];

	for(int i=0; i <numOfMeshes; i++){

		for(int j = 0; j < meshes[i]->numOfVertices; j++){
			vertices[j + numOfVertices] = meshes[i]->vertices[j];
		}
		meshes[i]->startVertex = numOfVertices;
		numOfVertices += meshes[i]->numOfVertices;
		

		for(int j = 0; j < meshes[i]->numOfIndices; j++){
			indices[j + numOfIndices] = meshes[i]->indices[j];
		}

		meshes[i]->startIndex = numOfIndices;
		numOfIndices += meshes[i]->numOfIndices;
	}

	


	HRESULT hr;

	//Create a vertex buffer
	D3D11_BUFFER_DESC bd;
	SecureZeroMemory(&bd, sizeof(bd));
	bd.ByteWidth = sizeof(SimpleVertex) * numOfVertices;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;


	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;
	hr =  g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
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

	InitData.pSysMem = indices;
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pIndexBuffer );
	if(FAILED(hr)) return hr;

	// Set index buffer
    g_pImmediateContext->IASetIndexBuffer( g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );

	// Set primitive topology
	g_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	if(vertices) delete vertices;
	if(indices) delete indices;


	return S_OK;
}

HRESULT Game::LoadObj(wchar_t* filename, Mesh* mesh){
	using namespace std;
	using namespace DirectX;

	ifstream fileStream;
	string line;

	mesh->numOfIndices =0;
	mesh->numOfVertices =0;
	mesh->startIndex =0;
	mesh->startVertex =0;
	mesh->textureResourceView = nullptr;
	OutputDebugStringW(L"\n Inits");


	UINT numOfSubests = 0;
	//Vectors to store the found data
	vector<XMFLOAT3> vertices (0);
	vector<XMFLOAT2> uvs (0);
	vector<XMFLOAT3> normals (0);
	vector<XMFLOAT4> faces (0);

	//UINTs to store the size of the vectors
	UINT numOfVertices = 0;
	UINT numOfUVs = 0;
	UINT numOfNormals = 0;
	UINT numOfFaces = 0;

	//Vector of created vertices
	UINT numOfUVertices = 0;
	vector<SimpleVertex> uniqueVertices (0);
	//Vector of indices
	UINT numOfIndices = 0;
	vector<WORD> indices (0);

	//Temp variables
	XMFLOAT2 tempFloat2;
	XMFLOAT3 tempFloat3;
	XMFLOAT4 tempFloat4;
	size_t spaces[3];
	string floats[3];
	size_t slashes[2];
	int position;
	bool found;
	SimpleVertex tempVertex;
	int times;

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
				spaces[0]  = line.find(" ");
				spaces[1] = line.find(" ", spaces[0] + 2);
				spaces[2] = line.find(" ", spaces[1] + 2);

				//break up in to substrings using .substr
				floats[0] = line.substr(spaces[0], spaces[1] - spaces[0]);
				floats[1] = line.substr(spaces[1], spaces[2]  - spaces[1]);
				floats[2] = line.substr(spaces[2]);

				//Store the x,y,z coordinates of the vertex 
				tempFloat3 = XMFLOAT3( stof(floats[0]),  stof(floats[1]), stof(floats[2]));
				vertices.push_back(tempFloat3);
				numOfVertices ++;

			}
			//If a Texture UV is found
			else if(line.compare(0,2,"vt") == 0){

				//find all spaces
				spaces[0]  = line.find(" ");
				spaces[1] = line.find(" ", spaces[0] + 2);

				//Break into substrings
				floats[0] = line.substr(spaces[0] , spaces[1] - spaces[0]);
				floats[1] = line.substr(spaces[1]);

				//Store the u,v coordinates
				tempFloat2 = XMFLOAT2(stof(floats[0]),  stof(floats[1]));
				uvs.push_back(tempFloat2);
				numOfUVs++;

			}
			//If a normal is found
			else if(line.compare(0,2,"vn") == 0){
				//find all spaces
				spaces[0]  = line.find(" ");
				spaces[1] = line.find(" ", spaces[0] + 2);
				spaces[2] = line.find(" ", spaces[1] + 2);

				//break up in to substrings usesing .substr
				floats[0] = line.substr(spaces[0], spaces[1] - spaces[0]);
				floats[1] = line.substr(spaces[1], spaces[2]  - spaces[1]);
				floats[2] = line.substr(spaces[2]);

				//Store the x,y,z of the normal
				tempFloat3 = XMFLOAT3( stof(floats[0]),  stof(floats[1]), stof(floats[2]));
				normals.push_back(tempFloat3);			
				numOfNormals++;

			}
			//If a row of faces is found
			else if(line.compare(0,2,"f ") == 0){
				//find all spaces
				size_t spaces[4];
				spaces[0]  = line.find(" ");
				spaces[1] = line.find(" ", spaces[0] + 2);
				spaces[2] = line.find(" ", spaces[1] + 2);
				spaces[3] = line.find(" ", spaces[2] + 2);

				times = 3;
				//break up in to substrings usesing .substr
				string floats[4];
				floats[0] = line.substr(spaces[0] , spaces[1] - spaces[0]);
				floats[1] = line.substr(spaces[1] , spaces[2]  - spaces[1]);
				floats[2] = line.substr(spaces[2]);

				//If there are four vertices
				if(spaces[3] != string::npos){	
					times = 4;	
					floats[2] = line.substr(spaces[2], spaces[3]  - spaces[2]);
					floats[3] = line.substr(spaces[3]);
				}

				//For each element of the face definition (Assumes Tris)
				for(int i =0; i < times; i++){
						//Find slashes 
						slashes[0] = floats[i].find("/");
						slashes[1] = floats[i].find("/", slashes[0] + 1);

						//Check for file format (Supported V, V/VT, V/VT/VN)
						if(slashes[0] == string::npos)
						{
							tempFloat4 = XMFLOAT4 (stof(floats[i]) - 1, -1,-1, -1);
						}
						else if(slashes[1] == string::npos)
						{
							 tempFloat4 = XMFLOAT4 (stof(floats[i].substr(0,slashes[0]))- 1, stof(floats[i].substr(slashes[0] + 1))-1,-1,-1);
						}
						else
						{
							 tempFloat4 = XMFLOAT4 (stof(floats[i].substr(0,slashes[0]))- 1, stof(floats[i].substr(slashes[0] + 1, (slashes[1] - slashes[0]) - 1)) - 1, stof(floats[i].substr(slashes[1] + 1)) - 1, -1);
						}

						found = false;
						position = 0;

						//Check to see if the face if there is a vertex with that definition
						for(int j =0; j < numOfFaces; j++){
							if(faces[j].x == tempFloat4.x && faces[j].y == tempFloat4.y && faces[j].z == tempFloat4.z){
								//If found store its W value
								found = true;
								position = faces[j].w;	
							}

						}

						//Push the face to the stored faces
						faces.push_back(tempFloat4);
						numOfFaces++;
						
						//If found push the index of where it was found 
						if(found){
							if( i< 3){
								indices.push_back(position);
								numOfIndices++;
							}
							else{
								indices.push_back(indices[numOfIndices - 3]);
								numOfIndices++;
								indices.push_back(indices[numOfIndices - 2]);
								numOfIndices++;
								indices.push_back(position);
								numOfIndices++;
							}
							
							faces[numOfFaces - 1].w = position; //Store the index of where it was found in the .w 
						}
						//If not found create a new unique vertex 
						else if (!found){
							tempVertex.Pos = vertices[faces[numOfFaces - 1].x];

							if(faces[numOfFaces-1].y != -1){
								tempVertex.TexUV = 
									XMFLOAT2( uvs[faces[numOfFaces - 1].y].x , 1 - uvs[faces[numOfFaces - 1].y].y);
							}
							else 
								tempVertex.TexUV = XMFLOAT2(0,0);

							if(faces[numOfFaces-1].z != -1){
								tempVertex.Normal = normals[faces[numOfFaces - 1].z];
							}
							else 
								tempVertex.Normal = XMFLOAT3(0,0,0);

							uniqueVertices.push_back(tempVertex);
							numOfUVertices++;

							//Push the index of the new vertex and associate it with the face
							if( i< 3){
								indices.push_back(numOfUVertices -1);	
								numOfIndices++;
							}
							else{
								indices.push_back(indices[numOfIndices - 3]);
								numOfIndices++;
								indices.push_back(indices[numOfIndices - 2]);
								numOfIndices++;
								indices.push_back(numOfUVertices -1);	
								numOfIndices++;
							}

							faces[numOfFaces-1].w = numOfUVertices -1;	
						}
					}
				}
			}		
		}
		else{
			OutputDebugStringW(L"\n Failed to open");
			return E_FAIL;
		}


	OutputDebugStringW(L"\n Copyin");

	mesh->numOfVertices = numOfUVertices;
	mesh->numOfIndices = numOfIndices;

	//Create an array to store the vertices and indices
	mesh->vertices = new SimpleVertex[mesh->numOfVertices];
	mesh->indices = new WORD[mesh->numOfIndices];


	//Copy the values into the mesh
	for(UINT i= 0; i < numOfUVertices; i++){
		mesh->vertices[i] = uniqueVertices[i];
	}

	for(UINT i= 0; i < numOfIndices; i++){
		mesh->indices[i]= indices[i];
	}

	//Close the file and return
	fileStream.close();


	OutputDebugStringW(L"\n Done");
	

	return S_OK;
}

HRESULT Game::LoadTexture(wchar_t* filename, Mesh* mesh){
	CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), (LPVOID*)&g_pFactory);

	g_pFactory->CreateDecoderFromFilename(filename, 0, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &g_pDecoder);

	IWICBitmapFrameDecode* frame;
	g_pDecoder->GetFrame(0, &frame);

	WICPixelFormatGUID pixelFormat;
	frame->GetPixelFormat(&pixelFormat);

	DXGI_FORMAT format = _WICToDXGI( pixelFormat);

	WICPixelFormatGUID convertGUID;
    memcpy( &convertGUID, &pixelFormat, sizeof(WICPixelFormatGUID) );

	UINT width, height;

	frame->GetSize(&width, &height);

	size_t bpp = 0;
	bpp = _WICBitsPerPixel( pixelFormat, g_pFactory);

	if(format == DXGI_FORMAT_UNKNOWN){
		format = DXGI_FORMAT_R8G8B8A8_UNORM;
        bpp = 0;
	}

	// Allocate temporary memory for image
    size_t rowPitch = ( width * bpp + 7 ) / 8;
    size_t imageSize = rowPitch * height;

	std::unique_ptr<uint8_t[]> temp( new uint8_t[ imageSize ] );

	frame->CopyPixels( 0, static_cast<UINT>( rowPitch ), static_cast<UINT>( imageSize ), temp.get() );  



	// Create texture
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
	desc.Format = format;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = temp.get();
    initData.SysMemPitch = static_cast<UINT>( rowPitch );
    initData.SysMemSlicePitch = static_cast<UINT>( imageSize );

    ID3D11Texture2D* tex = nullptr;
    g_pd3dDevice->CreateTexture2D( &desc, &initData, &tex );

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
   memset( &SRVDesc, 0, sizeof( SRVDesc ) );
   SRVDesc.Format = format;
   SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
   SRVDesc.Texture2D.MipLevels = 1;

   g_pd3dDevice->CreateShaderResourceView( tex, &SRVDesc, &mesh->textureResourceView );

	if(frame) frame->Release();

	return S_OK;
}

HRESULT Game::LoadWave(char* filename, Sound *sound){
	//http://www.rastertek.com/dx11tut14.html
	struct WaveHeaderType
	{
		char chunkId[4];
		unsigned long chunkSize;
		char format[4];
		char subChunkId[4];
		unsigned long subChunkSize;
		unsigned short audioFormat;
		unsigned short numChannels;
		unsigned long sampleRate;
		unsigned long bytesPerSecond;
		unsigned short blockAlign;
		unsigned short bitsPerSample;
		char dataChunkId[4];
		unsigned long dataSize;
	};

	int error;
	FILE* filePtr;
	unsigned int count;
	WaveHeaderType waveFileHeader;
	WAVEFORMATEX waveFormat;
	DSBUFFERDESC bufferDesc;
	HRESULT result;
	IDirectSoundBuffer* tempBuffer;
	unsigned char* waveData;
	unsigned char *bufferPtr;
	unsigned long bufferSize;

	fopen_s(&filePtr, filename, "rb");
	count = fread(&waveFileHeader, sizeof(waveFileHeader), 1, filePtr);

	if(count != 1)
	{
		return 0;
	}

	// Check that the chunk ID is the RIFF format.
	if((waveFileHeader.chunkId[0] != 'R') || (waveFileHeader.chunkId[1] != 'I') || 
	   (waveFileHeader.chunkId[2] != 'F') || (waveFileHeader.chunkId[3] != 'F'))
	{
		return false;
	}
 
	// Check that the file format is the WAVE format.
	if((waveFileHeader.format[0] != 'W') || (waveFileHeader.format[1] != 'A') ||
	   (waveFileHeader.format[2] != 'V') || (waveFileHeader.format[3] != 'E'))
	{
		return false;
	}
 
	// Check that the sub chunk ID is the fmt format.
	if((waveFileHeader.subChunkId[0] != 'f') || (waveFileHeader.subChunkId[1] != 'm') ||
	   (waveFileHeader.subChunkId[2] != 't') || (waveFileHeader.subChunkId[3] != ' '))
	{
		return false;
	}
 

	// Check for the data chunk header.
	if((waveFileHeader.dataChunkId[0] != 'd') || (waveFileHeader.dataChunkId[1] != 'a') ||
	   (waveFileHeader.dataChunkId[2] != 't') || (waveFileHeader.dataChunkId[3] != 'a'))
	{
		return false;
	}

	// Set the wave format of secondary buffer that this wave file will be loaded onto.
	waveFormat.wFormatTag = waveFileHeader.audioFormat;
	waveFormat.nSamplesPerSec = waveFileHeader.sampleRate;
	waveFormat.wBitsPerSample = waveFileHeader.bitsPerSample;
	waveFormat.nChannels = waveFileHeader.numChannels;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;
 
	// Set the buffer description of the secondary sound buffer that the wave file will be loaded onto.
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME;
	bufferDesc.dwBufferBytes = waveFileHeader.dataSize;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = &waveFormat;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	result = lpds->CreateSoundBuffer(&bufferDesc, &tempBuffer, NULL);
	result = tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&sound->soundBuffer);

	tempBuffer->Release();
	tempBuffer = 0;

	fseek(filePtr, sizeof(WaveHeaderType), SEEK_SET);

	waveData = new unsigned char[waveFileHeader.dataSize];

	count = fread(waveData, 1, waveFileHeader.dataSize, filePtr);

	error = fclose(filePtr);

	result = (sound->soundBuffer)->Lock(0, waveFileHeader.dataSize, (void**)&bufferPtr, (DWORD*)&bufferSize, NULL, 0, 0);

	memcpy(bufferPtr, waveData, waveFileHeader.dataSize);

	result = (sound->soundBuffer)->Unlock((void*)bufferPtr, bufferSize, NULL, 0);

	// Release the wave data since it was copied into the secondary buffer.
	delete [] waveData;
	waveData = 0;
 
	sound->volume = DSBVOLUME_MAX;

	return S_OK;
}

void Game::SoundPlay(Sound *sound, bool loop){
	sound->soundBuffer->SetCurrentPosition(0);
	sound->soundBuffer->SetVolume(sound->volume);
	if(loop)
		sound->soundBuffer->Play(0,0,DSBPLAY_LOOPING);
	else 
		sound->soundBuffer->Play(0,0,0);
}

LRESULT CALLBACK Game::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
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

HRESULT Game::HandleInputs(MSG msg){


	if(msg.message == WM_KEYUP){
		OutputDebugStringW(L"\n KEYUP");
		switch(msg.wParam){
			case 0x57:
				keyboard.w = 0;
				break;
			case 0x41:
				keyboard.a = 0;
				break;
			case 0x53:
				keyboard.s = 0;
				break;
			case 0x44:
				keyboard.d = 0;
				break;
			case 0x20:
				keyboard.space = 0;
				break;
		}

	}

	if(msg.message == WM_KEYDOWN){
		OutputDebugStringW(L"\n KEYDOWN");
		switch(msg.wParam){
			case 0x57:
				keyboard.w = 1;
				break;
			case 0x41:
				keyboard.a = 1;
				break;
			case 0x53:
				keyboard.s = 1;
				break;
			case 0x44:
				keyboard.d = 1;
				break;
			case 0x20:
				keyboard.space = 1;
				break;

		}
	}

	if(msg.message == WM_CHAR){
		OutputDebugStringW(L"\n KEY");
		switch(msg.wParam){
			case 0x57:
				keyboard.w = 1;
				break;
			case 0x41:
				keyboard.a = 1;
				break;
			case 0x53:
				keyboard.s = 1;
				break;
			case 0x44:
				keyboard.d = 1;
				break;
			case 0x20:
				keyboard.space = 1;
				break;
		}
	}

	return S_OK;

}

void SGEFramework::DestroyMesh(Mesh *mesh){
	if(mesh->indices) delete mesh->indices;
	if(mesh->vertices) delete mesh->vertices;
	if(mesh->textureResourceView) delete mesh->textureResourceView;

}



//Load a shader from a file
HRESULT CompileShader( _In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob )
{
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
    HRESULT hr = D3DCompileFromFile( srcFile, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                     entryPoint, profile,
                                     flags, 0, &shaderBlob, &errorBlob );
    if ( FAILED(hr) )
    {
        if ( errorBlob )
        {
            OutputDebugStringA( (char*)errorBlob->GetBufferPointer() );
            errorBlob->Release();
        }

        if ( shaderBlob )
           shaderBlob->Release();

        return hr;
    }    

    *blob = shaderBlob;

    return hr;
}


size_t _WICBitsPerPixel( REFGUID targetGuid , IWICImagingFactory *factory)
{

    IWICComponentInfo *cinfo;
    IWICPixelFormatInfo *pfinfo;
	factory->CreateComponentInfo( targetGuid, &cinfo );

    cinfo->QueryInterface( __uuidof(IWICPixelFormatInfo), reinterpret_cast<void**>( &pfinfo )  ) ;

    UINT bpp;
	pfinfo->GetBitsPerPixel( &bpp ) ;


    return bpp;
}



DXGI_FORMAT _WICToDXGI( const GUID& guid )
{
	 struct WICTranslate
	{
		GUID                wic;
		DXGI_FORMAT         format;
	};

	WICTranslate g_WICFormats[] = 
	{
		{ GUID_WICPixelFormat128bppRGBAFloat,       DXGI_FORMAT_R32G32B32A32_FLOAT },

		{ GUID_WICPixelFormat64bppRGBAHalf,         DXGI_FORMAT_R16G16B16A16_FLOAT },
		{ GUID_WICPixelFormat64bppRGBA,             DXGI_FORMAT_R16G16B16A16_UNORM },

		{ GUID_WICPixelFormat32bppRGBA,             DXGI_FORMAT_R8G8B8A8_UNORM },
		{ GUID_WICPixelFormat32bppBGRA,             DXGI_FORMAT_B8G8R8A8_UNORM }, // DXGI 1.1
		{ GUID_WICPixelFormat32bppBGR,              DXGI_FORMAT_B8G8R8X8_UNORM }, // DXGI 1.1

		{ GUID_WICPixelFormat32bppRGBA1010102XR,    DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM }, // DXGI 1.1
		{ GUID_WICPixelFormat32bppRGBA1010102,      DXGI_FORMAT_R10G10B10A2_UNORM },
		{ GUID_WICPixelFormat32bppRGBE,             DXGI_FORMAT_R9G9B9E5_SHAREDEXP },

	#ifdef DXGI_1_2_FORMATS

		{ GUID_WICPixelFormat16bppBGRA5551,         DXGI_FORMAT_B5G5R5A1_UNORM },
		{ GUID_WICPixelFormat16bppBGR565,           DXGI_FORMAT_B5G6R5_UNORM },

	#endif // DXGI_1_2_FORMATS

		{ GUID_WICPixelFormat32bppGrayFloat,        DXGI_FORMAT_R32_FLOAT },
		{ GUID_WICPixelFormat16bppGrayHalf,         DXGI_FORMAT_R16_FLOAT },
		{ GUID_WICPixelFormat16bppGray,             DXGI_FORMAT_R16_UNORM },
		{ GUID_WICPixelFormat8bppGray,              DXGI_FORMAT_R8_UNORM },

		{ GUID_WICPixelFormat8bppAlpha,             DXGI_FORMAT_A8_UNORM },

	#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
		{ GUID_WICPixelFormat96bppRGBFloat,         DXGI_FORMAT_R32G32B32_FLOAT },
	#endif
	};


    for( size_t i=0; i < _countof(g_WICFormats); ++i )
    {
        if ( memcmp( &g_WICFormats[i].wic, &guid, sizeof(GUID) ) == 0 )
            return g_WICFormats[i].format;
    }

    return DXGI_FORMAT_UNKNOWN;
}

