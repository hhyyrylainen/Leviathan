#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_2DTEXTRENDERER
#include "TextRenderer.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "3DRenderer.h"
#include "Graphics.h"

TextRenderer::TextRenderer(){
//	m_Font = NULL;
	m_FontShader = NULL;

	//m_sentence1 = NULL;
	//m_sentence2 = NULL;
}
TextRenderer::~TextRenderer(){

}
// ------------------------------------ //
bool TextRenderer::Init(ID3D11Device* dev, ID3D11DeviceContext* devcont, Window* wind, D3DXMATRIX baseview){
	// store values //
	ScreenWidth = wind->GetWidth();
	ScreenHeight = wind->GetHeight();
	m_baseViewMatrix = baseview;

	device = dev;
	// create fonts //
	wstring ArialFont = L"Arial.dds";

	LoadFont(/*FileSystem::GetFontFolder()+*/ArialFont);
	//m_Font = new Font;
	//if(!m_Font)
	//	return false;
	////if(!m_Font->Init(dev, FileSystem::GetFontFolder()+L"ArialSmall.dds")){
	//if(!m_Font->Init(dev, FileSystem::GetFontFolder()+L"font.dds")){

	//	Logger::Get()->Error(L"Failed to init TextRenderer, init font failed", true);
	//	return false;
	//}

	// create shader //
	m_FontShader = new FontShader;
	if(!m_FontShader)
		return false;

	if(!m_FontShader->Init(dev, wind)){
		Logger::Get()->Error(L"Failed to init TextRenderer, init fontshader failed", true);
		return false;
	}


	// start monitoring for change in resolution //
	StartMonitoring(DATAINDEX_HEIGHT, false);
	StartMonitoring(DATAINDEX_WIDTH, false);

	// init sentences //
	//Sentences.push_back(NULL);

	//if(!InitializeSentence(&Sentences.at(Sentences.size()-1), 16, dev)){

	//	return false;
	//}

	//if(!UpdateSentence(Sentences.at(Sentences.size()-1), L"Hello", 100, 100, 1.0f, 1.0f, 1.0f, devcont)){

	//	return false;
	//}
	
	//if(!InitializeSentence(&m_sentence1, 25, dev)){

	//	return false;
	//}

	//if(!UpdateSentence(m_sentence1, L"Alpha() Test{}!", 100, 100, 0.2f, 0.5f, 0.9f, 4.0f, devcont)){

	//	return false;
	//}
	return true;
}
void TextRenderer::Release(){
	// release shader
	SAFE_RELEASEDEL(m_FontShader);
	
	// release fonts //
	//SAFE_RELEASEDEL(m_Font);
	for(unsigned int i = 0; i < FontHolder.size(); i++){
		SAFE_RELEASEDEL(FontHolder[i]);
	}
	FontHolder.clear();
	// release sentences //
	while(Sentences.size() > 0){
		ReleaseSentence(&Sentences.at(Sentences.size()-1));
		Sentences.pop_back();

	}
	//ReleaseSentence(&m_sentence1);

	// stop monitoring everything //
	StopMonitoring(DATAINDEX_HEIGHT, L"", true);
}
// ------------------------------------ //
bool TextRenderer::CreateSentence(int id, int maxlenght, ID3D11Device* dev){
	Sentences.push_back(NULL);
	if(!InitializeSentence(&VECTOR_LAST(Sentences), id, maxlenght, dev)){

		Logger::Get()->Error(L"Failed to add new sentence, Init sentence failed");
		return false;
	}
	return true;
}
bool TextRenderer::UpdateSentenceID(int id, bool absolute, wstring &Font, wstring &text, int x, int y, Float4 &color, float sizepercent, 
	ID3D11DeviceContext* devcont, bool TranslateSize)
{
	for(unsigned int i = 0; i < Sentences.size(); i++){
		if(Sentences[i]->SentenceID == id){

			// get font id //
			int fontid = GetFontIndex(Font);
			UpdateSentence(Sentences[i], absolute, text, x, y, color[0], color[1], color[2], sizepercent, fontid, devcont, TranslateSize);
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

int TextRenderer::CountSentenceLenght(wstring &sentence, wstring &font, float heightmod, bool IsAbsolute, bool TranslateSize){
	int index = GetFontIndex(font);
	ARR_INDEX_CHECK(index, (int)FontHolder.size()){
		return FontHolder[index]->CountLenght(sentence, heightmod, IsAbsolute, TranslateSize);
	}
	return -1;
}
int TextRenderer::GetFontHeight(wstring &font, float heightmod, bool IsAbsolute, bool TranslateSize){
	int index = GetFontIndex(font);
	ARR_INDEX_CHECK(index, (int)FontHolder.size()){
		return FontHolder[index]->GetHeight(heightmod, IsAbsolute, TranslateSize);
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
void TextRenderer::LoadFont(wstring &file){
	FontHolder.push_back(new Font());

	if(!VECTOR_LAST(FontHolder)->Init(device, file)){

		Logger::Get()->Error(L"Error while loading font", true);
	}
}
int TextRenderer::GetFontIndex(wstring &name){
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

bool TextRenderer::InitializeSentence(SentenceType** sentence, int id, int maxlenght, ID3D11Device* device){
	
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

	(*sentence)->maxLength = maxlenght;
	(*sentence)->vertexCount = 6 * maxlenght;

	// no actual value is known //
	(*sentence)->Absolute = false;

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

	// Give the subresource structure a pointer to the vertex data.
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

	// Give the subresource structure a pointer to the index data.
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

bool TextRenderer::UpdateSentence(SentenceType* sentence, bool absolute, wstring &text, int posx, int posy, float red, float green, float blue, 
	float heightpercent, int fontindex, ID3D11DeviceContext* devcont, bool TranslateSize){
	HRESULT hr = S_OK;

	//DEBUG_OUTPUT(L"TextRenderer: text updated, old "+Convert::IntToWstring(sentence->posx)+L","+Convert::IntToWstring(sentence->posy)+L" new: "+
	//	Convert::IntToWstring(posx)+L","+Convert::IntToWstring(posy)+L"\n");

	// store color //
	sentence->red = red;
	sentence->green = green;
	sentence->blue = blue;

	// font index //
	sentence->FontID = fontindex;

	sentence->Height = heightpercent;

	// store more parameters for resolution change //
	sentence->Absolute = absolute;
	sentence->TranslateSize = TranslateSize;
	sentence->posx = posx;
	sentence->posy = posy;

	sentence->text = text;


	// get letters //
	int letters = text.length();

	// check for too long string //
	if(letters > sentence->maxLength){
		// needs to recreate buffers //
		SAFE_RELEASE((sentence)->vertexBuffer);
		SAFE_RELEASE((sentence)->indexBuffer);

		D3D11_SUBRESOURCE_DATA vertexData, indexData;


		(sentence)->maxLength = letters*2;
		(sentence)->vertexCount = 6 * (sentence)->maxLength;

		(sentence)->indexCount = (sentence)->vertexCount;

		// Create the vertex array.
		VertexType* vertices;
		vertices = new VertexType[(sentence)->vertexCount];
		if(!vertices)
		{
			return false;
		}

		// Create the index array.
		unsigned long* indices;
		indices = new unsigned long[(sentence)->indexCount];
		if(!indices)
		{
			return false;
		}

		// Initialize vertex array to zeros at first.
		memset(vertices, 0, (sizeof(VertexType) * (sentence)->vertexCount));

		// Initialize the index array.
		for(int i = 0; i<(sentence)->indexCount; i++){

			indices[i] = i;
		}
		D3D11_BUFFER_DESC vertexBufferDesc;

		// Set up the description of the dynamic vertex buffer.
		vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		vertexBufferDesc.ByteWidth = sizeof(VertexType) * (sentence)->vertexCount;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		// Give the subresource structure a pointer to the vertex data.
		vertexData.pSysMem = vertices;
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		// Create the vertex buffer.
		hr = device->CreateBuffer(&vertexBufferDesc, &vertexData, &(sentence)->vertexBuffer);
		if(FAILED(hr)){

			return false;
		}

	
		// Set up the description of the static index buffer.
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(unsigned long) * (sentence)->indexCount;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		// Give the subresource structure a pointer to the index data.
		indexData.pSysMem = indices;
		indexData.SysMemPitch = 0;
		indexData.SysMemSlicePitch = 0;

		// Create the index buffer.
		hr = device->CreateBuffer(&indexBufferDesc, &indexData, &(sentence)->indexBuffer);
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
	if(absolute){
		drawX = ((((float)ScreenWidth / 2) * -1) + posx);
		drawY = (((float)ScreenHeight / 2) - posy);
	} else {
		// pos is value between 0 - 1000 (promille)  scale it to screen //
		float relx = ScreenWidth*(posx/ResolutionScaling::GetPromilleFactor());
		float rely = ScreenHeight*(posy/ResolutionScaling::GetPromilleFactor());

		drawX = ((((float)ScreenWidth / 2) * -1) + relx);
		drawY = ((((float)ScreenHeight / 2)) - rely);
	}


	// use font to build vertex array //
	FontHolder[fontindex]->BuildVertexArray((void*) vertices, text, drawX, drawY, sentence->Height, absolute, TranslateSize); // this needs absolute parameter //
	//FontHolder[fontindex]->BuildVertexArray((void*) vertices, text, 50, 50, sentence->Height, absolute); // this needs absolute parameter //
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

	// Create pixel color
	Float4 pixelColor(sentence->red, sentence->green, sentence->blue, 1.0f);
	//pixelColor = D3DXVECTOR4(sentence->red, sentence->green, sentence->blue, 1.0f);

	// Render the text using the font shader.
	if(!m_FontShader->Render(devcont, sentence->indexCount, worldMatrix, m_baseViewMatrix, orthoMatrix, FontHolder[sentence->FontID]->GetTexture(), pixelColor)){
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
			UpdateSentence(Sentences[i], Sentences[i]->Absolute, Sentences[i]->text, Sentences[i]->posx, Sentences[i]->posy, Sentences[i]->red,
				Sentences[i]->green, Sentences[i]->blue, Sentences[i]->Height, Sentences[i]->FontID, temprequired->GetDeviceContext(), Sentences[i]->TranslateSize);
		}

	}
}
