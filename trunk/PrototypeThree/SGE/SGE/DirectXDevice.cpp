#include "stdafx.h"
#include "SGD3D.h"

using namespace SGD3D;

DirectXDevice::DirectXDevice(){
	driverType = D3D_DRIVER_TYPE_HARDWARE;
	featureLevel = D3D_FEATURE_LEVEL_11_0;
	d3dDevice = NULL;
	immediateContext = NULL;
	swapChain = NULL;
	renderTargetView = NULL;
	depthStencil = NULL;
	depthStencilState = NULL;
	disabledDepthStencilState = NULL;
	depthStencilView = NULL;
	vertexBuffer = NULL;
	indexBuffer = NULL;
	constantBuffer = NULL;
	samplerLinear = NULL;
	imagingFactory = NULL;
	bitmapDecoder = NULL;
	inputLayout = NULL;
}

DirectXDevice::~DirectXDevice(){
	CleanUp();
}

HRESULT DirectXDevice::InitializeDevice(SGE::Framework::GameDescription *gameDescription, HWND hWnd){
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
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	//create swap chain/ context and view
	HRESULT result = D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, 0, 0,D3D11_SDK_VERSION, &sd, &swapChain
												,&d3dDevice, &featureLevel, &immediateContext);
	if (FAILED(result)) return result;

	if(gameDescription)
		swapChain->SetFullscreenState(gameDescription->fullscreen, NULL);  

	//Add a render target view to the swap chain
	ID3D11Texture2D *BackBuffer = NULL;
	result = swapChain->GetBuffer(0, _uuidof(ID3D11Texture2D), (LPVOID*) &BackBuffer);
	if(FAILED(result)) return result;
	result = d3dDevice->CreateRenderTargetView(BackBuffer,0,&renderTargetView);
	if(FAILED(result)) return result;

	BackBuffer->Release();

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

	//Depth Stencil State
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	result = d3dDevice->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);
	if(FAILED(result))
	{
		return false;
	}

	depthStencilDesc.DepthEnable = false;

	result = d3dDevice->CreateDepthStencilState(&depthStencilDesc, &disabledDepthStencilState);
	if(FAILED(result))
	{
		return false;
	}

	immediateContext->OMSetDepthStencilState(depthStencilState,1);

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
	vp.Width = (float)width;
	vp.Height = (float)height;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;

	//Bind an array of viewports to the rasterizer stage of the pipeline.
	immediateContext->RSSetViewports(1,&vp);

	//Create constant buffer(s)
	D3D11_BUFFER_DESC bd;
	SecureZeroMemory(&bd, sizeof(bd));
	bd.ByteWidth = sizeof(CB_VS_PER_OBJECT);
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;

	result = d3dDevice->CreateBuffer(&bd, NULL, &constantBuffer);
	if(FAILED(result)) return result;

	D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory( &sampDesc, sizeof(sampDesc) );
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    result = d3dDevice->CreateSamplerState( &sampDesc, &samplerLinear );
    if( FAILED( result ) ) return result;

	float ClearColor[4] = { 0, 0, 0, 0 }; //red,green,blue,alpha
    immediateContext->ClearRenderTargetView( renderTargetView, ClearColor );
	immediateContext->ClearDepthStencilView( depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );
	result = swapChain->Present( 1, 0 );

	XMStoreFloat4x4(&projection,DirectX::XMMatrixPerspectiveFovLH( DirectX::XM_PIDIV2, width / (FLOAT)height, 0.01f, 1000.0f ));
	//XMStoreFloat4x4(&projection,(DirectX::XMMatrixOrthographicLH((float)width, (float)height, 0.01f, 1000.0f)));

	return S_OK;
}

