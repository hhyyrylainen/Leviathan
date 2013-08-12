#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_FONT
#include "RenderingFont.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "DDsHandler.h"

#include "FileSystem.h"
#include "..\DataStore.h"
#include "Graphics.h"
#include "..\ExceptionInvalidType.h"
#include "..\DebugVariableNotifier.h"

RenderingFont::RenderingFont() : Texture(NULL), FontsFace(NULL), FontData(){
	// we need to increase instance count //
	boost::lock_guard<boost::mutex> guard(LivingStaticMutex);
	LivingObjects++;
}
RenderingFont::~RenderingFont(){
	// we need to decrease instance count //
	boost::lock_guard<boost::mutex> guard(LivingStaticMutex);
	LivingObjects--;

	if(LivingObjects <= 0){
		// we can unload FreeType //
		FreeTypeLoaded = false;
		FT_Done_FreeType(FreeTypeLibrary);
		FreeTypeLibrary = NULL;
	}
}

bool RenderingFont::FreeTypeLoaded = false;
FT_Library RenderingFont::FreeTypeLibrary = NULL;
boost::mutex Leviathan::RenderingFont::LivingStaticMutex;
int Leviathan::RenderingFont::LivingObjects = 0;

// ------------------------------------ //
DLLEXPORT bool Leviathan::RenderingFont::Init(ID3D11Device* dev, const wstring &FontFile){
	// get name from the filename //
	Name = FileSystem::RemoveExtension(FontFile, true);

	wstring completepath = FileSystem::GetFontFolder()+FontFile;

	// load texture //
	if(!LoadTexture(dev, completepath)){
		Logger::Get()->Error(L"RenderingFont: Init: texture loading failed, file: "+FontFile);
		return false;
	}

	// load character data //
	if(!LoadFontData(dev, completepath)){
		Logger::Get()->Error(L"RenderingFont: Init: could not load font data, file: "+FontFile, true);
		return false;
	}
	// succeeded //
	return true;
}

