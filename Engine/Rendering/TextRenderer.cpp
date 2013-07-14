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
	// release expensive text //
	SAFE_DELETE_VECTOR(ExpensiveTexts);


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
		if(FontHolder[i]->GetName() == name){
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
	// create new object //
	*sentence = new SentenceType;
	if(!*sentence)
		return false;

	// initialize values //
	(*sentence)->SentenceID = id;
	(*sentence)->VertexBuffer = NULL;
	(*sentence)->IndexBuffer = NULL;

	(*sentence)->maxLength = maxlength;
	(*sentence)->vertexCount = 6 * maxlength;

	// no actual value is known //
	(*sentence)->CoordType = GUI_POSITIONABLE_COORDTYPE_RELATIVE;

	(*sentence)->indexCount = (*sentence)->vertexCount;

	
	// generate index buffer //
	(*sentence)->IndexBuffer = Rendering::ResourceCreator::GenerateDefaultIndexBuffer((*sentence)->indexCount);
	// set up dynamic vertex buffer //
	(*sentence)->VertexBuffer = Rendering::ResourceCreator::GenerateDefaultDynamicDefaultTypeVertexBuffer((*sentence)->vertexCount);

	// if either failed return false //
	if(!(*sentence)->IndexBuffer || !(*sentence)->VertexBuffer){

		DEBUG_BREAK;
		return false;
	}

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
		SAFE_RELEASE(sentence->VertexBuffer);
		SAFE_RELEASE(sentence->IndexBuffer);

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
		hr = device->CreateBuffer(&vertexBufferDesc, &vertexData, &sentence->VertexBuffer);
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
		hr = device->CreateBuffer(&indexBufferDesc, &indexData, &sentence->IndexBuffer);
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

	//// make draw x and draw y match screen pixels //
	//drawX = (float)(int)(drawX+0.5f);
	//drawY = (float)(int)(drawY+0.5f);

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
	hr = devcont->Map(sentence->VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(hr)){

		return false;
	}

	// Get a pointer to the data in the vertex buffer.
	VertexType* verticesPtr = (VertexType*)mappedResource.pData;

	// Copy the data into the vertex buffer.
	memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType) * sentence->vertexCount));

	// Unlock the vertex buffer.
	devcont->Unmap(sentence->VertexBuffer, 0);

	// Release the vertex array as it is no longer needed.
	SAFE_DELETE_ARRAY(vertices);

	return true;
}
void TextRenderer::ReleaseSentence(SentenceType** sentence){
	if(*sentence){
		// Release the sentence vertex buffer.
		SAFE_RELEASE((*sentence)->VertexBuffer);
		SAFE_RELEASE((*sentence)->IndexBuffer);

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
	devcont->IASetVertexBuffers(0, 1, &sentence->VertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	devcont->IASetIndexBuffer(sentence->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

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

DLLEXPORT bool Leviathan::TextRenderer::RenderExpensiveText(ExpensiveTextRendBlob* renderptr, ID3D11DeviceContext* devcont, D3DXMATRIX worldmatrix, 
	D3DXMATRIX orthomatrix)
{
	// get right expensive text object and render it //

	// check id //
	if(renderptr->TextID < 0)
		renderptr->TextID = IDFactory::GetID();

	ExpensiveText* text = GetExpensiveText(renderptr->TextID);


	// check does text need updating //
	if(!text->UpdateIfNeeded(renderptr, this, ScreenWidth, ScreenHeight)){
		// failed to update sentence's internal buffers/textures //
		DEBUG_BREAK;
		return false;
	}


	// prepare text for rendering //
	if(!text->PrepareRender(devcont)){

		DEBUG_BREAK;
		return false;
	}

	// render text //
	return text->Render(_FontShader, this, devcont, worldmatrix, orthomatrix);
}

DLLEXPORT ExpensiveText* Leviathan::TextRenderer::GetExpensiveText(const int &ID){
	// try to find one //
	for(size_t i = 0; i < ExpensiveTexts.size(); i++){
		if(ExpensiveTexts[i]->ID == ID){
			// found right one //
			return ExpensiveTexts[i];
		}
	}

	// create new //
	ExpensiveTexts.push_back(new ExpensiveText(ID));
	return ExpensiveTexts.back();
}

DLLEXPORT bool Leviathan::TextRenderer::ReleaseExpensiveText(const int &ID){
	for(size_t i = 0; i < ExpensiveTexts.size(); i++){
		if(ExpensiveTexts[i]->ID == ID){
			// found right one //
			SAFE_DELETE(ExpensiveTexts[i]);
			ExpensiveTexts.erase(ExpensiveTexts.begin()+i);
			return true;
		}
	}
	return false;
}

DLLEXPORT bool Leviathan::TextRenderer::RenderExpensiveTextToTexture(ExpensiveText* text, const int &TextureID){
	// get right font and pass data to it //
	int index = GetFontIndex(text->Font);
	ARR_INDEX_CHECKINV(index, (int)FontHolder.size()){
		// could not load font //
		return false;
	}

	// pass data from expensive text to font //
	Int2 RenderedDimensions(0, 0);

	bool Result = true;

	if(text->AdjustedToFit){
		Result = FontHolder[index]->RenderSentenceToTexture(TextureID, text->AdjustedSize, text->AdjustedText, RenderedDimensions, 
		text->RenderedBaseline);
	} else {
		Result =  FontHolder[index]->RenderSentenceToTexture(TextureID, text->Size, text->Text, RenderedDimensions, text->RenderedBaseline);
	}

	if(text->CoordType == GUI_POSITIONABLE_COORDTYPE_RELATIVE){

		text->RenderedToBox = Float2(((float)RenderedDimensions.X)/DataStore::Get()->GetWidth(), 
			((float)RenderedDimensions.Y)/DataStore::Get()->GetHeight());
	} else {

		text->RenderedToBox = Float2((float)RenderedDimensions.X, (float)RenderedDimensions.Y);
	}
	return Result;
}

DLLEXPORT bool Leviathan::TextRenderer::AdjustTextToFitBox(const float &Size, const Float2 &BoxToFit, const wstring &text, const wstring &font, 
	int CoordType, size_t &Charindexthatfits, float &EntirelyFitModifier, float &HybridScale, Float2 &Finallength, float scaletocutfrom /*= 0.5f*/)
{
	int index = GetFontIndex(font);
	ARR_INDEX_CHECKINV(index, (int)FontHolder.size()){
		// could not load font //
		return false;
	}

	// pass to font and return what it returns //
	return FontHolder[index]->AdjustTextSizeToFitBox(Size, BoxToFit, text, CoordType, Charindexthatfits, EntirelyFitModifier, HybridScale, 
		Finallength, scaletocutfrom);
}

// ------------------ ExpensiveText ------------------ //
DLLEXPORT bool Leviathan::ExpensiveText::UpdateIfNeeded(ExpensiveTextRendBlob* renderptr, TextRenderer* render, int ScreenWidth, int ScreenHeight){
	// copy values that are always safe to change //
	Color = renderptr->Color;

	// check has something changed //
	bool TextNeedsRendering = false;
	bool BuffersNeedUpdating = false;

	// things that affect both //
	if((renderptr->CoordType != CoordType) || ((ScreenHeight != this->ScreenHeight || ScreenWidth != this->ScreenWidth) && 
		(CoordType == GUI_POSITIONABLE_COORDTYPE_RELATIVE)))
	{
		TextNeedsRendering = true;
		// needs to reposition the buffer //
		BuffersNeedUpdating = true;
		// both need already be updated so might as well skip there //
		goto checkresultstartslabel;
	}

	if((FitToBox != renderptr->FitToBox) || (FitToBox && (BoxToFit != renderptr->BoxToFit))){
		// needs to redo everything //
		TextNeedsRendering = true;
		BuffersNeedUpdating = true;

		// both need already be updated so might as well skip there //
		goto checkresultstartslabel;
	}

	// check does the text texture need to be rendered again //
	if((renderptr->Font != this->Font) || (renderptr->Text != this->Text) || (renderptr->Size != Size))
	{

		TextNeedsRendering = true;
	}

	if((renderptr->Coord != Coord)){
		// needs to reposition the buffer //
		BuffersNeedUpdating = true;
	}

	if(!BuffersNeedUpdating && !TextNeedsRendering){
		// no need to update anything //
		return true;
	}

checkresultstartslabel:

	// copy new values //
	Size = renderptr->Size;
	Coord = renderptr->Coord;
	CoordType = renderptr->CoordType;
	BoxToFit = renderptr->BoxToFit;
	FitToBox = renderptr->FitToBox;
	Text = renderptr->Text;
	Font = renderptr->Font;
	this->ScreenWidth = ScreenWidth;
	this->ScreenHeight = ScreenHeight;

	// can no longer be adjusted //
	AdjustedToFit = false;
	AdjustedSize = Size;

	if(BuffersNeedUpdating){
		// set buffers as invalid //
		BuffersFine = false;
	}

	// update what needs to be updated //
	if(FitToBox){
		// update to AdjustedSize to make this fir the box //
		AdjustToFit(render);
	}

	// check texture rendering //
	if(TextNeedsRendering || TextureID < 0){

		if(!_VerifyTextures(render)){
			// failed //
			return false;
		}
	}
	// should be all good to go //
	return true;
}

bool Leviathan::ExpensiveText::PrepareRender(ID3D11DeviceContext* devcont){
	// verify buffers //
	if(!BuffersFine){
		// check buffer states //

		if(!_VerifyBuffers(devcont)){
			// failed //
			return false;
		}

		// buffers good //
		BuffersFine = true;
	}



	// Set vertex buffer stride and offset.
	unsigned int stride, offset;
	stride = sizeof(VertexType); 
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	devcont->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	devcont->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	devcont->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	return true;
}

DLLEXPORT bool Leviathan::ExpensiveText::Render(FontShader* shader, TextRenderer* trender, ID3D11DeviceContext* devcont, D3DXMATRIX worldmatrix, D3DXMATRIX orthomatrix){
	// Render the text using the font shader.
	ManagedTexture* tmptexture = Graphics::Get()->GetTextureManager()->GetTexture(TextureID, TEXTUREMANAGER_SEARCH_VOLATILEGENERATED, true);

	if(!tmptexture){
		// textures aren't generated //
trytoveerifyexpensivetexttextureslabel:

		if(!_VerifyTextures(trender)){
			// this should now be marked as invalid //
			DEBUG_BREAK;
		} else {
			// textures should now be fine //
			// recurse
			return Render(shader, trender, devcont, worldmatrix, orthomatrix);
		}
		return false;
	}

	// get resource //
	ID3D11ShaderResourceView* texture = tmptexture->GetView();

	if(!texture){
		// texture needs to be re created //
		goto trytoveerifyexpensivetexttextureslabel;
	}

	return shader->Render(devcont, 6, worldmatrix, trender->BaseViewMatrix, orthomatrix, texture, Color);
}

DLLEXPORT void Leviathan::ExpensiveText::AdjustToFit(TextRenderer* trenderer, bool trytocenter /*= false*/){
	// this text needs to be adjusted to fit the box that is passed into the class //
	// use the new function to determine what could be done //
	size_t Charindexthatfits = 0;
	float EntirelyFitModifier = 1;
	float HybridScale = 1;
	Float2 Finalsize = (Float2)0;

	if(!trenderer->AdjustTextToFitBox(Size, BoxToFit, Text, Font, CoordType, Charindexthatfits, EntirelyFitModifier, HybridScale, Finalsize, 0.4f)){

		DEBUG_BREAK;
		return;
	}
	// determine what is the best way to continue //
	AdjustedToFit = true;

	// set to the hybrid scale anyways //
	AdjustedSize = HybridScale;
	//AdjustedSize = 4;

	// add dots if all didn't fit //
	if(Charindexthatfits != Text.size()-1){
		// all didn't fit, so we need to cut it //
		AdjustedText = Text.substr(0, Charindexthatfits+1)+L"...";
	} else {
		// everything will fit //
		AdjustedText = Text;
	}

	if(trytocenter){
		// we will need to use FinalSize here to determine how much to add to start x and y to have the text centered //
		DEBUG_BREAK;

	}

}

bool Leviathan::ExpensiveText::_VerifyTextures(TextRenderer* trender){

	float ScaleUsed = Size;
	wstring& TextToRender = Text;

	if(AdjustedToFit){
		// use adjusted values //
		ScaleUsed = AdjustedSize;
		TextToRender = AdjustedText;
	}

	// if we are missing texture id create new or if we already have one release it //
	if(TextureID >= 0){
		// tell texture manager to ditch the old one //
		Graphics::Get()->GetTextureManager()->UnloadVolatile(TextureID);
	}

	TextureID = IDFactory::GetID();


	// use TextRenderer to render this to texture //
	if(!trender->RenderExpensiveTextToTexture(this, TextureID)){

		Logger::Get()->Error(L"ExpensiveText: VerifyTextures: failed to render text to texture");
		return false;
	}


	// textures are successfully rendered //
	return true;
}

bool Leviathan::ExpensiveText::_VerifyBuffers(ID3D11DeviceContext* devcont){

	// check index buffer state, which is always same //
	if(!IndexBuffer){
		// this just needs 6 element default vertex buffer //
		IndexBuffer = Rendering::ResourceCreator::GenerateDefaultIndexBuffer(6);
	}


	// vertex buffer needs some more work //
	if(!VertexBuffer){
		// create new from scratch //
		VertexBuffer = Rendering::ResourceCreator::GenerateDefaultDynamicDefaultTypeVertexBuffer(6);
	}


	// update current vertex buffer //
	D3D11_MAPPED_SUBRESOURCE VBufferData;

	HRESULT hr = devcont->Map(VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &VBufferData);
	if(FAILED(hr)){

		return false;
	}

	// cast to right type //
	VertexType* VertexDataPtr = (VertexType*)VBufferData.pData;

	// calculate the data that needs to be passed //

	// convert data //
	VertexType* vertices = Rendering::ResourceCreator::GenerateQuadIntoVertexBuffer(Coord, RenderedToBox, 6, CoordType, 
		QUAD_FILLSTYLE_UPPERLEFT_0_BOTTOMRIGHT_1);
	if(!vertices){
		return false;
	}

	// copy data to the mapped buffer //
	memcpy(VertexDataPtr, (void*)vertices, sizeof(VertexType)*6);

	// Unmap the buffer //
	devcont->Unmap(VertexBuffer, 0);

	// release vertice data //
	SAFE_DELETE_ARRAY(vertices);


	// buffers are now fine //
	return true;
}