HRESULT DirectXDevice::ProcessContent(){


	if(meshes.size() > 0){
		Vertex *vertices;
		WORD * indices;

		int numOfVertices = 0;
		int numOfIndices = 0;

		int totalVertices = 0;
		int totalIndices = 0;

		for(UINT i=0; i < meshes.size(); i++){
			totalVertices += meshes[i]->numOfVertices;
			totalIndices += meshes[i]->numOfIndices;
		}

		vertices = new Vertex[totalVertices];
		indices = new WORD[totalIndices];


		for(UINT i=0; i < meshes.size(); i++){

			for(UINT j = 0; j < meshes[i]->numOfVertices; j++){
				vertices[j + numOfVertices] = meshes[i]->vertices[j];
			}
			meshes[i]->startVertex = numOfVertices;
			numOfVertices += meshes[i]->numOfVertices;
		
			for(UINT j = 0; j < meshes[i]->numOfIndices; j++){
				indices[j + numOfIndices] = meshes[i]->indices[j];
			}

			meshes[i]->startIndex = numOfIndices;
			numOfIndices += meshes[i]->numOfIndices;
		}


		HRESULT result;

		//Create a vertex buffer
		D3D11_BUFFER_DESC bd;
		SecureZeroMemory(&bd, sizeof(bd));
		bd.ByteWidth = sizeof(Vertex) * numOfVertices;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;


		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = vertices;
		result =  d3dDevice->CreateBuffer(&bd, &InitData, &vertexBuffer);
		if(FAILED(result)) return result;

		//Set vertex buffer
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		immediateContext->IASetVertexBuffers(0,1, &vertexBuffer, &stride, &offset);


		//Create an index buffer
		SecureZeroMemory(&bd, sizeof(bd));
		bd.ByteWidth = sizeof(WORD) * numOfIndices;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;

		InitData.pSysMem = indices;
		result = d3dDevice->CreateBuffer( &bd, &InitData, &indexBuffer );
		if(FAILED(result)) return result;

		// Set index buffer
		immediateContext->IASetIndexBuffer( indexBuffer, DXGI_FORMAT_R16_UINT, 0 );

		if(vertices) delete vertices;
		if(indices) delete indices;
	}

	// Set primitive topology
	immediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	

	if(vertexShaders.size() > 0)
		immediateContext->VSSetShader( vertexShaders[0], NULL, 0 );
	else 
		return E_FAIL;
	if(pixelShaders.size() > 0)
		immediateContext->PSSetShader( pixelShaders[0], NULL, 0 );
	else 
		return E_FAIL;
	if(samplerLinear)
		immediateContext->PSSetSamplers(0,1,&samplerLinear);
	else 
		return E_FAIL;


	return S_OK;
}

HRESULT DirectXDevice::PositionCamera(SGE::Vector4 position, SGE::Vector4 at) {
	using namespace DirectX;
	XMVECTOR Eye = XMVectorSet( (float)position.x, (float)position.y, (float)position.z, 0.0f );
	XMVECTOR At = XMVectorSet((float)at.x, (float)at.y, (float)at.z, 0.0f );
	XMVECTOR Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	XMStoreFloat4x4(&camera,XMMatrixLookAtLH( Eye, At, Up ));
	
	return S_OK;
}

HRESULT DirectXDevice::Draw(){
	HRESULT result = swapChain->Present( 1, 0 );
	if (FAILED(result)) return result;

	return S_OK;
}

HRESULT DirectXDevice::DrawMesh(SGE::Graphics::Mesh* mesh, SGE::Graphics::Texture* texture, SGE::Vector4 position, SGE::Vector4 scale, SGE::Vector4 rotation){
	using namespace DirectX;

	XMMATRIX world = XMMatrixScaling(scale.x, scale.y, scale.z) * XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z) * 
		XMMatrixTranslation(position.x, position.y, position.z);

	if(mesh->index != -1 && mesh->index < meshes.size()){
		CB_VS_PER_OBJECT cb;
		cb.world = XMMatrixTranspose(world);
		cb.worldViewProj = XMMatrixTranspose(world * XMLoadFloat4x4(&camera) * XMLoadFloat4x4(&projection));
		immediateContext->UpdateSubresource(constantBuffer,0,NULL,&cb,0,0);
		immediateContext->VSSetConstantBuffers( 0, 1, &constantBuffer );

		if(texture && texture->index != -1 && texture->index < textures.size())
			immediateContext->PSSetShaderResources( 0, 1, &textures[texture->index] );

		immediateContext->DrawIndexed(meshes[mesh->index]->numOfIndices,meshes[mesh->index]->startIndex,meshes[mesh->index]->startVertex);
	}
	else 
		return E_FAIL;

	return S_OK;
}

