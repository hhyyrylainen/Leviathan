#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RENDERINGRESOURCECREATOR
#include "RenderingResourceCreator.h"
#endif
using namespace Leviathan;
using namespace Leviathan::Rendering;
// ------------------------------------ //
#include "Graphics.h"


DLLEXPORT Leviathan::Rendering::ResourceCreator::ResourceCreator(){

}

DLLEXPORT Leviathan::Rendering::ResourceCreator::~ResourceCreator(){

}

Graphics* Leviathan::Rendering::ResourceCreator::GraphicsAccess = NULL;
// ------------------------------------ //
void Leviathan::Rendering::ResourceCreator::StoreGraphicsInstance(Graphics* instance){
	// set static thing //
	GraphicsAccess = instance;
}

ID3D11Buffer* Leviathan::Rendering::ResourceCreator::GenerateDefaultIndexBuffer(const int &indexcount){
	// generates a "default" index buffer with indexcount elements //
	D3D11_BUFFER_DESC IBufferDesc = CreateBufferDefinition(D3D11_BIND_INDEX_BUFFER, sizeof(ULONG)*indexcount, false);

	// create initial data //
	ULONG* indices = new ULONG[indexcount];
	if(!indices){
		return NULL;
	}

	// fill with data //
	for(int i = 0; i < indexcount; i++){
		// point directly to matching index //
		indices[i] = i;
	}


	// create buffer //
	D3D11_SUBRESOURCE_DATA indexdata;

	ZeroMemory(&indexdata, sizeof(indexdata));

	// set pointing to indices array //
	indexdata.pSysMem = indices;

	ID3D11Buffer* tmpholder = NULL;

	// Finally create the index buffer that is requested //
	HRESULT hr = GraphicsAccess->GetRenderer()->GetDevice()->CreateBuffer(&IBufferDesc, &indexdata, &tmpholder);
	if(FAILED(hr)){

		SAFE_DELETE_ARRAY(indices);
		return tmpholder;
	}

	// release data //
	SAFE_DELETE_ARRAY(indices);

	return tmpholder;
}

ID3D11Buffer* Leviathan::Rendering::ResourceCreator::GenerateDefaultDynamicDefaultTypeVertexBuffer(const int &count){
	// generates a "default" vertex buffer with VertexType elements and memory initialized to zeros //
	D3D11_BUFFER_DESC VBufferDescription = Rendering::ResourceCreator::CreateBufferDefinition(D3D11_BIND_VERTEX_BUFFER, sizeof(VertexType)*count, true);

	// create initial data that is set to zeros //
	VertexType* vertices = new VertexType[count];
	if(!vertices){
		return NULL;
	}
	// set initial data to zeros //
	ZeroMemory(vertices, sizeof(vertices));

	D3D11_SUBRESOURCE_DATA vertexdata;

	ZeroMemory(&vertexdata, sizeof(vertexdata));
		// set pointing to vertices array //
	vertexdata.pSysMem = vertices;

	ID3D11Buffer* tmpholder = NULL;

	// Create the vertex buffer //
	HRESULT hr =  GraphicsAccess->GetRenderer()->GetDevice()->CreateBuffer(&VBufferDescription, &vertexdata, &tmpholder);
	if(FAILED(hr)){
		return tmpholder;
	}


	// release memory //
	SAFE_DELETE_ARRAY(vertices);

	return tmpholder;
}

D3D11_BUFFER_DESC Leviathan::Rendering::ResourceCreator::CreateBufferDefinition(const D3D11_BIND_FLAG &bindflags, const UINT bytewidth, bool allowcpu /*= false*/){
	// creates a "default" D3D11_BUFFER_DESC according to parameters //
	D3D11_BUFFER_DESC BDesc;
	// set everything to zeros //
	ZeroMemory(&BDesc, sizeof(BDesc));

	// apply parameter flags //
	// Set up the description of the static index buffer.
	BDesc.Usage = allowcpu ? D3D11_USAGE_DYNAMIC: D3D11_USAGE_DEFAULT;
	BDesc.ByteWidth = bytewidth;
	BDesc.BindFlags = bindflags;
	BDesc.CPUAccessFlags = allowcpu ? D3D11_CPU_ACCESS_WRITE: 0;

	// return created buffer //
	return BDesc;
}

VertexType* Leviathan::Rendering::ResourceCreator::GenerateQuadIntoVertexBuffer(const Float2 &location, const Float2 &size, const int &numvertices, 
	const int &Coordtype, const int &style /*= QUAD_FILLSTYLE_UPPERLEFT_0_BOTTOMRIGHT_1*/)
{
	// create the vertex array //
	if(numvertices < 4 || numvertices % 2 != 0){
		// cannot be done //
		assert(0 && "invalid square point count");
	}

	if(numvertices != 6)
		DEBUG_BREAK;


	// allocate vertices array that is returned //
	VertexType* vertices = new VertexType[numvertices];
	if(!vertices){
		return NULL;
	}

	Generate2DQuadCoordinatesWithStyle(vertices[0].texture, vertices[2].texture, vertices[4].texture, vertices[5].texture, style);

	Float4 Quadcoordinates;

	Generate2DCoordinatesFromLocationAndSize(location, size, Coordtype, Quadcoordinates);

	// First triangle.
	vertices[0].position = D3DXVECTOR3(Quadcoordinates.X, Quadcoordinates.Z, 0.0f);  // Top left.

	vertices[1].position = D3DXVECTOR3(Quadcoordinates.Y, Quadcoordinates.W, 0.0f);  // Bottom right.
	vertices[1].texture = vertices[5].texture;

	vertices[2].position = D3DXVECTOR3(Quadcoordinates.X, Quadcoordinates.W, 0.0f);  // Bottom left.


	// Second triangle.
	vertices[3].position = D3DXVECTOR3(Quadcoordinates.X, Quadcoordinates.Z, 0.0f);  // Top left.
	vertices[3].texture = vertices[0].texture;

	vertices[4].position = D3DXVECTOR3(Quadcoordinates.Y, Quadcoordinates.Z, 0.0f);  // Top right.


	vertices[5].position = D3DXVECTOR3(Quadcoordinates.Y, Quadcoordinates.W, 0.0f);  // Bottom right.


	// return the generated vertices //
	return vertices;
}