void RenderingFont::Release(){
	// release FreeType objects //
	if(FontsFace)
		FT_Done_Face(FontsFace);

	Texture->UnLoad(true);
	SAFE_DELETE_VECTOR(FontData);
}
// ------------------------------------ //
bool Leviathan::RenderingFont::LoadFontData(ID3D11Device* dev, const wstring &file){
	// is data already in memory? //
	if(FontData.size() > 0){
		// it is //
		return true;
	}

	// check is there font data //
	wstring texturedatafile = FileSystem::ChangeExtension(file, L"levfd");
	if(!FileSystem::FileExists(texturedatafile)){
		Logger::Get()->Info(L"Font data file doesn't exist, creating..., regenerating texture", true);
		// create data since it doesn't exist //
		if(!LoadTexture(dev, file, true)){
			Logger::Get()->Error(L"LoadFontData: no data file found and generating it from font file failed", true);
			return false;
		}
		// all data should now be properly loaded //
		return true;
	}
	// load data from file //
	wifstream reader;
	reader.open(texturedatafile);
	if(!reader.is_open())
		return false;

	// read in height //
	reader >> FontHeight;
	// character count //
	int DataToRead = 0;
	reader >> DataToRead;
	// resize to have enough space at once //
	FontData.resize(DataToRead, NULL);

	for(int i = 0; (i < DataToRead) && reader.good(); i++){
		// create object for loading //
		int CCode = 0;
		UINT GlyphIndex = 0;

		// read data //
		reader >> CCode;
		reader >> GlyphIndex;

		if(CCode == 0){
			// invalid line //
			Logger::Get()->Error(L"RenderingFont: LoadFontData: invalid levfd data, file: "+file+L"number: "+Convert::ToWstring(i+1));
			continue;
		}

		// create new instance //
		unique_ptr<FontsCharacter> Curload = unique_ptr<FontsCharacter>(new FontsCharacter(CCode, GlyphIndex));

		// load rest of data to the new instance //
		reader >> Curload->PixelWidth;
		reader >> Curload->AdvancePixels;
		// texture coordinates //
		reader >> Curload->TopLeft.X;
		reader >> Curload->TopLeft.Y;
		reader >> Curload->BottomRight.X;
		reader >> Curload->BottomRight.Y;
		// set to right spot //
		size_t spot = ConvertCharCodeToIndex(CCode);
		if(FontData[spot] != NULL){
			// that's an error //
			Logger::Get()->Error(L"RenderingFont: LoadFontData: data has multiple characters with charcode: "+Convert::ToWstring(CCode)+L", file: "
				+file);
			SAFE_DELETE(FontData[spot]);
		}
		// set //
		FontData[spot] = Curload.release();
	}
	reader.close();

	// this is still needed with cheap text for kerning data //
	return _VerifyFontFTDataLoaded();
}
// ------------------------------------ //
bool Leviathan::RenderingFont::LoadTexture(ID3D11Device* dev, const wstring &file, bool forcegen /*= false*/){
	// check does file exist //
	if(forcegen || !FileSystem::FileExists(file)){
		Logger::Get()->Info(L"No font texture found: generating...", false);
		// try to generate one //
		if(!_VerifyFontFTDataLoaded()){
			Logger::Get()->Error(L"RenderingFont: LoadTexture: failed to load FreeType face, cannot generate file: "+file, true);
			return false;
		}
		// multiplier that increases generated fonts size //
		int fontsizemultiplier = DataStore::Get()->GetFontSizeMultiplier();

		FT_Error errorcode = FT_Set_Pixel_Sizes(FontsFace, 0, FONT_BASEPIXELHEIGHT*fontsizemultiplier);
		if(errorcode){
			Logger::Get()->Error(L"FontGenerator: FreeType: set pixel size failed", true);
			return false;
		}

		// height "should" be forced to be this //
		int Height = FontHeight = CalculatePixelSizeAtScale((float)fontsizemultiplier)+2;
		int baseline = Height-((int)floorf(Height/3.f));

		// "image" //
		ScaleableFreeTypeBitmap FinishedImage(512, 512);

		// reserve data in the font data holder //
		FontData.resize(RENDERINGFONT_CHARCOUNT, NULL);

		// shortcut to glyph //
		FT_GlyphSlot slot = FontsFace->glyph;

		// vector that marks which characters are already on a row //
		vector<bool> FittedCharacters(FontData.size(), false);

		bool done = false;
		const int RowMaxWidth = 512;
		int FillingRow = 0;
		int CurrentRowLength = 0;
		int RowX = 0;
		// loop until all characters have been rendered //
		while(!done){
			// flags that are used to determine what to do after looping through characters //
			bool Fitted = false;
			bool CheckedAny = false;

			// calculate start y for this row //
			const int RowY = FillingRow*FontHeight;

			// should go from 32 to RENDERINGFONT_MAXCHARCODE
			for(size_t i = 0; i < FontData.size(); i++){
				if(FittedCharacters[i])
					continue;

				// at least checking a character //
				CheckedAny = true;

				wchar_t chartolook = (wchar_t)(i+32);

				// check do we need to generate a new object //
				if(FontData[i] == NULL){
					// create new instance, with getting index of glyph //
					unique_ptr<FontsCharacter> CurChar(new FontsCharacter(chartolook, FT_Get_Char_Index(FontsFace, chartolook)));
					// generation specific data //
					CurChar->Generating = new GeneratingDataForCharacter();

					// load glyph for metrics //
					errorcode = FT_Load_Glyph(FontsFace, CurChar->CharacterGlyphIndex, FT_LOAD_DEFAULT);
					if(errorcode){
						goto glyphprocesserrorlabel;
					}
					// copy the bitmap //
					errorcode = FT_Get_Glyph(slot, &CurChar->Generating->ThisRendered);
					if(errorcode){
						goto glyphprocesserrorlabel;
					}

					// space needs special treatment //
					if(chartolook == L' '){
						// we need to make up stuff about space character //
						CurChar->PixelWidth = 6;
						CurChar->AdvancePixels = 6;

					} else {
						// set various things //
						CurChar->PixelWidth = slot->advance.x >> 6;
						CurChar->AdvancePixels = slot->advance.x >> 6;
					}

					// set instance pointer //
					FontData[i] = CurChar.release();
				}
				// accessibility ptr //
				FontsCharacter* CurChar = FontData[i];
				// now we can try to fit this character somewhere //
				// check can it fit //
				if(CurrentRowLength+FontData[i]->PixelWidth+1 > RowMaxWidth){
					// can not fit //
					continue;
				}
				// it can fit, render it here //
				// render bitmap //
				errorcode = FT_Glyph_To_Bitmap(&CurChar->Generating->ThisRendered, FT_RENDER_MODE_NORMAL, NULL, true);
				if(errorcode){
					goto glyphprocesserrorlabel;
				}
				// get bitmap //
				FT_Bitmap& charimg = ((FT_BitmapGlyph)CurChar->Generating->ThisRendered)->bitmap;

				// store important data for rendering //
				CurChar->Generating->RenderedTop = ((FT_BitmapGlyph)CurChar->Generating->ThisRendered)->top;
				CurChar->Generating->RenderedLeft = ((FT_BitmapGlyph)CurChar->Generating->ThisRendered)->left;

				// calculate copy position //
				int BitmapPosX = RowX+CurChar->Generating->RenderedLeft;
				int BitmapPosY = RowY+abs(CurChar->Generating->RenderedTop-baseline);

				if(BitmapPosY < RowY){
					// something is probably wrong //
					//DEBUG_BREAK;
				}

				// copy bitmap //
				FinishedImage.RenderFTBitmapIntoThis(BitmapPosX, BitmapPosY, charimg);
				// set font data's texture coordinate positions as pixel positions since we don't know final size //
				FontData[i]->TopLeft = Float2((float)RowX, (float)RowY);
				FontData[i]->BottomRight = Float2((float)(RowX+CurChar->AdvancePixels), (float)(RowY+FontHeight-1));

				// increment StartX according to width //
				RowX += CurChar->AdvancePixels+1;
				CurrentRowLength = RowX+1;
				// set as added and set that a character (at least one) fit to the current row //
				FittedCharacters[i] = Fitted = true;

				continue;
				// fail check //
glyphprocesserrorlabel:
				DEBUG_BREAK;
				Logger::Get()->Error(L"FontGenerator: FreeType: action failed, on glyph "+Convert::ToWstring(i+32));
				// "fake" that this has been added //
				FittedCharacters[i] = true;
				continue;
			}
			// checks //
			if(!CheckedAny){
				// we are done //
				done = true;
				break;
			}
			if(!Fitted){
				// no character could fit to this row, move to next //
				FillingRow++;
				CurrentRowLength = 0;
				RowX = 0;
			}
		}
		// font data is now almost done //
		// make sure that width and height are dividable by 2 //
		FinishedImage.MakeSizeDividableBy2();

		// update the main image //
		FinishedImage.UpdateStats();

		int RImgWidth = FinishedImage.GetWidth();
		int RImgHeight = FinishedImage.GetHeight();

		// now that image is actually finished we can calculate actual texture coordinates //
		for(size_t i = 0; i < FontData.size(); i++){
			// calculate final texture coordinates //
			FontData[i]->TopLeft = Float2(FontData[i]->TopLeft.X/RImgWidth, FontData[i]->TopLeft.Y/RImgHeight);
			FontData[i]->BottomRight = Float2(FontData[i]->BottomRight.X/RImgWidth, FontData[i]->BottomRight.Y/RImgHeight);

			// release data //
			FT_Done_Glyph(FontData[i]->Generating->ThisRendered);
			SAFE_DELETE(FontData[i]->Generating);
		}
		// save FontData now that it has everything filled out //
		WriteDataToFile();

		// master image is now ready to be generated into a DDS bitmap //
		size_t mimgbuffersize = 0;
		int junk = 0;
		// use bitmap's function to create a DDS //
		char* MainImageBuffer = FinishedImage.GenerateDDSToMemory(mimgbuffersize, junk);
		// file name should always be this //
		wstring file = FileSystem::GetFontFolder()+Name+L".dds";
		Logger::Get()->Info(L"RenderingFont: LoadTexture: writing texture file, file: "+file);

		ofstream writer;
		// remember to open in binary mode or things will break //
		writer.open(Convert::WstringToString(file), ios::binary);
		if(!writer.is_open()){
			// error //
			Logger::Get()->Error(L"RenderingFont: LoadTexture: failed to write font texture to file: "+file);
			SAFE_DELETE(MainImageBuffer);
			return false;
		}

		// save the DDS file to actual file //
		writer.write(MainImageBuffer, mimgbuffersize);
		writer.close();

		Logger::Get()->Info(L"RenderingFont: LoadTexture: successfully generated texture file: "+file);
	}

	Texture = shared_ptr<ManagedTexture>(new ManagedTexture(FileSystem::GetFontFolder()+Name+L".dds", IDFactory::GetID(), TEXTURETYPE_TEXT));
	
	if(!Texture->Load(dev)){

		Logger::Get()->Error(L"RenderingFont: LoadTexture: failed to load texture file");
		return false;
	}

	return true;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::RenderingFont::BuildVertexArray(VertexType* vertexptr, const wstring &text, float drawx, float drawy, float textmodifier){
	// set right size for kerning //
	const UINT heightpixels = CalculatePixelSizeAtScale(textmodifier);
	EnsurePixelSize(heightpixels);

	// index inside vertexptr array //
	size_t index = 0;
	assert(FontData.size() && "non initialized font in BuildVertexArray");

	bool kerningcheck = FT_HAS_KERNING(FontsFace) != 0;

	// draw letters to vertices //
	for(size_t i = 0; i < text.size(); i++){
		// whitespace check //
		if(text[i] < L' '){
			// just whitespace, jump over //
			drawx += 3.0f*textmodifier;

		} else {
			const size_t letterindex = ConvertCharCodeToIndex(text[i]);

			// add kerning //
			if(kerningcheck){
				// fetch kerning, if not first character //
				if(i > 0){
					// change render position according to kerning distance //
					drawx += GetKerningBetweenCharacters(textmodifier, text[i-1], text[i]);
				}
			}

			// first triangle //
			// top left //
			vertexptr[index].position = D3DXVECTOR3(drawx, drawy, 0.0f); 
			vertexptr[index].texture = FontData[letterindex]->TopLeft;
			index++;
			// bottom right //
			vertexptr[index].position = D3DXVECTOR3(drawx+(FontData[letterindex]->PixelWidth*textmodifier), drawy-(FontHeight*textmodifier), 0.0f); 
			vertexptr[index].texture = FontData[letterindex]->BottomRight;
			index++;
			// bottom left //
			vertexptr[index].position = D3DXVECTOR3(drawx, drawy-(FontHeight*textmodifier), 0.0f); 
			vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex]->TopLeft.X, FontData[letterindex]->BottomRight.Y);
			index++;
			// second triangle //
			// top left //
			vertexptr[index].position = D3DXVECTOR3(drawx, drawy, 0.0f); 
			vertexptr[index].texture = FontData[letterindex]->TopLeft;
			index++;
			// top right //
			vertexptr[index].position = D3DXVECTOR3(drawx+(FontData[letterindex]->PixelWidth*textmodifier) , drawy, 0.0f); 
			vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex]->BottomRight.X, FontData[letterindex]->TopLeft.Y);
			index++;
			// bottom right //
			vertexptr[index].position = D3DXVECTOR3(drawx+(FontData[letterindex]->PixelWidth*textmodifier), drawy-(FontHeight*textmodifier), 0.0f); 
			vertexptr[index].texture = FontData[letterindex]->BottomRight;
			index++;

			// update location //
			drawx += FontData[letterindex]->AdvancePixels*textmodifier;
		}
	}
	// succeeded in constructing the vertex array //
	return true;
}