HRESULT DirectXDevice::DrawGameObject(SGE::Framework::GameObject* gameObject){
	using namespace DirectX;
	
	if(!gameObject)
		return E_FAIL;

	XMMATRIX world = XMMatrixScaling(gameObject->Scale().x, gameObject->Scale().y, gameObject->Scale().z) 
		* XMMatrixRotationRollPitchYaw(gameObject->Rotation().x, gameObject->Rotation().y, gameObject->Rotation().z) 
		* XMMatrixTranslation(gameObject->Position().x, gameObject->Position().y, gameObject->Position().z);

	if(gameObject->Mesh().index != -1 && gameObject->Mesh().index < meshes.size()){
		CB_VS_PER_OBJECT cb;
		cb.world = XMMatrixTranspose(world);
		cb.worldViewProj = XMMatrixTranspose(world * XMLoadFloat4x4(&camera) * XMLoadFloat4x4(&projection));
		immediateContext->UpdateSubresource(constantBuffer,0,NULL,&cb,0,0);
		immediateContext->VSSetConstantBuffers( 0, 1, &constantBuffer );

		if(gameObject->Texture().index != -1 && gameObject->Texture().index < textures.size())
			immediateContext->PSSetShaderResources( 0, 1, &textures[gameObject->Texture().index] );

		immediateContext->DrawIndexed(meshes[gameObject->Mesh().index]->numOfIndices,meshes[gameObject->Mesh().index]->startIndex,meshes[gameObject->Mesh().index]->startVertex);

		SGE::Vector4 direction = gameObject->InitialDirection();
		XMVECTOR currentRotation = XMVectorSet(direction.x, direction.y, direction.z, direction.w);
		currentRotation =	XMVector3TransformCoord(currentRotation, XMMatrixRotationRollPitchYaw(gameObject->Rotation().x, gameObject->Rotation().y, gameObject->Rotation().z) );
		direction = SGE::Vector4(XMVectorGetX(currentRotation),XMVectorGetY(currentRotation),XMVectorGetZ(currentRotation),XMVectorGetW(currentRotation));
		gameObject->CurrentDirection(direction);
	}
	else 
		return E_FAIL;

	return S_OK;

}

HRESULT DirectXDevice::Clear(){
	float ClearColor[4] = { 0.1f, 0.1f, 0.3f, 1.0f }; //red,green,blue,alpha
    immediateContext->ClearRenderTargetView( renderTargetView, ClearColor );
	immediateContext->ClearDepthStencilView( depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );
	return S_OK;
}

HRESULT DirectXDevice::Clear(float clearColor[4]){
    immediateContext->ClearRenderTargetView( renderTargetView, clearColor );
	immediateContext->ClearDepthStencilView( depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );
	return S_OK;
}

