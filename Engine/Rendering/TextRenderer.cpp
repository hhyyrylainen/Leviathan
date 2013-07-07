#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_2DTEXTRENDERER
#include "TextRenderer.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "3DRenderer.h"
#include "Graphics.h"
#include "..\GuiPositionable.h"

TextRenderer::TextRenderer(){
	_FontShader = NULL;

}
TextRenderer::~TextRenderer(){

}
// ------------------------------------ //
bool TextRenderer::Init(ID3D11Device* dev, ID3D11DeviceContext* devcont, Window* wind, D3DXMATRIX baseview){
	// store values //
	ScreenWidth = wind->GetWidth();
	ScreenHeight = wind->GetHeight();
	BaseViewMatrix = baseview;

	device = dev;
	// create fonts //
	wstring ArialFont = L"Arial.dds";

	LoadFont(ArialFont);


	// create shader //
	_FontShader = new FontShader;
	if(!_FontShader)
		return false;

	if(!_FontShader->Init(dev)){
		Logger::Get()->Error(L"Failed to init TextRenderer, init font shader failed", true);
		return false;
	}


	// start monitoring for change in resolution //
	vector<shared_ptr<VariableBlock>> tolisten(2);
	tolisten[0] = shared_ptr<VariableBlock>(new VariableBlock(DATAINDEX_HEIGHT));
	tolisten[1] = shared_ptr<VariableBlock>(new VariableBlock(DATAINDEX_WIDTH));

	StartMonitoring(tolisten);

	return true;
}
void TextRenderer::Release(){
	// release shader
	SAFE_RELEASEDEL(_FontShader);
	
	// release fonts //
	for(unsigned int i = 0; i < FontHolder.size(); i++){
		SAFE_RELEASEDEL(FontHolder[i]);
	}
	FontHolder.clear();
	// release sentences //
	while(Sentences.size() > 0){
		ReleaseSentence(&Sentences.at(Sentences.size()-1));
		Sentences.pop_back();
	}

	// stop monitoring everything //
	StopMonitoring(MonitoredValues, true);
}
// ------------------------------------ //
bool TextRenderer::CreateSentence(int id, int maxlength, ID3D11Device* dev){
	Sentences.push_back(NULL);
	if(!InitializeSentence(&VECTOR_LAST(Sentences), id, maxlength, dev)){

		Logger::Get()->Error(L"Failed to add new sentence, Init sentence failed");
		return false;
	}
	return true;
}
DLLEXPORT bool Leviathan::TextRenderer::UpdateSentenceID(int id, int Coordtype, const wstring &font, const wstring &text, const Float2 &coordinates, 
	const Float4 &color, float sizepercent, ID3D11DeviceContext* devcont){
	for(size_t i = 0; i < Sentences.size(); i++){
		if(Sentences[i]->SentenceID == id){

			// get font id //
			int fontid = GetFontIndex(font);
			UpdateSentence(Sentences[i], Coordtype, text, coordinates, color, sizepercent, fontid, devcont);
			return true;
		}
	}
	return false;
}
void TextRenderer::ReleaseSentenceID(int id){
	for(unsigned int i = 0; i < Sentences.size(); i++){
		if(Sentences[i]->SentenceID == id){
			// release //
			ReleaseSentence(&Sentences[i]);
			SAFE_DELETE(Sentences[i]);
			Sentences.erase(Sentences.begin()+i);
			break;
		}
	}
}
void TextRenderer::HideSentence(int id, bool hidden){
	for(unsigned int i = 0; i < Sentences.size(); i++){
		if(Sentences[i]->SentenceID == id){
			// set hidden flag //
			Sentences[i]->Hidden = hidden;
		}
	}

}
// ------------------------------------ //
DLLEXPORT float Leviathan::TextRenderer::CountSentenceLength(const wstring &sentence, const wstring &font, float heightmod, int coordtype){
	int index = GetFontIndex(font);
	ARR_INDEX_CHECK(index, (int)FontHolder.size()){
		return FontHolder[index]->CountLength(sentence, heightmod, coordtype);
	}
	return -1;
}
DLLEXPORT float Leviathan::TextRenderer::GetFontHeight(const wstring &font, float heightmod, int coordtype){
	int index = GetFontIndex(font);
	ARR_INDEX_CHECK(index, (int)FontHolder.size()){
		return FontHolder[index]->GetHeight(heightmod, coordtype);
	}
	return -1;
}