bool Leviathan::Rendering::ResourceCreator::Generate2DCoordinatesFromLocationAndSize(const Float2 &location, const Float2 &size, int coordtype, Float4 &receiver){

	if(coordtype == GUI_POSITIONABLE_COORDTYPE_RELATIVE){
		// generate absolute positions while //

		// Calculate the screen coordinates of the left side of the bitmap.
		receiver.X = ((DataStore::Get()->GetWidth() / 2.f)*-1)+location.X*DataStore::Get()->GetWidth();

		// Calculate the screen coordinates of the right side of the bitmap.
		receiver.Y = receiver.X + size.X*DataStore::Get()->GetWidth();

		// Calculate the screen coordinates of the top of the bitmap.
		receiver.Z = (DataStore::Get()->GetHeight() / 2.f)-location.Y*DataStore::Get()->GetHeight();

		// Calculate the screen coordinates of the bottom of the bitmap.
		receiver.W = receiver.Z-size.Y*DataStore::Get()->GetHeight();
		return true;
	} else {

		// Calculate the screen coordinates of the left side of the bitmap.
		receiver.X = ((DataStore::Get()->GetWidth() / 2.f) *-1)+location.X;

		// Calculate the screen coordinates of the right side of the bitmap.
		receiver.Y = receiver.X+size.X;

		// Calculate the screen coordinates of the top of the bitmap.
		receiver.Z = (DataStore::Get()->GetHeight() / 2.f)-location.Y;

		// Calculate the screen coordinates of the bottom of the bitmap.
		receiver.W = receiver.Z-size.Y;
		return true;
	}
}

void Leviathan::Rendering::ResourceCreator::Generate2DQuadCoordinatesWithStyle(D3DXVECTOR2 &leftop, D3DXVECTOR2 &leftbottom, 
	D3DXVECTOR2 &righttop, D3DXVECTOR2 &rigthbottom, int texturecoordinatestyle)
{
	// switch on style //
	switch(texturecoordinatestyle){
	default:
		// maybe assert here, or just fall through...
	case QUAD_FILLSTYLE_UPPERLEFT_1_BOTTOMRIGHT_0:
		{
			// opposite flow direction //
			leftop = D3DXVECTOR2(1.0f, 1.0f);
			leftbottom = D3DXVECTOR2(1.0f, 0.0f);
			rigthbottom = D3DXVECTOR2(0.0f, 0.0f);
			righttop = D3DXVECTOR2(0.0f, 1.0f);
		}
		return;
	case QUAD_FILLSTYLE_UPPERLEFT_0_BOTTOMRIGHT_1:
		{
			leftop = D3DXVECTOR2(0.0f, 0.0f);
			leftbottom = D3DXVECTOR2(0.0f, 1.0f);
			rigthbottom = D3DXVECTOR2(1.0f, 1.0f);
			righttop = D3DXVECTOR2(1.0f, 0.0f);
		}
		return;
	}
}

ID3D11Texture2D* Leviathan::Rendering::ResourceCreator::GenerateDefaultTexture(const int &width, const int &height, const DXGI_FORMAT &format, const D3D11_BIND_FLAG &bflags){
	// create a "default" texture and return it //
	D3D11_TEXTURE2D_DESC TDesc;
	// set everything to zeros //
	ZeroMemory(&TDesc, sizeof(TDesc));
	
	// set right parameters //

	throw exception("Unfinished");


	//D3D11_SUBRESOURCE_DATA InitialData[2];


	//ID3D11Texture2D* tmpptr = NULL;


	//HRESULT hr =  GraphicsAccess->GetRenderer()->GetDevice()->CreateTexture2D(&TDesc, &InitialData[0], &tmpptr);
	//if(FAILED(hr)){
	//	// failed to create //
	//	return NULL;
	//}

	//return tmpptr;
}

bool Leviathan::Rendering::ResourceCreator::CreateDynamicConstantBufferForVSShader(ID3D11Buffer** receiver, const UINT &bufferbytewidth){
	// setup the buffer description //
	D3D11_BUFFER_DESC BuffDesc;
	BuffDesc.Usage = D3D11_USAGE_DYNAMIC;
	BuffDesc.ByteWidth = bufferbytewidth;
	BuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	BuffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	BuffDesc.MiscFlags = 0;
	BuffDesc.StructureByteStride = 0;
	
	// create the buffer and interpret return code //
	HRESULT hr = GraphicsAccess->GetRenderer()->GetDevice()->CreateBuffer(&BuffDesc, NULL, receiver);
	return FAILED(hr) ? false: true;
}

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //

