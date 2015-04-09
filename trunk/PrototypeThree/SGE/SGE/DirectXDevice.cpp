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
	depthStencilView = NULL;
}

DirectXDevice::~DirectXDevice(){
	CleanUp();
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

	if (FAILED(result)) return result;

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


	return S_OK;
}

HRESULT DirectXDevice::Draw(){
	HRESULT result = swapChain->Present( 1, 0 );
	if (FAILED(result)) return result;

	return S_OK;
}

HRESULT DirectXDevice::DrawMesh(SGE::Graphics::Mesh mesh){

	return S_OK;
}

HRESULT DirectXDevice::DrawGameObject(SGE::Framework::GameObject){

	return S_OK;
}

HRESULT DirectXDevice::Clear(){
	float ClearColor[4] = { 0.1f, 0.1f, 0.3f, 1.0f }; //red,green,blue,alpha
    immediateContext->ClearRenderTargetView( renderTargetView, ClearColor );
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

	VSBlob->Release();

	vertexShaders.push_back(vertexShader);
	shader->index = vertexShaders.size() - 1;

	return S_OK;
}

HRESULT DirectXDevice::LoadPShader(wchar_t* filename, char* entryPoint, SGE::Graphics::PixelShader *shader){
	shader->index =-1;

	//Compile the vertex shader
	ID3DBlob *PSBlob = NULL;
	HRESULT result = CompileShader( filename, entryPoint, "vs_4_0", &PSBlob );
	if(FAILED(result)) return result;

	ID3D11VertexShader *vertexShader;
	result = d3dDevice->CreateVertexShader( PSBlob->GetBufferPointer(), PSBlob->GetBufferSize(), NULL, &vertexShader );
	if(FAILED(result)) return result;

	PSBlob->Release();

	vertexShaders.push_back(vertexShader);
	shader->index = vertexShaders.size() - 1;

	return S_OK;
}

HRESULT DirectXDevice::CleanUp(){
	if(d3dDevice) d3dDevice->Release();
	d3dDevice = nullptr;
	if(immediateContext) immediateContext->Release();
	immediateContext = nullptr;
	if(swapChain) swapChain->Release();
	swapChain = nullptr;
	if(renderTargetView) renderTargetView->Release();
	renderTargetView = nullptr;
	if(depthStencil) depthStencil->Release();
	depthStencil = nullptr;
	if(depthStencilView) depthStencilView->Release();
	depthStencilView = nullptr;

	for(UINT i =0; i < vertexShaders.size(); i++){
		if(vertexShaders[i]) vertexShaders[i]->Release();
	}
	for(UINT i =0; i < pixelShaders.size(); i++){
		if(pixelShaders[i]) pixelShaders[i]->Release();
	}
	for(UINT i =0; i < textures.size(); i++){
		if(textures[i]) textures[i]->Release();
	}
	for(UINT i =0; i < meshes.size(); i++){
		if(meshes[i]) DestroyMesh(meshes[i]);
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

void SGD3D::DestroyMesh(Mesh* mesh){
	if(mesh->indices) delete[] mesh->indices;
	if(mesh->vertices) delete[] mesh->vertices;
};