// ------------------------------------ //
bool TextRenderer::Render(ID3D11DeviceContext* devcont, D3DXMATRIX worldmatrix, D3DXMATRIX orthomatrix){

	// draw sentences //
	for(unsigned int i = 0; i < Sentences.size(); i++){
		if(Sentences[i]->Hidden)
			continue;
		if(!RenderSentence(devcont, Sentences.at(i), worldmatrix, orthomatrix)){

			Logger::Get()->Error(L"Failed to render sentence id:"+i, false);
			continue;
		}


	}
	//RenderSentence(devcont, m_sentence1, worldmatrix, orthomatrix);

	return true;
}
bool TextRenderer::RenderSingle(int ID, ID3D11DeviceContext* devcont, D3DXMATRIX worldmatrix, D3DXMATRIX orthomatrix){
	CheckUpdatedValues();
	for(unsigned int i = 0; i < Sentences.size(); i++){
		if(Sentences[i]->SentenceID != ID)
			continue;
		if(!RenderSentence(devcont, Sentences.at(i), worldmatrix, orthomatrix)){

			Logger::Get()->Error(L"Failed to render sentence id:"+i, false);
			return false;
		}
		return true;
	}
	return false;
}
// ------------------------------------ //
void Leviathan::TextRenderer::LoadFont(const wstring &file){
	FontHolder.push_back(new RenderingFont());

	if(!FontHolder.back()->Init(device, file)){

		Logger::Get()->Error(L"Error while loading font, file: "+file, true);
	}
}
int Leviathan::TextRenderer::GetFontIndex(const wstring &name){
	// try to get id //
	int index = -1;
	for(unsigned int i = 0; i < FontHolder.size(); i++){
		if(FontHolder[i]->Name == name){
			index = i;
			break;
		}
	}
	if(index == -1){
		// try to load font
		wstring File = name+L".dds";
		LoadFont(File);
		index = FontHolder.size()-1;
		Logger::Get()->Info(L"Font loaded! :"+name, false);
	}
	return index;
}

bool TextRenderer::InitializeSentence(SentenceType** sentence, int id, int maxlength, ID3D11Device* device){
	
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT hr = S_OK;

	// create new object //
	*sentence = new SentenceType;
	if(!*sentence)
		return false;

	// initialize values //
	(*sentence)->SentenceID = id;
	(*sentence)->vertexBuffer = NULL;
	(*sentence)->indexBuffer = NULL;

	(*sentence)->maxLength = maxlength;
	(*sentence)->vertexCount = 6 * maxlength;

	// no actual value is known //
	(*sentence)->CoordType = GUI_POSITIONABLE_COORDTYPE_RELATIVE;

	(*sentence)->indexCount = (*sentence)->vertexCount;

	// Create the vertex array.
	VertexType* vertices;
	vertices = new VertexType[(*sentence)->vertexCount];
	if(!vertices){
		QUICK_ERROR_MESSAGE;
		return false;
	}

	// Create the index array.
	unsigned long* indices;
	indices = new unsigned long[(*sentence)->indexCount];
	if(!indices){
		QUICK_ERROR_MESSAGE;
		return false;
	}

	// Initialize vertex array to zeros at first.
	memset(vertices, 0, (sizeof(VertexType) * (*sentence)->vertexCount));

	// Initialize the index array.
	for(int i = 0; i<(*sentence)->indexCount; i++){

		indices[i] = i;
	}
	D3D11_BUFFER_DESC vertexBufferDesc;

	// Set up the description of the dynamic vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * (*sentence)->vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the sub resource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Create the vertex buffer.
	hr = device->CreateBuffer(&vertexBufferDesc, &vertexData, &(*sentence)->vertexBuffer);
	if(FAILED(hr)){
		QUICK_ERROR_MESSAGE;
		return false;
	}

	
	// Set up the description of the static index buffer.
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * (*sentence)->indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the sub resource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	hr = device->CreateBuffer(&indexBufferDesc, &indexData, &(*sentence)->indexBuffer);
	if(FAILED(hr)){
		QUICK_ERROR_MESSAGE;
		return false;
	}

	// release allocated arrays
	SAFE_DELETE_ARRAY(vertices);
	SAFE_DELETE_ARRAY(indices);

	return true;
}

