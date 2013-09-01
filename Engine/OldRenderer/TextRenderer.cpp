#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_2DTEXTRENDERER
#include "TextRenderer.h"
#endif
using namespace Leviathan;
using namespace Rendering;
// ------------------------------------ //
#include "3DRenderer.h"
#include "Graphics.h"
#include "..\GuiPositionable.h"
#include "..\DebugVariableNotifier.h"
#include "RenderingResourceCreator.h"

TextRenderer::TextRenderer() : Graph(NULL){

}

TextRenderer::~TextRenderer(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::TextRenderer::Init(Graphics* graph){
	// store Graphics object ptr //
	Graph = graph;

	// always create arial font //
	LoadFont(L"Arial.dds");

	return true;
}

void TextRenderer::Release(){
	// release expensive text //
	SAFE_DELETE_VECTOR(ExpensiveTexts);
	SAFE_DELETE_VECTOR(CheapTexts);

	
	// release fonts //
	for(unsigned int i = 0; i < FontHolder.size(); i++){
		SAFE_RELEASEDEL(FontHolder[i]);
	}
	FontHolder.clear();
}

DLLEXPORT bool Leviathan::TextRenderer::ReleaseText(const int &ID){
	// we need to check both vectors and delete if we find matching id //
	for(size_t i = 0; i < CheapTexts.size(); i++){
		if(CheapTexts[i]->ID == ID){

			SAFE_RELEASEDEL(CheapTexts[i]);
			CheapTexts.erase(CheapTexts.begin()+i);
			return true;
		}
	}

	for(size_t i = 0; i < ExpensiveTexts.size(); i++){
		if(ExpensiveTexts[i]->ID == ID){

			SAFE_RELEASEDEL(ExpensiveTexts[i]);
			ExpensiveTexts.erase(ExpensiveTexts.begin()+i);
			return true;
		}
	}

	return false;
}

// ------------------------------------ //
DLLEXPORT float Leviathan::TextRenderer::CountSentenceLength(const wstring &sentence, const wstring &font, bool expensive, float heightmod, int coordtype){
	int index = GetFontIndex(font);
	ARR_INDEX_CHECK(index, (int)FontHolder.size()){
		if(!expensive)
			return FontHolder[index]->CountLengthNonExpensive(sentence, heightmod, coordtype);
		return FontHolder[index]->CountLengthExpensive(sentence, heightmod, coordtype);
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

DLLEXPORT RenderingFont* Leviathan::TextRenderer::GetFontFromName(const wstring &name){
	int index = GetFontIndex(name);
	ARR_INDEX_CHECKINV(index, (int)FontHolder.size()){
		// could not load font //
		return NULL;
	}
	return FontHolder[index];
}
// ------------------------------------ //
bool Leviathan::TextRenderer::LoadFont(const wstring &file){
	// add new and initialize //
	FontHolder.push_back(new RenderingFont());
	if(!FontHolder.back()->Init(Graph->GetRenderer()->GetDevice(), file)){

		Logger::Get()->Error(L"TextRenderer: LoadFont: failed to initialize, font: "+file);
		return false;
	}

	return true;
}

int Leviathan::TextRenderer::GetFontIndex(const wstring &name){
	// try to get index //
	for(unsigned int i = 0; i < FontHolder.size(); i++){
		if(FontHolder[i]->GetName() == name){
			return i;
		}
	}
	// not found, try to load font //
	const wstring File = name+L".dds";
	if(!LoadFont(File)){
		return -1;
	}

	Logger::Get()->Info(L"TextRenderer: GetFontIndex: new font loaded! : "+name, true);
	// return the new font from the last index //
	return FontHolder.size()-1;
}
// ------------------------------------ //
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
	// rendered to box is now different; vertex buffer needs updating //
	text->BuffersFine = false;

	return Result;
}

DLLEXPORT bool Leviathan::TextRenderer::AdjustTextToFitBox(const Float2 &BoxToFit, const wstring &text, const wstring &font, int CoordType, 
	size_t &Charindexthatfits, float &EntirelyFitModifier, float &HybridScale, Float2 &Finallength, float scaletocutfrom /*= 0.5f*/)
{
	int index = GetFontIndex(font);
	ARR_INDEX_CHECKINV(index, (int)FontHolder.size()){
		// could not load font //
		return false;
	}

	// pass to font and return what it returns //
	return FontHolder[index]->AdjustTextSizeToFitBox(BoxToFit, text, CoordType, Charindexthatfits, EntirelyFitModifier, HybridScale, 
		Finallength, scaletocutfrom);
}

DLLEXPORT ExpensiveText* Leviathan::TextRenderer::GetExpensiveText(const int &ID){
	for(size_t i = 0; i < ExpensiveTexts.size(); i++){
		if(ExpensiveTexts[i]->ID == ID)
			return ExpensiveTexts[i];
	}
	// create new //
	ExpensiveTexts.push_back(new ExpensiveText(ID));
	// init it's buffers //
	ExpensiveTexts.back()->Init(Graph->GetRenderer()->GetDevice());
	return ExpensiveTexts.back();
}

DLLEXPORT CheapText* Leviathan::TextRenderer::GetCheapText(const int &ID){
	for(size_t i = 0; i < CheapTexts.size(); i++){
		if(CheapTexts[i]->ID == ID)
			return CheapTexts[i];
	}
	// create new //
	CheapTexts.push_back(new CheapText(ID));
	// init it's buffers //
	CheapTexts.back()->Init(Graph->GetRenderer()->GetDevice());
	return CheapTexts.back();
}

DLLEXPORT void Leviathan::TextRenderer::UpdateShaderRenderTask(int TextID, ShaderRenderTask* SHRender, RenderingPassInfo* pass){

	for(size_t i = 0; i < CheapTexts.size(); i++){
		if(CheapTexts[i]->ID == TextID){
			CheapTexts[i]->SetRenderingVariablesToSH(this, SHRender);
			return;
		}
	}

	for(size_t i = 0; i < ExpensiveTexts.size(); i++){
		if(ExpensiveTexts[i]->ID == TextID){
			ExpensiveTexts[i]->SetRenderingVariablesToSH(this, SHRender);
			return;
		}
	}

	
}

DLLEXPORT Rendering::BaseRenderableBufferContainer* Leviathan::TextRenderer::GetRenderingBuffers(int TextID, ShaderRenderTask* SHRender){
	for(size_t i = 0; i < CheapTexts.size(); i++){
		if(CheapTexts[i]->ID == TextID)
			return CheapTexts[i];
	}

	for(size_t i = 0; i < ExpensiveTexts.size(); i++){
		if(ExpensiveTexts[i]->ID == TextID)
			return ExpensiveTexts[i];
	}

	return NULL;
}

// ------------------ ExpensiveText ------------------ //
DLLEXPORT bool Leviathan::ExpensiveText::UpdateIfNeeded(TextRenderer* render, const wstring &text, const wstring &font, const Float2 &location, 
	int coordtype, float size, float adjustcut, const Float2 &boxtofit, bool fittobox, int screenwidth, int screenheight)
{
	// check has something changed //
	bool TextNeedsRendering = false;
	bool BuffersNeedUpdating = false;

	// things that affect both //
	if(coordtype != CoordType || (screenheight != ScreenHeight || screenwidth != ScreenWidth)){

		TextNeedsRendering = true;
		// needs to reposition the buffer //
		BuffersNeedUpdating = true;
		// both need already be updated so might as well skip there //
		goto checkresultstartslabel;
	}

	if(FitToBox != fittobox || BoxToFit != boxtofit){
		// needs to redo everything //
		TextNeedsRendering = true;
		BuffersNeedUpdating = true;

		// both need already be updated so might as well skip there //
		goto checkresultstartslabel;
	}

	// check does the text texture need to be rendered again //
	if(font != Font || text != Text || size != Size || AdjustCutModifier != adjustcut){

		TextNeedsRendering = true;
	}

	if(Coord != location){
		// needs to reposition the buffer //
		BuffersNeedUpdating = true;
	}

	if(!BuffersNeedUpdating && !TextNeedsRendering){
		// no need to update anything //
		return true;
	}

checkresultstartslabel:

	// copy new values //
	Size = size;
	Coord = location;
	CoordType = coordtype;
	BoxToFit = boxtofit;
	FitToBox = fittobox;
	Text = text;
	Font = font;
	ScreenWidth = screenwidth;
	ScreenHeight = screenheight;
	AdjustCutModifier = adjustcut;
	// can no longer be adjusted //
	AdjustedToFit = false;
	AdjustedSize = Size;

	if(BuffersNeedUpdating){
		// set buffers as invalid //
		BuffersFine = false;
	}

	// update what needs to be updated //
	if(FitToBox){
		// update to AdjustedSize to make this fit the box //
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

DLLEXPORT void Leviathan::ExpensiveText::AdjustToFit(TextRenderer* trenderer, bool trytocenter /*= false*/){
	// this text needs to be adjusted to fit the box that is passed into the class //
	// use the new function to determine what could be done //
	size_t Charindexthatfits = 0;
	float EntirelyFitModifier = 1;
	float HybridScale = 1;
	Float2 Finalsize = (Float2)0;

	if(!trenderer->AdjustTextToFitBox(BoxToFit, Text, Font, CoordType, Charindexthatfits, EntirelyFitModifier, HybridScale, Finalsize, 
		AdjustCutModifier))
	{
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
	// we can return if buffers are fine //
	if(BuffersFine)
		return true;
	BuffersFine = true;

	// update current vertex buffer //

	auto AutoUnlocked = ResourceCreator::MapConstantBufferForWriting<VertexType>(devcont, VertexBuffer);

	// create new data for buffer //
	unique_ptr<VertexType[]> vertices(ResourceCreator::GenerateQuadIntoVertexBuffer(Coord, RenderedToBox, 6, CoordType, 
		QUAD_FILLSTYLE_UPPERLEFT_0_BOTTOMRIGHT_1));
	if(!vertices || AutoUnlocked.get() == NULL){
		return false;
	}

	// copy data to the mapped buffer //
	memcpy(AutoUnlocked->LockedResourcePtr, vertices.get(), sizeof(VertexType)*6);


	// buffers are now fine //
	return true;
}

DLLEXPORT bool Leviathan::ExpensiveText::SetBuffersForRendering(ID3D11DeviceContext* devcont, int &indexbuffersize){
	if(!BuffersFine){
		// check buffer states //
		if(!_VerifyBuffers(devcont)){
			// failed //
			return false;
		}

		// buffers good //
		BuffersFine = true;
	}

	indexbuffersize = GetIndexCount();
	return RenderBuffers(devcont);
}

DLLEXPORT bool Leviathan::ExpensiveText::SetRenderingVariablesToSH(TextRenderer* trenderer, ShaderRenderTask* task){
	// set parameters for rendering this text //
	shared_ptr<ManagedTexture> tmptexture = Graphics::Get()->GetTextureManager()->GetTexture(TextureID, TEXTUREMANAGER_SEARCH_VOLATILEGENERATED, true);
	// check is texture NULL (which is very rare) or if it is the error texture for getting the text unloaded //
	if(!tmptexture.get() || trenderer->GetOwningGraphics()->GetTextureManager()->IsTextureError(tmptexture.get())){
		// textures aren't generated //
trytoveerifyexpensivetexttextureslabel:

		if(!_VerifyTextures(trenderer)){
			// this should now be marked as invalid //
			DEBUG_BREAK;
		} else {
			// textures should now be fine //
			// recurse
			return SetRenderingVariablesToSH(trenderer, task);
		}
		return false;
	}
	// get resource //
	ID3D11ShaderResourceView* texture = tmptexture->GetView();

	if(!texture){
		// texture needs to be re created //
		goto trytoveerifyexpensivetexttextureslabel;
	}

	// set parameters for shader //
	if(!task->GetBaseTextureHolder() ||task->GetBaseTextureHolder()->TextureCount != 1){
		// new texture holder //
		task->SetTextures(new SingleTextureHolder(tmptexture));
	} else {
		// set texture //
		static_cast<SingleTextureHolder*>(task->GetBaseTextureHolder())->Texture1 = tmptexture;
	}

	return true;
}

bool Leviathan::ExpensiveText::CreateBuffers(ID3D11Device* device){
	// buffers are always the same size for this text, we can create them here //

	// this just needs 6 element default vertex index //
	IndexBuffer = Rendering::ResourceCreator::GenerateDefaultIndexBuffer(6);

	// dynamic vertex buffer for updating location //
	VertexBuffer = Rendering::ResourceCreator::GenerateDefaultDynamicDefaultTypeVertexBuffer(6);


	BuffersFine = false;
	return true;
}

// ------------------ CheapText ------------------ //
Leviathan::CheapText::CheapText(const int &id) : BaseRenderableBufferContainer("C0:T0", sizeof(VertexType)), ID(id){
	// set max character count to 0 to force updating on first Update call //
	MaxTextLength = 0;

}

Leviathan::CheapText::~CheapText(){
	// release buffers //
	Release();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::CheapText::Update(TextRenderer* render, const wstring &text, const wstring &font, const Float2 &pos, int coordtype, 
	float sizemodifier, int screenwidth, int screenheight)
{
	// return if none are updated //
	if(Text == text && Font == font && Position == pos && CoordType == coordtype && SizeModifier == sizemodifier && ScreenWidth == screenwidth &&
		ScreenHeight == screenheight)
	{
		return true;
	}

	// check for too long string //
	if((int)text.length() > MaxTextLength || MaxTextLength == 0){
		// needs to recreate buffers //
		SAFE_RELEASE(VertexBuffer);
		SAFE_RELEASE(IndexBuffer);

		// update max length //
		MaxTextLength = text.size()*2;
		VertexCount = 6 * MaxTextLength;
	}

	// copy new values //
	Text = text;
	Position = pos;
	CoordType = coordtype;
	SizeModifier = sizemodifier;
	ScreenWidth = screenwidth;
	ScreenHeight = screenheight;



	// generate buffers if they haven't been created already //
	IndexBuffer = Rendering::ResourceCreator::GenerateDefaultIndexBuffer(VertexCount);
	// dynamic buffer for updating texture coordinates //
	VertexBuffer = Rendering::ResourceCreator::GenerateDefaultDynamicDefaultTypeVertexBuffer(VertexCount);


	// create new vertex array //
	unique_ptr<VertexType[]> vertices(new VertexType[VertexCount]);
	ZeroMemory(vertices.get(), sizeof(VertexType)*VertexCount);

	// calculate where to draw //
	float drawX = 0;
	float drawY = 0;
	if(CoordType == GUI_POSITIONABLE_COORDTYPE_RELATIVE){
		// pos is value between 0 - 1 scale to screen size //
		drawX = ((((float)ScreenWidth / 2) * -1) + ScreenWidth*Position.X);
		drawY = ((((float)ScreenHeight / 2)) - ScreenHeight*Position.Y);

	} else {
		drawX = ((((float)ScreenWidth / 2) * -1) + Position.X);
		drawY = (((float)ScreenHeight / 2) - Position.Y);
	}
	
	// use font to build vertex array //
	if(!render->GetFontFromName(Font)->BuildVertexArray(vertices.get(), text, drawX, drawY, SizeModifier)){
		// sentence probably has invalid characters //
		return false;
	}
	if(!vertices){
		// font has failed //
		Logger::Get()->Error(L"Render sentence has failed");
		return false;
	}

	// lock vertex buffer for writing //
	auto AutoUnlocker = Rendering::ResourceCreator::MapConstantBufferForWriting<VertexType>(render->GetOwningGraphics()->GetRenderer()->
		GetDeviceContext(), VertexBuffer);
	if(AutoUnlocker.get() == NULL){
		// failed to lock buffer //
		return false;
	}
	// copy data to the actual buffer //
	memcpy(AutoUnlocker->LockedResourcePtr, (void*)vertices.get(), sizeof(VertexType)*VertexCount);

	return true;
}

bool Leviathan::CheapText::CreateBuffers(ID3D11Device* device){
	// we actually cannot create buffers here //
	return true;
}

DLLEXPORT bool Leviathan::CheapText::SetRenderingVariablesToSH(TextRenderer* trenderer, ShaderRenderTask* task){
	// get font's texture //
	shared_ptr<ManagedTexture> texture = trenderer->GetFontFromName(Font)->GetTexture();

	// set parameters for shader //
	if(!task->GetBaseTextureHolder() ||task->GetBaseTextureHolder()->TextureCount != 1){
		// new texture holder //
		task->SetTextures(new SingleTextureHolder(texture));
	} else {
		// set texture //
		static_cast<SingleTextureHolder*>(task->GetBaseTextureHolder())->Texture1 = texture;
	}

	return true;
}
