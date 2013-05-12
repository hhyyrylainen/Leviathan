#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RENDERING_MODEL
#include "Model.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
RenderModel::RenderModel(){
	// init values to NULL //
	Vertexbuffer = NULL;
	Indexbuffer = NULL;
	pRenderModel = NULL;
	pTexture = NULL;

	VertexCount = 0;
	IndexCount = 0;
	Inited = false;
	
	mtype = MTEXTURETYPE;
}
RenderModel::~RenderModel(){
	if(Inited){
		this->Release();
	}
}

bool RenderModel::Init(ID3D11Device* device, wstring modelfile, wstring texturefile, wstring texture2){
	// load model //
	if(!LoadRenderModel(modelfile)){
		Logger::Get()->Error(L"Failed to init RenderModel: loading model from file: "+modelfile);
		return false;
	}
	// init buffers //
	if(!InitBuffers(device)){
		Logger::Get()->Error(L"Failed to init RenderModel, init buffers failed");
		return false;
	}

	// Load the texture for this model.
	if(!LoadTextures(device, texturefile, texture2)){
		Logger::Get()->Error(L"Failed to init RenderModel, init Texture failed",0);
		return false;
	}
	Inited = true;

	return true;
}
void RenderModel::Release(){
	// release all objects //

	SAFE_RELEASEDEL(pTexture);
	SAFE_DELETE_ARRAY(pRenderModel);
	SAFE_RELEASE(Indexbuffer);
	SAFE_RELEASE(Vertexbuffer);

	Inited = false;
}
void RenderModel::Render(ID3D11DeviceContext* devcont){
	// set buffers to renderer for rendering //
	RenderBuffers(devcont);
}
int RenderModel::GetIndexCount(){
	return IndexCount;
}

ID3D11ShaderResourceView** RenderModel::GetTextureArray(){
	return pTexture->GetTextureArray();
}
ID3D11ShaderResourceView* RenderModel::GetTexture(){
	return pTexture->GetTexture();
}
// ---------------------------------- //
bool RenderModel::LoadRenderModel(wstring modelfile){
	// file type check //
	wstring filetype = FileSystem::GetExtension(modelfile);
	if(filetype == L"x"){
		// directx model file //

	}
	if(filetype == L"obj"){
		// autodesk object file, hopefully from Blender //
		wifstream reader;
		wchar_t readchar;


		// Initialize the counts.
		VertexCount = 0;
		int texturecount = 0;
		int normalcount = 0;
		int facecount = 0;

		// Open the file.
		reader.open(modelfile);

		// Check if it was successful in opening the file.
		if(!reader.good()){

			return false;
		}

		// Read from the file and continue to read until the end of the file is reached.
		reader.get(readchar);
		while(reader.good()){

			// If the line starts with 'v' then count either the vertex, the texture coordinates, or the normal vector.
			if(readchar == L'v'){
				reader.get(readchar);
				if(readchar == L' '){ VertexCount++; }
				if(readchar == L't'){ texturecount++; }
				if(readchar == L'n'){ normalcount++; }
			}

			// If the line starts with 'f' then increment the face count.
			if(readchar == 'f'){
				reader.get(readchar);
				if(readchar == L' '){ facecount++; }
			}
		
			// Otherwise read in the remainder of the line.
			while(readchar != L'\n'){
				reader.get(readchar);
			}

			// Start reading the beginning of the next line.
			reader.get(readchar);
		}

		// go back to beginning
		reader.clear();
		reader.seekg(0, 0);
		

		VertexFormat *vertices, *texcoords, *normals;
		FaceType *faces;
		int vertexindex, texcoordindex, normalindex, faceindex; //vindex, tindex, nindex;

		// Initialize the four data structures.
		vertices = new VertexFormat[VertexCount];
		if(!vertices){
			return false;
		}

		texcoords = new VertexFormat[texturecount];
		if(!texcoords){
			return false;
		}

		normals = new VertexFormat[normalcount];
		if(!normals){
			return false;
		}

		faces = new FaceType[facecount];
		if(!faces){
			return false;
		}

		// Initialize the indexes.
		vertexindex = 0;
		texcoordindex = 0;
		normalindex = 0;
		faceindex = 0;

		if(!reader.good()){
			return false;
		}

		// Read in the vertices, texture coordinates, and normals into the data structures.
		// Important: Also convert to left hand coordinate system since Maya uses right hand coordinate system.
		reader.get(readchar);
		while(reader.good()){
			if(readchar == L'v'){
				reader.get(readchar);

				// Read in the vertices.
				if(readchar == L' '){
					reader >> vertices[vertexindex].x >> vertices[vertexindex].y >> vertices[vertexindex].z;

					// Invert the Z vertex to change to left hand system.
					vertices[vertexindex].z = vertices[vertexindex].z * -1.0f;
					vertexindex++; 
				}

				// Read in the texture uv coordinates.
				if(readchar == L't'){ 
					reader >> texcoords[texcoordindex].x >> texcoords[texcoordindex].y;

					// Invert the V texture coordinates to left hand system.
					texcoords[texcoordindex].y = 1.0f - texcoords[texcoordindex].y;
					texcoordindex++; 
				}

				// Read in the normals.
				if(readchar == L'n'){ 
					reader >> normals[normalindex].x >> normals[normalindex].y >> normals[normalindex].z;

					// Invert the Z normal to change to left hand system.
					normals[normalindex].z = normals[normalindex].z * -1.0f;
					normalindex++; 
				}
			}

			// Read in the faces.
			if(readchar == L'f'){
				reader.get(readchar);
				if(readchar == L' '){

					// Read the face data in backwards to convert it to a left hand system from right hand system.
					reader >> faces[faceindex].vIndex3;
					// check if coord index is defined //
					if(reader.peek() == L'/'){
						// check for value
						reader.get(readchar);
						if(reader.peek() != L'/'){
							reader >> faces[faceindex].tIndex3;
							if(reader.peek() == L'/'){
								reader.get(readchar);
								reader >> faces[faceindex].nIndex3;
							}
						} else {
							faces[faceindex].tIndex3 = 0;
							// skip 1 '/' to get to normal index //
							reader.get(readchar);
							//reader.get(readchar);
							reader >> faces[faceindex].nIndex3;
						}
					} else {
						faces[faceindex].tIndex3 = 0;
						faces[faceindex].nIndex3 = 0;
					}
					// second //
					reader >> faces[faceindex].vIndex2;
					// check if coord index is defined //
					if(reader.peek() == L'/'){
						// check for value
						reader.get(readchar);
						if(reader.peek() != L'/'){
							reader >> faces[faceindex].tIndex2;
							if(reader.peek() == L'/'){
								reader.get(readchar);
								reader >> faces[faceindex].nIndex2;
							}
						} else {
							faces[faceindex].tIndex2 = 0;
							// skip 1 '/' to get to normal index //
							reader.get(readchar);
							//reader.get(readchar);
							reader >> faces[faceindex].nIndex2;
						}
					} else {
						faces[faceindex].tIndex2 = 0;
						faces[faceindex].nIndex2 = 0;
					}

					// first which is actually last in file //
					reader >> faces[faceindex].vIndex1;
					// check if coord index is defined //
					if(reader.peek() == L'/'){
						// check for value
						reader.get(readchar);
						if(reader.peek() != L'/'){
							reader >> faces[faceindex].tIndex1;
							if(reader.peek() == L'/'){
								reader.get(readchar);
								reader >> faces[faceindex].nIndex1;
							}
						} else {
							faces[faceindex].tIndex1 = 0;
							// skip 1 '/' to get to normal index //
							reader.get(readchar);
							//reader.get(readchar);
							reader >> faces[faceindex].nIndex1;
						}
					} else {
						faces[faceindex].tIndex1 = 0;
						faces[faceindex].nIndex1 = 0;
					}
					faceindex++;
				}
			}

			// Read in the remainder of the line.
			while(readchar != L'\n'){
				reader.get(readchar);
			}

			// Start reading the beginning of the next line.
			reader.get(readchar);
		}

		// Close the file.
		reader.close();


		vertexindex = 0;

		int vIndex = 0;
		int tIndex = 0;
		int nIndex = 0;


		// loading model //

		VertexCount = facecount*3;

		// Set the number of indices to be the same as the vertex count.
		IndexCount = VertexCount;

		// Create the model using the vertex count that was read in.
		pRenderModel = new RenderModelType[VertexCount];
		if(!pRenderModel){
			return false;
		}

		// Load in vertex data
		int fileverticeindex = 0;

		for(int i = 0; i < facecount; i++){


			vIndex = faces[i].vIndex1 - 1;
			tIndex = faces[i].tIndex1 - 1;
			nIndex = faces[i].nIndex1 - 1;

			pRenderModel[fileverticeindex].x = vertices[vIndex].x;
			pRenderModel[fileverticeindex].y = vertices[vIndex].y;
			pRenderModel[fileverticeindex].z = vertices[vIndex].z;
			if(tIndex > -1){
				pRenderModel[fileverticeindex].tu = texcoords[tIndex].x;
				pRenderModel[fileverticeindex].tv = texcoords[tIndex].y;
			} else {
				pRenderModel[fileverticeindex].tu = 0;
				pRenderModel[fileverticeindex].tv = 0;
			}
			if(nIndex > -1){	
				pRenderModel[fileverticeindex].nx = normals[nIndex].x;
				pRenderModel[fileverticeindex].ny = normals[nIndex].y;
				pRenderModel[fileverticeindex].nz = normals[nIndex].z;
			} else {
				pRenderModel[fileverticeindex].nx = 0;
				pRenderModel[fileverticeindex].ny = 0;
				pRenderModel[fileverticeindex].nz = 0;
			}

			fileverticeindex++;
			vIndex = faces[i].vIndex2 - 1;
			tIndex = faces[i].tIndex2 - 1;
			nIndex = faces[i].nIndex2 - 1;

			pRenderModel[fileverticeindex].x = vertices[vIndex].x;
			pRenderModel[fileverticeindex].y = vertices[vIndex].y;
			pRenderModel[fileverticeindex].z = vertices[vIndex].z;
			if(tIndex > -1){
				pRenderModel[fileverticeindex].tu = texcoords[tIndex].x;
				pRenderModel[fileverticeindex].tv = texcoords[tIndex].y;
			} else {
				pRenderModel[fileverticeindex].tu = 0;
				pRenderModel[fileverticeindex].tv = 0;
			}
			if(nIndex > -1){	
				pRenderModel[fileverticeindex].nx = normals[nIndex].x;
				pRenderModel[fileverticeindex].ny = normals[nIndex].y;
				pRenderModel[fileverticeindex].nz = normals[nIndex].z;
			} else {
				pRenderModel[fileverticeindex].nx = 0;
				pRenderModel[fileverticeindex].ny = 0;
				pRenderModel[fileverticeindex].nz = 0;
			}

			fileverticeindex++;
			vIndex = faces[i].vIndex3 - 1;
			tIndex = faces[i].tIndex3 - 1;
			nIndex = faces[i].nIndex3 - 1;

			pRenderModel[fileverticeindex].x = vertices[vIndex].x;
			pRenderModel[fileverticeindex].y = vertices[vIndex].y;
			pRenderModel[fileverticeindex].z = vertices[vIndex].z;
			if(tIndex > -1){
				pRenderModel[fileverticeindex].tu = texcoords[tIndex].x;
				pRenderModel[fileverticeindex].tv = texcoords[tIndex].y;
			} else {
				pRenderModel[fileverticeindex].tu = 0;
				pRenderModel[fileverticeindex].tv = 0;
			}
			if(nIndex > -1){	
				pRenderModel[fileverticeindex].nx = normals[nIndex].x;
				pRenderModel[fileverticeindex].ny = normals[nIndex].y;
				pRenderModel[fileverticeindex].nz = normals[nIndex].z;
			} else {
				pRenderModel[fileverticeindex].nx = 0;
				pRenderModel[fileverticeindex].ny = 0;
				pRenderModel[fileverticeindex].nz = 0;
			}

			fileverticeindex++;
		}

		// release data structures //
		SAFE_DELETE_ARRAY(vertices);
		SAFE_DELETE_ARRAY(texcoords);
		SAFE_DELETE_ARRAY(normals);
		SAFE_DELETE_ARRAY(faces);

		//for(int i = 0; i < FileVertices.size(); i++){

		//	pRenderModel[i].x = FileVertices[i].x;
		//	pRenderModel[i].y = FileVertices[i].y;
		//	pRenderModel[i].z = FileVertices[i].z;
		//	pRenderModel[i].tu = FileVertices[i].tx;
		//	pRenderModel[i].tv = FileVertices[i].ty;
		//	pRenderModel[i].nx = FileVertices[i].nx;
		//	pRenderModel[i].ny = FileVertices[i].ny;
		//	pRenderModel[i].nz = FileVertices[i].nz;
		//}

		return true;

	}
	Logger::Get()->Info(L"Typeless model: "+modelfile);

	// Open the model file.
	wifstream reader;
	reader.open(modelfile);
	
	// If it could not open the file then exit.
	if(!reader.good()){
		Logger::Get()->Error(L"Can't LoadRenderModel, file could not be opened or is empty"+modelfile);
		return false;
	}

	// Read up to the value of vertex count.
	wchar_t read;
	reader.get(read);
	while(read != L':'){

		reader.get(read);
	}

	// Read in the vertex count.
	reader >> VertexCount;

	// Set the number of indices to be the same as the vertex count.
	IndexCount = VertexCount;

	// Create the model using the vertex count that was read in.
	pRenderModel = new RenderModelType[VertexCount];
	if(!pRenderModel){
		return false;
	}

	// Read up to the beginning of the data.
	reader.get(read);
	while(read != L':'){

		reader.get(read);
	}
	reader.get(read);
	reader.get(read);

	// Read in the vertex data.
	for(int i = 0; i < VertexCount; i++){

		reader >> pRenderModel[i].x >> pRenderModel[i].y >> pRenderModel[i].z;
		reader >> pRenderModel[i].tu >> pRenderModel[i].tv;
		reader >> pRenderModel[i].nx >> pRenderModel[i].ny >> pRenderModel[i].nz;
	}

	// Close the model file.
	reader.close();

	return true;
}
bool RenderModel::InitBuffers(ID3D11Device* device){
		
		
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT hr = S_OK;

	// create vertex array //
	VertexType* verticet;
	verticet = new VertexType[VertexCount];

	// create index array //
	unsigned long* indices;
	indices = new unsigned long[IndexCount];
	// load arrays with data //

	for(int i = 0; i < VertexCount; i++){

		verticet[i].position = D3DXVECTOR3(pRenderModel[i].x, pRenderModel[i].y, pRenderModel[i].z);
		verticet[i].texture = D3DXVECTOR2(pRenderModel[i].tu, pRenderModel[i].tv);
		verticet[i].normal = D3DXVECTOR3(pRenderModel[i].nx, pRenderModel[i].ny, pRenderModel[i].nz);

		indices[i] = i;
	}

	// set buffer descs //
	D3D11_BUFFER_DESC Vertexbufferdesc;
	Vertexbufferdesc.Usage = D3D11_USAGE_DEFAULT;
	Vertexbufferdesc.ByteWidth = sizeof(VertexType) * VertexCount;
	Vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	Vertexbufferdesc.CPUAccessFlags = 0;
	Vertexbufferdesc.MiscFlags = 0;
	Vertexbufferdesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = verticet;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// create vertex buffer //
	hr = device->CreateBuffer(&Vertexbufferdesc, &vertexData, &Vertexbuffer);
	if(FAILED(hr))
	{
		Logger::Get()->Error(L"Failed to init RenderModelhandler buffers, create vertex buffer failed",0);
		return false;
	}

	// Set up the description of the static index buffer.
	D3D11_BUFFER_DESC Indexbufferdesc;
	Indexbufferdesc.Usage = D3D11_USAGE_DEFAULT;
	Indexbufferdesc.ByteWidth = sizeof(unsigned long) * IndexCount;
	Indexbufferdesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	Indexbufferdesc.CPUAccessFlags = 0;
	Indexbufferdesc.MiscFlags = 0;
	Indexbufferdesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// create index buff //
	hr = device->CreateBuffer(&Indexbufferdesc, &indexData, &Indexbuffer);
	if(FAILED(hr))
	{
		Logger::Get()->Error(L"Failed to init RenderModelhandler buffers, create index buffer failed",0);
		return false;
	}

	SAFE_DELETE_ARRAY(verticet);
	SAFE_DELETE_ARRAY(indices);

	return true;
}

bool RenderModel::LoadTextures(ID3D11Device* dev, wstring filename, wstring filename2){

	// Create the texture object.
	pTexture = new TextureArray;
	if(!pTexture){
		return false;
	}

	// Initialize the texture object.
	if(!pTexture->Init(dev, filename, filename2)){
		return false;
	}

	return true;
}

// ---------------------------------- //
void RenderModel::RenderBuffers(ID3D11DeviceContext* devcont){
	unsigned int stride;
	unsigned int offset;


	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType); 
	offset = 0;
	
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	devcont->IASetVertexBuffers(0, 1, &Vertexbuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	devcont->IASetIndexBuffer(Indexbuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	devcont->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

bool RenderModel::UsingMultitextures(){
	return pTexture->IsMultiTexture();
}