DLLEXPORT bool Leviathan::RenderingFont::RenderSentenceToTexture(const int &TextureID, const float &sizemodifier, const wstring &text, 
	Int2 &RenderedToBox, int &baselinefromimagetop)
{
	// first we need to calculate how large the bitmap is going to be //
	// open FT data //
	if(!_VerifyFontFTDataLoaded()){
		// can't do anything //
		return false;
	}

	// set right size //
	const int heightpixels = CalculatePixelSizeAtScale(sizemodifier);
	EnsurePixelSize(heightpixels);

	// create bitmap matching "size" //
	ScaleableFreeTypeBitmap bitmap((int)(text.size()*17*sizemodifier), heightpixels);

	int PenPosition = 0;

	// set base line to be 1/3 from the bottom (actually the top of the bitmap's coordinates) //
	int baseline = heightpixels-(int)floorf(heightpixels/3.f);
	
	bitmap.SetBaseLine(baseline);

	FT_GlyphSlot slot = FontsFace->glyph;

	bool kerningcheck = FT_HAS_KERNING(FontsFace) != 0;

	// fill it with data //
	for(size_t i = 0; i < text.size(); i++){
		if(text[i] < 32){
			// whitespace //
			PenPosition += (int)(3.f*sizemodifier);
			continue;
		}
		// apply kerning //
		if(kerningcheck){
			// fetch kerning, if not first character //
			if(i > 0){
				// change pen position according to kerning distance //
				PenPosition += (int)(GetKerningBetweenCharacters(sizemodifier, text[i-1], text[i]));
			}
		}

		// load glyph //
		FT_Error errorcode = FT_Load_Char(FontsFace, text[i], FT_LOAD_RENDER);
		if(errorcode){
			Logger::Get()->Error(L"RenderSentenceToTexture: FreeType: failed to load glyph "+text[i]);
			continue;
		}

		// get the bitmap //
		FT_Bitmap& charimg = slot->bitmap;

		int BitmapPosX = PenPosition+slot->bitmap_left;
		//int BitmapPosY = abs(baseline-slot->bitmap_top);
		int BitmapPosY = baseline-slot->bitmap_top;

		// render the bitmap to the result bitmap //
		bitmap.RenderFTBitmapIntoThis(BitmapPosX, BitmapPosY, charimg);

		// advance position //
		PenPosition += slot->advance.x >> 6;
	}
	// ensure that texture is dividable 2 //
	bitmap.MakeSizeDividableBy2();
	// determine based on parameters what to do //

	// use the method to create DDS file to memory //
	size_t MemorySize = 0;
	// copy bitmap to DDS in memory and fetch the baseline height in the image to the return value //
	char* FileInMemory = bitmap.GenerateDDSToMemory(MemorySize, baselinefromimagetop);

	// we can copy the bitmap's calculated values to the size of the box //
	bitmap.CopySizeToVal(RenderedToBox);
	ID3D11ShaderResourceView* tempview = NULL;

	HRESULT hr = D3DX11CreateShaderResourceViewFromMemory(Graphics::Get()->GetRenderer()->GetDevice(), FileInMemory, MemorySize, NULL, NULL, 
		&tempview, NULL);
	if(FAILED(hr)){

		DEBUG_BREAK;
		SAFE_DELETE(FileInMemory);
		return false;
	}
	// load texture from that file and add it to texture id //
	Graphics::Get()->GetTextureManager()->AddVolatileGenerated(TextureID, L"RenderingFont", tempview, TEXTURETYPE_TEXT);


	//DebugVariableNotifier::UpdateVariable(L"RenderSentenceToTexture::Bitmap::Width", new VariableBlock(bitmap.GetWidth()
	//	/(float)DataStore::Get()->GetWidth()));
	//DebugVariableNotifier::UpdateVariable(L"RenderSentenceToTexture::Bitmap::Height", new VariableBlock(bitmap.GetHeight()
	//	/(float)DataStore::Get()->GetHeight()));
	//DebugVariableNotifier::UpdateVariable(L"RenderSentenceToTexture::used::sizemodifier", new VariableBlock(sizemodifier));
	//DebugVariableNotifier::UpdateVariable(L"RenderSentenceToTexture::used::Text", new VariableBlock(text));
	//DebugVariableNotifier::UpdateVariable(L"RenderSentenceToTexture::used::Text::size", new VariableBlock((int)text.size()));

	// release memory //
	SAFE_DELETE(FileInMemory);
	return true;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::RenderingFont::AdjustTextSizeToFitBox(const Float2 &BoxToFit, const wstring &text, int CoordType, size_t &Charindexthatfits, 
	float &EntirelyFitModifier, float &HybridScale, Float2 &Finallength, float scaletocutfrom)
{
	// calculate theoretical max height //
	float TMax = BoxToFit.Y/GetHeight(1, CoordType);

	// calculate length of 3 dots //
	float dotslength = CalculateDotsSizeAtScale(TMax);

	float CalculatedTotalLength = CalculateTextLengthAndLastFittingExpensive(TMax, CoordType, text, BoxToFit.X, Charindexthatfits, dotslength);

	// check did all fit //
	if(Charindexthatfits == text.size()-1){
		// everything fits at the maximum size //
		// set data and return //
		EntirelyFitModifier = HybridScale = TMax;
		Finallength = Float2(CalculatedTotalLength, GetHeight(TMax, CoordType));

		return true;
	}
	// finding a scale at which the text could fit entirely //
	float MinWantedScale = TMax*scaletocutfrom;

	float LowVal = TMax*(scaletocutfrom/3);
	// how well the length needs to match //
	const float threshold = 0.02f;

	// looping variables //
	bool Stop = false;
	int itrs = 0;
	// loop until we have narrowed down to under threshold //
	while((TMax - LowVal) > threshold){
		// calculate size to test this on //
		float TestSize = (TMax+LowVal)/2;

		// Use Low value so that we undershoot rather than overshoot //
		HybridScale = LowVal;

		// check is scale too low //
		if(TestSize <= MinWantedScale){
			// calculate values at the lowest possibly wanted value //
			TestSize = MinWantedScale;
			// and break afterwards //
			Stop = true;
			// update return value //
			HybridScale = TestSize;
		}

		// update dots length //
		dotslength = CalculateDotsSizeAtScale(TestSize);
		CalculatedTotalLength = CalculateTextLengthAndLastFittingExpensive(TestSize, CoordType, text, BoxToFit.X, Charindexthatfits, dotslength);

		if(Charindexthatfits < text.size()-1 || CalculatedTotalLength >= BoxToFit.X){
			// adjust max size //
			TMax = TestSize;
		} else {
			// too small text, adjust minimum size //
			LowVal = TestSize;
		}
		// update possible return value before looping //
		Finallength = Float2(CalculatedTotalLength, GetHeight(HybridScale, CoordType));

		// we can return here if size will go too low //
		if(Stop)
			break;
		itrs++;
	}

	//DebugVariableNotifier::UpdateVariable(L"RenderingFont::AdjustTextSizeToFitBox::itrs", new VariableBlock(itrs));
	//DebugVariableNotifier::UpdateVariable(L"RenderingFont::AdjustTextSizeToFitBox::HybridScale", new VariableBlock(HybridScale));
	//DebugVariableNotifier::UpdateVariable(L"RenderingFont::CalculatedTotalLength", new VariableBlock(CalculatedTotalLength));
	// everything should be done now //
	return true;
}

float Leviathan::RenderingFont::CalculateTextLengthAndLastFittingNonExpensive(float TextSize, int CoordType, const wstring &text, const float &fitlength, 
	size_t & Charindexthatfits, float delimiterlength)
{
	// set right size for kerning //
	if(!EnsurePixelSize(CalculatePixelSizeAtScale(TextSize))){
		Logger::Get()->Error(L"RenderingFont: CalculateTextLengthAndLastFitting: set pixel size failed, cannot calculate");
		return -1;
	}
	
	// calculated length after done //
	float CalculatedTotalLength = 0.f;
	bool FitsIfLastFits = false;
	// used in checking if character would fit to the box //
	float curneededlength = 0;

	// kerning flag //
	bool kerningcheck = FT_HAS_KERNING(FontsFace) != 0;

	// calculate length using the provided size //
	for(size_t i = 0; i < text.size(); i++){
		// check is this whitespace //
		if(text[i] < L' '){
			// white space //
			CalculatedTotalLength += (int)(3.0f*TextSize);
		} else {
			// add kerning //
			if(kerningcheck){
				// fetch kerning, if not first character //
				if(i > 0){
					// change pen position according to kerning distance //
					CalculatedTotalLength += (int)GetKerningBetweenCharacters(TextSize, text[i-1], text[i]);
				}
			}

			// get size from letter index //
			CalculatedTotalLength += (int)(FontData[ConvertCharCodeToIndex(text[i])]->AdvancePixels*TextSize);
		}

textfittingtextstartofblocklabel:
		// get length that would need to fit here and check would it actually fit //
		if(FitsIfLastFits){
			// we need to only check if last character fits //
			if(i+1 < text.size()){
				// not last yet //
				continue;
			}

			// will just need to try to fit this last character //
			curneededlength = CalculatedTotalLength;
			// fall through to continue checks //

		} else {
			// check does this character and dots fit to the "box" //
			curneededlength = CalculatedTotalLength+delimiterlength;
		}

		// if coordinates are relative the box is too and the length needs to be made relative //
		if(CoordType == GUI_POSITIONABLE_COORDTYPE_RELATIVE){

			curneededlength /= DataStore::Get()->GetWidth();
		}

		// check would all this stuff fit //
		if(curneededlength <= fitlength){
			// this character would fit with truncation to the box (or is last character and would fit on its own) //
			Charindexthatfits = i;
			
		} else {
			// skip this if already set //
			if(FitsIfLastFits)
				continue;

			// this character wouldn't fit if it had to be cut from here //
			FitsIfLastFits = true;

			// check is this last character, because then we need to go back and check without delimiter //
			if(i+1 >= text.size() /*&& !Jumped*/){
				//// set jumped so that we can't get stuck in an infinite loop //
				//Jumped = true;
				// can't get stuck anymore...
				goto textfittingtextstartofblocklabel;
			}
		}
	}
	// update total length if relative //
	if(CoordType == GUI_POSITIONABLE_COORDTYPE_RELATIVE){

		CalculatedTotalLength /= DataStore::Get()->GetWidth();
	}

	return CalculatedTotalLength;
}

DLLEXPORT float Leviathan::RenderingFont::CalculateTextLengthAndLastFittingExpensive(float TextSize, int CoordType, const wstring &text, 
	const float &fitlength, size_t & Charindexthatfits, float delimiterlength)
{
	// set right size for kerning //
	if(!EnsurePixelSize(CalculatePixelSizeAtScale(TextSize))){
		Logger::Get()->Error(L"RenderingFont: CalculateTextLengthAndLastFitting: set pixel size failed, cannot calculate");
		return -1;
	}

	//DebugVariableNotifier::UpdateVariable(L"CalculateLastFitLength::TextAreaSize::X", new VariableBlock(fitlength));

	// calculated length after done //
	float CalculatedTotalLength = 0.f;
	//bool FitsIfLastFits = false;
	// used in checking if character would fit to the box //
	float curneededlength = 0;

	// kerning flag //
	bool kerningcheck = FT_HAS_KERNING(FontsFace) != 0;
	FT_GlyphSlot slot = FontsFace->glyph;

	// calculate length using the provided size //
	for(size_t i = 0; i < text.size(); i++){
		// check is this whitespace //
		if(text[i] < 32){
			// white space //
			CalculatedTotalLength += (int)(3.0f*TextSize);
		} else {
			// add kerning //
			if(kerningcheck){
				// fetch kerning, if not first character //
				if(i > 0){
					// change pen position according to kerning distance //
					CalculatedTotalLength += (int)GetKerningBetweenCharacters(TextSize, text[i-1], text[i]);
				}
			}

			// load glyph //
			FT_Error errorcode = FT_Load_Glyph(FontsFace, FontData[ConvertCharCodeToIndex(text[i])]->CharacterGlyphIndex, FT_LOAD_DEFAULT);
			if(errorcode){
				Logger::Get()->Error(L"RenderSentenceToTexture: FreeType: failed to load glyph "+text[i]);
				continue;
			}

			// get size from letter FreeType face //
			CalculatedTotalLength += (float)(slot->advance.x >> 6);
		}

//textfittingtextstartofblocklabel:
		// get length that would need to fit here and check would it actually fit //
		//if(FitsIfLastFits){
		//	// we need to only check if last character fits //
		//	if(i+1 < text.size()){
		//		// not last yet //
		//		continue;
		//	}

		//	// will just need to try to fit this last character //
		//	curneededlength = CalculatedTotalLength;
		//	// fall through to continue checks //

		//} else {
		//	// check does this character and dots fit to the "box" //
		//	curneededlength = CalculatedTotalLength+delimiterlength;
		//}

		i+1 >= text.length() ? curneededlength = CalculatedTotalLength: curneededlength = CalculatedTotalLength+delimiterlength;

		// if coordinates are relative the box is too and the length needs to be made relative //
		if(CoordType == GUI_POSITIONABLE_COORDTYPE_RELATIVE){

			curneededlength /= DataStore::Get()->GetWidth();
		}

		// check would all this stuff fit //
		if(curneededlength <= fitlength){
			// this character would fit with truncation to the box (or is last character and would fit on its own) //
			Charindexthatfits = i;

		}/* else {
		//	// skip this if already set //
		//	if(FitsIfLastFits)
		//		continue;

		//	// this character wouldn't fit if it had to be cut from here //
		//	FitsIfLastFits = true;

		//	// check is this last character, because then we need to go back and check without delimiter //
		//	if(i+1 >= text.size()){
		//		//// set jumped so that we can't get stuck in an infinite loop //
		//		//Jumped = true;
		//		// can't get stuck anymore...
		//		goto textfittingtextstartofblocklabel;
		//	}
		}*/
	}
	// update total length if relative //
	if(CoordType == GUI_POSITIONABLE_COORDTYPE_RELATIVE){

		CalculatedTotalLength /= DataStore::Get()->GetWidth();
	}

	//DebugVariableNotifier::UpdateVariable(L"CalculateLastFitLength::Calculated::X", new VariableBlock(CalculatedTotalLength));

	return CalculatedTotalLength;
}

// ------------------------------------ //
bool Leviathan::RenderingFont::_VerifyFontFTDataLoaded(){
	// verify FreeType 2 //
	if(!CheckFreeTypeLoad()){
		// can't do anything //
		return false;
	}

	// check is it already loaded //
	if(FontsFace != NULL)
		return true;

	// load file matching this font //
	wstring fontgenfile = FileSystem::Get()->SearchForFile(FILEGROUP_OTHER, Name, L"ttf", true);

	// look for it in registry //
	if(fontgenfile.size() == 0){
		// set name to arial because we can't find other fonts //
		Name = L"Arial";

		if(Name == L"Arial"){
			HKEY hKey;
			LONG lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts", 0, KEY_READ,
				&hKey);

			if(lRes != ERROR_SUCCESS){
				Logger::Get()->Error(L"FontGenerator: could not locate Arial font, OpenKey failed", true);
				return false;
			}

			WCHAR szBuffer[512];
			DWORD dwBufferSize;

			RegQueryValueEx(hKey, L"Arial (TrueType)", 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);

			//fontgenfile = L"C:\\Windows\\Fonts\\";
			FileSystem::GetWindowsFolder(fontgenfile);
			fontgenfile += L"Fonts\\";
			fontgenfile += szBuffer;
		}
	}
	if(!FileSystem::FileExists(fontgenfile)){
		Logger::Get()->Error(L"LoadTexture failed: could not generate new texture: no text definition file", true);
		return false;
	}

	FT_Error errorcode = FT_New_Face(this->FreeTypeLibrary, Convert::WstringToString(fontgenfile).c_str(), 0, &FontsFace);

	if(errorcode == FT_Err_Unknown_File_Format ){
		Logger::Get()->Error(L"FontGenerator: FreeType: unknown format!", true);
		return false;
	} else if (errorcode) {

		Logger::Get()->Error(L"FontGenerator: FreeType: cannot open file!", true);
		return false;
	}

	return true;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::RenderingFont::WriteDataToFile(){
	// write font data file //
	wstring datafilepath = FileSystem::GetFontFolder()+Name+L".levfd";

	// generate file content //
	wstringstream datafile;
	// character data count //
	datafile << FontHeight << L" " <<  FontData.size() << endl;

	// copy font data //
	for(size_t i = 0; i < FontData.size(); i++){
		
		// output data to stream //
		datafile << FontData[i]->CharCode << L" ";

		datafile << FontData[i]->CharacterGlyphIndex << " ";
		datafile << FontData[i]->PixelWidth << " ";
		datafile << FontData[i]->AdvancePixels << " ";
		datafile << FontData[i]->TopLeft.X << " " << FontData[i]->TopLeft.Y << " ";
		datafile << FontData[i]->BottomRight.X << " " << FontData[i]->BottomRight.Y << " ";
		// this object is written, put empty line //
		datafile << endl;
	}
	// write to file //
	wofstream writer;

	writer.open(datafilepath);
	// check can we write stuff //
	if(!writer.is_open()){
		Logger::Get()->Error(L"RenderingFont: WriteDataToFile: failed to open file for reading, file: "+datafilepath, true);
		return false;
	}

	// write the whole stringstream to the file and close //
	writer << datafile.str();
	writer.close();


	Logger::Get()->Info(L"RenderingFont: WriteDataToFile: wrote font data file, font: "+Name+L", file: "+datafilepath);
	return true;
}
// ------------------------------------ //
bool Leviathan::RenderingFont::CheckFreeTypeLoad(){
	if(!FreeTypeLoaded){

		if(!FreeTypeLibrary){
			// "new" library object //
			FreeTypeLibrary = FT_Library();
		}

		FT_Error error = FT_Init_FreeType(&FreeTypeLibrary);
		if(error){

			Logger::Get()->Error(L"RenderingFont: FreeTypeLoad failed", error, true);
			return false;
		}
		FreeTypeLoaded = true;
	}
	return true;
}

bool Leviathan::RenderingFont::_SetFTPixelSize(const int & size){
	// set the face size //
	SetSize = size;

	FT_Error errorcode = FT_Set_Pixel_Sizes(FontsFace, 0, SetSize);
	if(errorcode){
		Logger::Get()->Error(L"RenderingFont: FreeType: set pixel size failed", true);
		return false;
	}
	return true;
}

DLLEXPORT float Leviathan::RenderingFont::CalculateDotsSizeAtScale(const float &scale){
	// kerning needs to be right //
	EnsurePixelSize(CalculatePixelSizeAtScale(scale));

	// load the dot character
	FT_Error errorcode = FT_Load_Glyph(FontsFace, FontData[ConvertCharCodeToIndex(L'.')]->CharacterGlyphIndex, FT_LOAD_DEFAULT);
	if(errorcode){
		Logger::Get()->Error(L"RenderSentenceToTexture: FreeType: failed to load glyph dots");
		return -1;
	}

	//return 3*(FontData[ConvertCharCodeToIndex(L'.')]->AdvancePixels*scale)+2*(GetKerningBetweenCharacters(scale, '.', L'.'));
	return 3*(FontsFace->glyph->advance.x >> 6)+2*(GetKerningBetweenCharacters(scale, L'.', L'.'));
}



// ------------------ FontsCharacter ------------------ //
Leviathan::FontsCharacter::FontsCharacter(const int &charcode, const FT_UInt &glyphindex /*= 0*/) : TopLeft(0, 0), BottomRight(0, 0), PixelWidth(0),
	Generating(NULL)
{
	CharCode = charcode;
	CharacterGlyphIndex = glyphindex;
}

Leviathan::FontsCharacter::FontsCharacter(const int &charcode, const int &pixelwidth, const Float2 &texturecoordtopleft, const Float2 
	&texturecoordbotomright, const FT_UInt &glyphindex /*= 0*/) : TopLeft(texturecoordtopleft), BottomRight(texturecoordbotomright), 
	PixelWidth(pixelwidth)
{
	CharCode = charcode;
	CharacterGlyphIndex = glyphindex;
}

Leviathan::GeneratingDataForCharacter::GeneratingDataForCharacter() : ThisRendered(NULL), RenderedTop(-1), RenderedLeft(-1){

}