bool Leviathan::TextRenderer::UpdateSentence(SentenceType* sentence, int Coordtype, const wstring &text, const Float2 &position, const Float4 &colour, 
	float textmodifier, int fontindex, ID3D11DeviceContext* devcont)
{
	HRESULT hr = S_OK;

	//DEBUG_OUTPUT(L"TextRenderer: text updated, old "+Convert::IntToWstring(sentence->posx)+L","+Convert::IntToWstring(sentence->posy)+L" new: "+
	//	Convert::IntToWstring(posx)+L","+Convert::IntToWstring(posy)+L"\n");

	// store color //
	sentence->Colour = colour;

	// font index //
	sentence->FontID = fontindex;

	sentence->SizeModifier = textmodifier;

	// store more parameters for resolution change //
	sentence->CoordType = Coordtype;
	sentence->Position = position;


	sentence->text = text;

	// get letters //
	int letters = text.length();

	// check for too long string //
	if(letters > sentence->maxLength){
		// needs to recreate buffers //
		SAFE_RELEASE(sentence->vertexBuffer);
		SAFE_RELEASE(sentence->indexBuffer);

		D3D11_SUBRESOURCE_DATA vertexData, indexData;


		sentence->maxLength = letters*2;
		sentence->vertexCount = 6 * sentence->maxLength;

		sentence->indexCount = sentence->vertexCount;

		// Create the vertex array.
		VertexType* vertices;
		vertices = new VertexType[sentence->vertexCount];
		if(!vertices){
			return false;
		}

		// Create the index array.
		unsigned long* indices;
		indices = new unsigned long[sentence->indexCount];
		if(!indices){
			return false;
		}

		// Initialize vertex array to zeros at first.
		memset(vertices, 0, (sizeof(VertexType) * (sentence)->vertexCount));

		// Initialize the index array.
		for(int i = 0; i < sentence->indexCount; i++){

			indices[i] = i;
		}
		D3D11_BUFFER_DESC vertexBufferDesc;

		// Set up the description of the dynamic vertex buffer.
		vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		vertexBufferDesc.ByteWidth = sizeof(VertexType) * sentence->vertexCount;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		// Give the sub resource structure a pointer to the vertex data.
		vertexData.pSysMem = vertices;
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		// Create the vertex buffer.
		hr = device->CreateBuffer(&vertexBufferDesc, &vertexData, &sentence->vertexBuffer);
		if(FAILED(hr)){

			return false;
		}

	
		// Set up the description of the static index buffer.
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(unsigned long) * sentence->indexCount;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		// Give the sub resource structure a pointer to the index data.
		indexData.pSysMem = indices;
		indexData.SysMemPitch = 0;
		indexData.SysMemSlicePitch = 0;

		// Create the index buffer.
		hr = device->CreateBuffer(&indexBufferDesc, &indexData, &sentence->indexBuffer);
		if(FAILED(hr)){

			return false;
		}

		// release allocated arrays
		SAFE_DELETE_ARRAY(vertices);
		SAFE_DELETE_ARRAY(indices);
	}

	VertexType* vertices = new VertexType[sentence->vertexCount];
	if(!vertices){

		return false;
	}

	// Initialize vertex array to zeros at first.
	memset(vertices, 0, (sizeof(VertexType) * sentence->vertexCount));

	// Calculate the X and Y pixel position on the screen to start drawing to.
	float drawX = 0;
	float drawY = 0;
	if(Coordtype == GUI_POSITIONABLE_COORDTYPE_RELATIVE){
		// pos is value between 0 - 1 scale to screen size //
		float relx = ScreenWidth*position.X;
		float rely = ScreenHeight*position.Y;

		drawX = ((((float)ScreenWidth / 2) * -1) + relx);
		drawY = ((((float)ScreenHeight / 2)) - rely);

	} else {
		drawX = ((((float)ScreenWidth / 2) * -1) + position.X);
		drawY = (((float)ScreenHeight / 2) - position.Y);
	}

	// make draw x and draw y match screen pixels //
	drawX = (float)(int)(drawX+0.5f);
	drawY = (float)(int)(drawY+0.5f);

	// use font to build vertex array //
	if(!FontHolder[fontindex]->BuildVertexArray(vertices, text, drawX, drawY, sentence->SizeModifier, Coordtype)){
		// sentence probably has invalid characters //
		SAFE_DELETE_ARRAY(vertices);

		return false;
	}
	if(!vertices){
		// font has failed //
		Logger::Get()->Error(L"Render sentence has failed, recreating fonts", true);
		this->Release();
		Logger::Get()->Error(L"Quitting...");
		return false;
	}

	// Lock the vertex buffer so it can be written to.
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	hr = devcont->Map(sentence->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(hr)){

		return false;
	}

	// Get a pointer to the data in the vertex buffer.
	VertexType* verticesPtr = (VertexType*)mappedResource.pData;

	// Copy the data into the vertex buffer.
	memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType) * sentence->vertexCount));

	// Unlock the vertex buffer.
	devcont->Unmap(sentence->vertexBuffer, 0);

	// Release the vertex array as it is no longer needed.
	SAFE_DELETE_ARRAY(vertices);

	return true;
}
void TextRenderer::ReleaseSentence(SentenceType** sentence){
	if(*sentence){
		// Release the sentence vertex buffer.
		SAFE_RELEASE((*sentence)->vertexBuffer);
		SAFE_RELEASE((*sentence)->indexBuffer);

		// Release the sentence.
		delete *sentence;
		*sentence = NULL;
	}

}