HRESULT DirectXDevice::LoadObj(wchar_t * filename, SGE::Graphics::Mesh* mesh){
	using namespace std;
	using namespace DirectX;

	mesh->index = -1;

	ifstream fileStream;
	string line;

	SGD3D::Mesh *newMesh = new SGD3D::Mesh;

	newMesh->numOfIndices =0;
	newMesh->numOfVertices =0;
	newMesh->startIndex =0;
	newMesh->startVertex =0;

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
	vector<Vertex> uniqueVertices (0);
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
	Vertex tempVertex;
	int times;


	//Open the file
	fileStream.open(filename);

	if(fileStream.is_open()){
		while(getline(fileStream, line)){
			//If a group is found
			if(line.compare(0,2,"g ") == 0){
				numOfSubests++;
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
					for(UINT j =0; j < numOfFaces; j++){
						if(faces[j].x == tempFloat4.x && faces[j].y == tempFloat4.y && faces[j].z == tempFloat4.z){
							//If found store its W value
							found = true;
							position = (int)faces[j].w;	
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
							
						faces[numOfFaces - 1].w = (float)position; //Store the index of where it was found in the .w 
					}
					//If not found create a new unique vertex 
					else if (!found){
						tempVertex.Pos = vertices[(UINT)faces[numOfFaces - 1].x];

						if(faces[numOfFaces-1].y != -1){
							tempVertex.TexUV = 
								XMFLOAT2( uvs[(UINT)faces[numOfFaces - 1].y].x , 1 - uvs[(UINT)faces[numOfFaces - 1].y].y);
						}
						else 
							tempVertex.TexUV = XMFLOAT2(0,0);

						if(faces[numOfFaces-1].z != -1){
							tempVertex.Normal = normals[(UINT)faces[numOfFaces - 1].z];
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

						faces[numOfFaces-1].w = (float)numOfUVertices -1;	
					}
				}
			}
		} //End while readline
	}//End if open
	else{
		return E_FAIL;
	}


	newMesh->numOfVertices = numOfUVertices;
	newMesh->numOfIndices = numOfIndices;

	//Create an array to store the vertices and indices
	newMesh->vertices = new Vertex[newMesh->numOfVertices];
	newMesh->indices = new WORD[newMesh->numOfIndices];


	//Copy the values into the mesh
	for(UINT i= 0; i < numOfUVertices; i++){
		newMesh->vertices[i] = uniqueVertices[i];
	}

	for(UINT i= 0; i < numOfIndices; i++){
		newMesh->indices[i]= indices[i];
	}

	//Close the file and return
	fileStream.close();

	meshes.push_back(newMesh);
	mesh->index = meshes.size() - 1;

	return S_OK;
}

HRESULT DirectXDevice::LoadTexture(wchar_t* filename, SGE::Graphics::Texture* texture){

	texture->index = -1;
	HRESULT result;

	result = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), (LPVOID*)&imagingFactory);
	if (FAILED(result)) return result;
	result = imagingFactory->CreateDecoderFromFilename(filename, 0, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &bitmapDecoder);
	if (FAILED(result)) return result;

	IWICBitmapFrameDecode* frame;
	result = bitmapDecoder->GetFrame(0, &frame);
	if (FAILED(result)) return result;
	WICPixelFormatGUID pixelFormat;
	result = frame->GetPixelFormat(&pixelFormat);
	if (FAILED(result)) return result;

	DXGI_FORMAT format = _WICToDXGI( pixelFormat);

	WICPixelFormatGUID convertGUID;
    memcpy( &convertGUID, &pixelFormat, sizeof(WICPixelFormatGUID) );

	UINT width, height;

	result = frame->GetSize(&width, &height);
	if (FAILED(result)) return result;

	size_t bpp = 0;
	
    if ( format == DXGI_FORMAT_UNKNOWN )
	{
		struct WICConvert
		{
			GUID        source;
			GUID        target;
		};


		static WICConvert WICConvert[] = 
		{
			// Note target GUID in this conversion table must be one of those directly supported formats (above).

			{ GUID_WICPixelFormatBlackWhite,            GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM

			{ GUID_WICPixelFormat1bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
			{ GUID_WICPixelFormat2bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
			{ GUID_WICPixelFormat4bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
			{ GUID_WICPixelFormat8bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 

			{ GUID_WICPixelFormat2bppGray,              GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM 
			{ GUID_WICPixelFormat4bppGray,              GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM 

			{ GUID_WICPixelFormat16bppGrayFixedPoint,   GUID_WICPixelFormat16bppGrayHalf }, // DXGI_FORMAT_R16_FLOAT 
			{ GUID_WICPixelFormat32bppGrayFixedPoint,   GUID_WICPixelFormat32bppGrayFloat }, // DXGI_FORMAT_R32_FLOAT 

		#ifdef DXGI_1_2_FORMATS

			{ GUID_WICPixelFormat16bppBGR555,           GUID_WICPixelFormat16bppBGRA5551 }, // DXGI_FORMAT_B5G5R5A1_UNORM

		#else

			{ GUID_WICPixelFormat16bppBGR555,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
			{ GUID_WICPixelFormat16bppBGRA5551,         GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
			{ GUID_WICPixelFormat16bppBGR565,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM

		#endif // DXGI_1_2_FORMATS

			{ GUID_WICPixelFormat32bppBGR101010,        GUID_WICPixelFormat32bppRGBA1010102 }, // DXGI_FORMAT_R10G10B10A2_UNORM

			{ GUID_WICPixelFormat24bppBGR,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
			{ GUID_WICPixelFormat24bppRGB,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
			{ GUID_WICPixelFormat32bppPBGRA,            GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
			{ GUID_WICPixelFormat32bppPRGBA,            GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 

			{ GUID_WICPixelFormat48bppRGB,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
			{ GUID_WICPixelFormat48bppBGR,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
			{ GUID_WICPixelFormat64bppBGRA,             GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
			{ GUID_WICPixelFormat64bppPRGBA,            GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
			{ GUID_WICPixelFormat64bppPBGRA,            GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM

			{ GUID_WICPixelFormat48bppRGBFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
			{ GUID_WICPixelFormat48bppBGRFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
			{ GUID_WICPixelFormat64bppRGBAFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
			{ GUID_WICPixelFormat64bppBGRAFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
			{ GUID_WICPixelFormat64bppRGBFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
			{ GUID_WICPixelFormat64bppRGBHalf,          GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
			{ GUID_WICPixelFormat48bppRGBHalf,          GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 

			{ GUID_WICPixelFormat96bppRGBFixedPoint,    GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 
			{ GUID_WICPixelFormat128bppPRGBAFloat,      GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 
			{ GUID_WICPixelFormat128bppRGBFloat,        GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 
			{ GUID_WICPixelFormat128bppRGBAFixedPoint,  GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 
			{ GUID_WICPixelFormat128bppRGBFixedPoint,   GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 

			{ GUID_WICPixelFormat32bppCMYK,             GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
			{ GUID_WICPixelFormat64bppCMYK,             GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
			{ GUID_WICPixelFormat40bppCMYKAlpha,        GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
			{ GUID_WICPixelFormat80bppCMYKAlpha,        GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM

		#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
			{ GUID_WICPixelFormat32bppRGB,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
			{ GUID_WICPixelFormat64bppRGB,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
			{ GUID_WICPixelFormat64bppPRGBAHalf,        GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
		#endif

			// We don't support n-channel formats
		};

        for( size_t i=0; i < _countof(WICConvert); ++i )
        {
            if ( memcmp( &WICConvert[i].source, &pixelFormat, sizeof(WICPixelFormatGUID) ) == 0 )
            {
                memcpy( &convertGUID, &WICConvert[i].target, sizeof(WICPixelFormatGUID) );

                format = _WICToDXGI( WICConvert[i].target );
                assert( format != DXGI_FORMAT_UNKNOWN );
                bpp = _WICBitsPerPixel( convertGUID, imagingFactory );
                break;
            }
        }

        if ( format == DXGI_FORMAT_UNKNOWN )
            return HRESULT_FROM_WIN32( ERROR_NOT_SUPPORTED );
    }
    else
    {
        bpp = _WICBitsPerPixel( pixelFormat, imagingFactory );
    }


	// Allocate temporary memory for image
    size_t rowPitch = ( width * bpp + 7 ) / 8;
    size_t imageSize = rowPitch * height;

	std::unique_ptr<uint8_t[]> temp( new uint8_t[ imageSize ] );

	if ( memcmp( &convertGUID, &pixelFormat, sizeof(GUID) ) == 0)
    {
        // No format conversion or resize needed
        HRESULT hr = frame->CopyPixels( 0, static_cast<UINT>( rowPitch ), static_cast<UINT>( imageSize ), temp.get() );  
        if ( FAILED(hr) )
            return hr;
    }
	else 
	{
		// Format conversion but no resize
        IWICImagingFactory* pWIC = imagingFactory;
        if ( !pWIC )
            return E_NOINTERFACE;

        IWICFormatConverter * FC;
        HRESULT hr = pWIC->CreateFormatConverter( &FC );
        if ( FAILED(hr) )
            return hr;

        hr = FC->Initialize( frame, convertGUID, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom );
        if ( FAILED(hr) )
            return hr;

        hr = FC->CopyPixels( 0, static_cast<UINT>( rowPitch ), static_cast<UINT>( imageSize ), temp.get() );  
        if ( FAILED(hr) )
            return hr;

	}


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
    result = d3dDevice->CreateTexture2D( &desc, &initData, &tex );
	if (FAILED(result)) return result;

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
   memset( &SRVDesc, 0, sizeof( SRVDesc ) );
   SRVDesc.Format = format;
   SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
   SRVDesc.Texture2D.MipLevels = 1;

   ID3D11ShaderResourceView * newTexture;

   result = d3dDevice->CreateShaderResourceView( tex, &SRVDesc, &newTexture);
   if (FAILED(result)) return result;

   textures.push_back(newTexture);
   texture->index = textures.size() - 1;

	if(frame) frame->Release();
	if(imagingFactory) imagingFactory->Release();
	if(bitmapDecoder) bitmapDecoder->Release();

	return S_OK;
}

HRESULT DirectXDevice::LoadVShader(wchar_t* filename, char* entryPoint, SGE::Graphics::VertexShader *shader){
	shader->index = -1;

	//Compile the vertex shader
	ID3DBlob *VSBlob = NULL;
	HRESULT result = CompileShader( filename, entryPoint, "vs_4_0", &VSBlob );
	if(FAILED(result)) return result;

	ID3D11VertexShader *vertexShader;
	result = d3dDevice->CreateVertexShader( VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), NULL, &vertexShader );
	if(FAILED(result)) return result;

		//Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	 {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
	UINT numElements = ARRAYSIZE( layout );

	//Create the input layout
	result = d3dDevice->CreateInputLayout( layout, numElements, VSBlob->GetBufferPointer(),
                                          VSBlob->GetBufferSize(), &inputLayout );
	VSBlob->Release();
	if(FAILED(result)) return result;

	//Set the input layout
	immediateContext->IASetInputLayout(inputLayout);

	vertexShaders.push_back(vertexShader);
	shader->index = vertexShaders.size() - 1;

	return S_OK;
}

HRESULT DirectXDevice::LoadPShader(wchar_t* filename, char* entryPoint, SGE::Graphics::PixelShader *shader){
	shader->index =-1;

	//Compile the vertex shader
	ID3DBlob *PSBlob = NULL;
	HRESULT result = CompileShader( filename, entryPoint, "ps_4_0", &PSBlob );
	if(FAILED(result)) return result;

	ID3D11PixelShader *pixelShader;
	result = d3dDevice->CreatePixelShader( PSBlob->GetBufferPointer(), PSBlob->GetBufferSize(), NULL, &pixelShader );
	if(FAILED(result)) return result;

	PSBlob->Release();

	pixelShaders.push_back(pixelShader);
	shader->index = pixelShaders.size() - 1;

	return S_OK;
}

HRESULT DirectXDevice::SetVShader(SGE::Graphics::VertexShader *shader){
	if(shader->index != -1 && shader->index < vertexShaders.size())
		immediateContext->VSSetShader( vertexShaders[shader->index], NULL, 0 );
	else 
		return E_FAIL;

	return S_OK;

}

HRESULT DirectXDevice::SetPShader(SGE::Graphics::PixelShader *shader){
	if(shader->index != -1 && shader->index < pixelShaders.size())
		immediateContext->PSSetShader( pixelShaders[shader->index], NULL, 0 );
	else 
		return E_FAIL;

	return S_OK;
}

HRESULT DirectXDevice::CreateConstantBuffer(size_t byteWidth, SGE::Graphics::ConstantBuffer *buffer){

	ID3D11Buffer* tempConstantBuffer;
	buffer->index = -1;

	D3D11_BUFFER_DESC bd;
	SecureZeroMemory(&bd, sizeof(bd));
	bd.ByteWidth = byteWidth;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;

	HRESULT result = d3dDevice->CreateBuffer(&bd, NULL, &tempConstantBuffer);
	if(FAILED(result)) return result;

	constantBuffers.push_back(tempConstantBuffer);
	buffer->index = constantBuffers.size() - 1;

	return S_OK;
}

HRESULT DirectXDevice::UpdateConstantBuffer(SGE::Graphics::CB * data, SGE::Graphics::ConstantBuffer *buffer){

	if(buffer->index == -1 || buffer->index >= constantBuffers.size())
		return E_FAIL;

	immediateContext->UpdateSubresource(constantBuffers[buffer->index],0,NULL,data,0,0);
	immediateContext->VSSetConstantBuffers( buffer->index + 1, 1, &constantBuffers[buffer->index] );

	return S_OK;
}

HRESULT DirectXDevice::CleanUp(){
	swapChain->SetFullscreenState(FALSE, NULL);  

	if(d3dDevice) d3dDevice->Release();
	if(immediateContext) immediateContext->Release();
	if(swapChain) swapChain->Release();
	if(renderTargetView) renderTargetView->Release();
	if(depthStencil) depthStencil->Release();
	if(depthStencilState) depthStencilState->Release();
	if(disabledDepthStencilState) disabledDepthStencilState->Release();
	if(depthStencilView) depthStencilView->Release();
	if(vertexBuffer) vertexBuffer->Release();
	if(indexBuffer) indexBuffer->Release();
	if(constantBuffer) constantBuffer->Release();
	if(samplerLinear) samplerLinear->Release();

	for(UINT i =0; i < vertexShaders.size(); i++){
		if(vertexShaders[i]) vertexShaders[i]->Release();
	}
	for(UINT i =0; i < pixelShaders.size(); i++){
		if(pixelShaders[i]) pixelShaders[i]->Release();
	}
	for(UINT i =0; i < constantBuffers.size(); i++){
		if(constantBuffers[i]) constantBuffers[i]->Release();
	}
	for(UINT i =0; i < textures.size(); i++){
		if(textures[i]) textures[i]->Release();
	}
	for(UINT i =0; i < meshes.size(); i++){
		if(meshes[i]) meshes[i]->Destroy();
		if(meshes[i]) delete meshes[i];
	}

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
            errorBlob->Release();
        }

        if ( shaderBlob )
           shaderBlob->Release();

        return result;
    }    

    *blob = shaderBlob;

    return result;

}

DXGI_FORMAT DirectXDevice::_WICToDXGI( const GUID& guid ){
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

size_t DirectXDevice::_WICBitsPerPixel( REFGUID targetGuid , IWICImagingFactory *factory){

    IWICComponentInfo *cinfo;
    IWICPixelFormatInfo *pfinfo;
	factory->CreateComponentInfo( targetGuid, &cinfo );

    cinfo->QueryInterface( __uuidof(IWICPixelFormatInfo), reinterpret_cast<void**>( &pfinfo )  ) ;

    UINT bpp;
	pfinfo->GetBitsPerPixel( &bpp ) ;


    return bpp;
}