// ------------------------------------ //
bool TextRenderer::RenderSentence(ID3D11DeviceContext* devcont, SentenceType* sentence, D3DXMATRIX worldMatrix, D3DXMATRIX orthoMatrix){

	// update check //
	CheckUpdatedValues();

	// Set vertex buffer stride and offset.
	unsigned int stride, offset;
	stride = sizeof(VertexType); 
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	devcont->IASetVertexBuffers(0, 1, &sentence->vertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	devcont->IASetIndexBuffer(sentence->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	devcont->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Render the text using the font shader.
	if(!_FontShader->Render(devcont, sentence->indexCount, worldMatrix, BaseViewMatrix, orthoMatrix, FontHolder[sentence->FontID]->GetTexture(), sentence->Colour)){
		false;
	}

	return true;
}

void Leviathan::TextRenderer::CheckUpdatedValues(){
	// check that is the window size changed //
	if(ValuesUpdated){
		// received new values //

		// new values don't have to be checked, just pop them //
		_PopUdated();

		// update stored values //
		Window* wind = Graphics::Get()->GetWindow();
		ScreenWidth = wind->GetWidth();
		ScreenHeight = wind->GetHeight();


		// get a pointer to device context for recreating arrays //
		Dx11Renderer* temprequired = Graphics::Get()->GetRenderer();

		if(!temprequired){

			QUICK_ERROR_MESSAGE;
			return;
		}

		// recreate all vertex arrays //
		for(unsigned int i = 0; i < Sentences.size(); i++){

			// call update sentence with sentence's current values //
			UpdateSentence(Sentences[i], Sentences[i]->CoordType, Sentences[i]->text, Sentences[i]->Position, Sentences[i]->Colour,
				Sentences[i]->SizeModifier, Sentences[i]->FontID, temprequired->GetDeviceContext());
		}

	}
}
