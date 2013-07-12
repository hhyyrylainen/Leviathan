#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_FONT
#include "RenderingFont.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "DDsHandler.h"

#include "FileSystem.h"
#include "..\GuiPositionable.h"
#include "..\DataStore.h"
#include "Graphics.h"
#include "..\ScaleableFreeTypeBitmap.h"

RenderingFont::RenderingFont(){
	// set initial data to NULL //
	Textures = NULL;
	FontsFace = NULL;
}
RenderingFont::~RenderingFont(){

}

bool RenderingFont::FreeTypeLoaded = false;
FT_Library RenderingFont::FreeTypeLibrary = FT_Library();
// ------------------------------------ //
DLLEXPORT bool Leviathan::RenderingFont::Init(ID3D11Device* dev, const wstring &FontFile){
	// get name from the filename //
	Name = FileSystem::RemoveExtension(FontFile, true);

	// load texture //
	if(!LoadTexture(dev, FontFile)){
		Logger::Get()->Error(L"RenderingFont: Init: texture loading failed, file: "+FontFile);
		return false;
	}

	// load character data //
	if(!LoadFontData(dev, FontFile)){
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

	SAFE_RELEASEDEL(Textures);
	SAFE_DELETE_VECTOR(FontData);
}
// ------------------------------------ //
DLLEXPORT float Leviathan::RenderingFont::CountLength(const wstring &sentence, float heightmod, int Coordtype){
	// call another function, this is so that the parameters don't all need to be created to call it //
	float delimiter = 0;
	size_t lastfit = 0;
	float fitlength = 0;
	// actual counting //
	return CalculateTextLengthAndLastFitting(heightmod, Coordtype, sentence, fitlength, lastfit, delimiter);
}
DLLEXPORT float Leviathan::RenderingFont::GetHeight(float heightmod, int Coordtype){
	if(Coordtype != GUI_POSITIONABLE_COORDTYPE_RELATIVE)
		return FontHeight*heightmod;
	// if it is relative, scale height by window size // 
	heightmod = ResolutionScaling::ScaleTextSize(heightmod);

	// scale from screen height to promilles //
	return (FontHeight*heightmod)/DataStore::Get()->GetHeight();
 }
// ------------------------------------ //
bool Leviathan::RenderingFont::LoadFontData(ID3D11Device* dev,const wstring &file){
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



	// load data //
	wifstream reader;
	reader.open(texturedatafile);
	if(!reader.is_open())
		return false;
	// read in coordinates //
	int index = 0;
	// read in height //

	reader >> FontHeight;

	int DataToRead = 0;
	reader >> DataToRead;

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

	return true;
}
// ------------------------------------ //
bool Leviathan::RenderingFont::LoadTexture(ID3D11Device* dev, const wstring &file, bool forcegen /*= false*/){
	// check does file exist //
	if(forcegen || !FileSystem::FileExists(file)){
		Logger::Get()->Info(L"No font texture found: generating...", false);
		// try to generate one //
		if(!_VerifyFontFTDataLoaded()){
			Logger::Get()->Error(L"RenderingFont: LoadTexture: could not generate new texture, file: "+file, true);
			return false;
		}
		// multiplier that increases generated fonts size //
		int fontsizemultiplier = DataStore::Get()->GetFontSizeMultiplier();

		FT_Error errorcode = FT_Set_Pixel_Sizes(FontsFace, 0, FONT_BASEPIXELHEIGHT*fontsizemultiplier);
		if(errorcode){
			Logger::Get()->Error(L"FontGenerator: FreeType: set pixel size failed", true);
			return false;
		}

		int TotalWidth = 0;
		// height "should" be forced to be this //
		int Height = (FONT_BASEPIXELHEIGHT*fontsizemultiplier);

		// "image" //
		vector<vector<unsigned char>> Grayscale(Height);
		//Grayscale.reserve(Height);
		// stores start and end pixel positions of characters //
		vector<Int2> CharStartEnd;
		CharStartEnd.reserve(255-33);

		for(int i = 33; i <= 255; i++){ // really should start from 33 to <= 255
			wchar_t chartolook = (wchar_t)i;

			int Index = FT_Get_Char_Index(FontsFace, chartolook);

			if(Index == 0){
				// missing glyph //
				// ignore here //

			}


			errorcode = FT_Load_Glyph(FontsFace, Index, FT_LOAD_DEFAULT);
			if(errorcode){
				Logger::Get()->Error(L"FontGenerator: FreeType: failed to load glyph number "+Convert::IntToWstring(i)+L" char "+chartolook, true);
				return false;
			}
			// check is it already rendered //
			if(FontsFace->glyph->format != FT_GLYPH_FORMAT_BITMAP){
				// needs to render the glyph //
				errorcode = FT_Render_Glyph(FontsFace->glyph, FT_RENDER_MODE_NORMAL);
				if(errorcode){
					Logger::Get()->Error(L"FontGenerator: FreeType: failed to render glyph "+Convert::IntToWstring(i)+L" char "+chartolook, true);
					return false;
				}
			}
			FT_Bitmap charimg = FontsFace->glyph->bitmap;
			
			int YBearing = FontsFace->glyph->metrics.horiBearingY/64;

			if(Height < charimg.rows){
				Height = charimg.rows;
				DEBUG_BREAK;
			}

			CharStartEnd.push_back(Int2(TotalWidth, TotalWidth+charimg.width));
			TotalWidth += charimg.width;

			// baseline is 
			int baseline = Height/3;
			int glyphtop = Height-(baseline+YBearing);
			// little padding on top //
			glyphtop += 1;

			for(int x = 0; x < charimg.width; x++){
				for(int y = 0; y < Height; y++){

					// try to get correct color //
					if((y >= glyphtop) && (y-glyphtop < charimg.rows)){
						// copy colour from glyph //
						Grayscale[y].push_back(charimg.buffer[(y-glyphtop)*charimg.width+x]);

					} else {
						// set empty //
						Grayscale[y].push_back(0);
					}
				}
			}
			// add empty row //
			for(int a = 0; a < Height; a++){
				Grayscale[a].push_back(0);
				Grayscale[a].push_back(0);
			}
			// increase pos (to account for the empty row) //
			TotalWidth += 2;
		}

		FT_Done_Face(FontsFace);

		wstring FileToUse = FileSystem::GetFontFolder()+file;

		DDSHandler::WriteDDSFromGrayScale(FileToUse, Grayscale, TotalWidth, Height/*, DDS_RGB*/);


		Logger::Get()->Info(L"FontGenerator: font successfully generated", false);
	}

	Textures = new TextureArray();
	if(!Textures)
		return false;

	if(!Textures->Init(dev, FileSystem::GetFontFolder()+file, L"")){
		Logger::Get()->Error(L"LoadTexture failed Texture init returned false", true);
		return false;
	}

	return true;
}
// ------------------------------------ //
bool Leviathan::RenderingFont::BuildVertexArray(VertexType* vertexptr, const wstring &text, float drawx, float drawy, float textmodifier, int Coordtype){
	// if it is non absolute and translate size is true, scale height by window size // 
	if(Coordtype == GUI_POSITIONABLE_COORDTYPE_RELATIVE){

		textmodifier = ResolutionScaling::ScaleTextSize(textmodifier);
	}

	int index = 0;
	if(!FontData.size()){
		Logger::Get()->Error(L"Trying to render font which doesn't have Fontdata");
		Release();
		SAFE_DELETE_ARRAY(vertexptr);
		return false;
	}

	// draw letters to vertices //
	for(size_t i = 0; i < text.size(); i++){
		int letterindex = ((int)text[i]) - 33; // no space character in letter data array //

		if(letterindex < 1){
			// space move pos over //
			drawx += 3.0f*textmodifier;

		} else {

			if(Coordtype == GUI_POSITIONABLE_COORDTYPE_RELATIVE){

				float AbsolutedWidth = FontData[letterindex].PixelWidth*textmodifier;
				float AbsolutedHeight = FontHeight*textmodifier;

				vertexptr[index].position = D3DXVECTOR3(drawx, drawy, 0.0f); // top left
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].TopLeft, 0.0f);
				index++;

				vertexptr[index].position = D3DXVECTOR3(drawx + AbsolutedWidth, drawy - AbsolutedHeight, 0.0f); // bottom right
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].BottomRight, 1.0f);
				index++;

				vertexptr[index].position = D3DXVECTOR3(drawx, drawy - AbsolutedHeight, 0.0f); // bottom left
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].TopLeft, 1.0f);
				index++;
				// second triangle //
				vertexptr[index].position = D3DXVECTOR3(drawx, drawy, 0.0f); // top left
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].TopLeft, 0.0);
				index++;

				vertexptr[index].position = D3DXVECTOR3(drawx + AbsolutedWidth , drawy, 0.0f); // top right
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].BottomRight, 0.0f);
				index++;

				vertexptr[index].position = D3DXVECTOR3(drawx + AbsolutedWidth, drawy - AbsolutedHeight, 0.0f); // bottom right
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].BottomRight, 1.0f);
				index++;

				// update location //
				drawx += textmodifier*1.0f + (FontData[letterindex].PixelWidth*textmodifier);

			} else {

				// first triangle //
				vertexptr[index].position = D3DXVECTOR3(drawx, drawy, 0.0f); // top left
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].TopLeft, 0.0);
				index++;

				vertexptr[index].position = D3DXVECTOR3(drawx + (FontData[letterindex].PixelWidth*textmodifier) , drawy - (FontHeight*textmodifier), 0.0f); // bottom right
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].BottomRight, 1.0f);
				index++;

				vertexptr[index].position = D3DXVECTOR3(drawx, drawy - (FontHeight*textmodifier), 0.0f); // bottom left
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].TopLeft, 1.0f);
				index++;
				// second triangle //
				vertexptr[index].position = D3DXVECTOR3(drawx, drawy, 0.0f); // top left
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].TopLeft, 0.0);
				index++;

				vertexptr[index].position = D3DXVECTOR3(drawx + (FontData[letterindex].PixelWidth*textmodifier) , drawy, 0.0f); // top right
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].BottomRight, 0.0f);
				index++;

				vertexptr[index].position = D3DXVECTOR3(drawx + (FontData[letterindex].PixelWidth*textmodifier), drawy - (FontHeight*textmodifier), 0.0f); // bottom right
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].BottomRight, 1.0f);
				index++;

				// update location //
				drawx += textmodifier*1.0f + (FontData[letterindex].PixelWidth*textmodifier);
			}
		}
	}
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
	FT_Error errorcode = FT_Set_Pixel_Sizes(FontsFace, 0, (UINT)(32*sizemodifier+0.5f));
	if(errorcode){
		Logger::Get()->Error(L"RenderSentenceToTexture: FreeType: set pixel size failed", true);
		return false;
	}
	// create bitmap matching "size" //
	ScaleableFreeTypeBitmap bitmap((int)(text.size()*28*sizemodifier), (int)(32*sizemodifier+0.5f));

	int PenPosition = 0;

	// set base line to be 1/3 from the bottom (actually the top of the bitmap's coordinates) //
	int baseline = (int)(((32*sizemodifier)/3)-0.5f);

	bitmap.SetBaseLine((int)((32*sizemodifier)-baseline));


	FT_GlyphSlot slot = FontsFace->glyph;

	// fill it with data //
	for(size_t i = 0; i < text.size(); i++){
		if(text[i] < 32){
			// whitespace //
			
			PenPosition += 3;

			continue;
		}

		// load glyph //
		errorcode = FT_Load_Char(FontsFace, text[i], FT_LOAD_RENDER);
		if(errorcode){
			Logger::Get()->Error(L"RenderSentenceToTexture: FreeType: failed to load glyph "+text[i]);
			continue;
		}
		// get the bitmap //
		FT_Bitmap& charimg = slot->bitmap;

		int BitmapPosX = PenPosition+slot->bitmap_left;
		int BitmapPosY = slot->bitmap_top-baseline;

		// render the bitmap to the result bitmap //
		bitmap.RenderFTBitmapIntoThis(BitmapPosX, BitmapPosY, charimg);

		// advance position //
		PenPosition += slot->advance.x >> 6;
	}
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
	Graphics::Get()->GetTextureManager()->AddVolatileGenerated(TextureID, L"RenderingFont", tempview);

	SAFE_DELETE(FileInMemory);

	return true;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::RenderingFont::AdjustTextSizeToFitBox(const float &Size, const Float2 &BoxToFit, const wstring &text, int CoordType, 
	size_t &Charindexthatfits, float &EntirelyFitModifier, float &HybridScale, Float2 &Finallength, float scaletocutfrom)
{
	// calculate theoretical max height //
	float TMax = BoxToFit.Y/GetHeight(1, CoordType);


	// make absolute modifier //
	if(CoordType == GUI_POSITIONABLE_COORDTYPE_RELATIVE){

		TMax = ResolutionScaling::ScaleTextSize(TMax);
	}


	// calculate length of 3 dots //
	float dotslength = CalculateDotsSizeAtScale(TMax);

	float CalculatedTotalLength = CalculateTextLengthAndLastFitting(TMax, CoordType, text, BoxToFit.X, Charindexthatfits, dotslength);

	// check did all fit //
	if(Charindexthatfits == text.size()-1){
		// everything fits at the maximum size //
		// set data and return //
		EntirelyFitModifier = HybridScale = ResolutionScaling::UnScaleTextFromSize(TMax);
		Finallength = Float2(CalculatedTotalLength, GetHeight(TMax, CoordType));

		return true;
	}

	// check at which scale the text would entirely fit //
	// AdjustedScale = original * (wanted length/got length)
	float AdjustedScale = TMax*(BoxToFit.X/CalculatedTotalLength);

	// adjusted scale is now the scale that allows all characters to fit //
	EntirelyFitModifier = ResolutionScaling::UnScaleTextFromSize(AdjustedScale);

	// check is it too low //
	if(AdjustedScale < scaletocutfrom*TMax){
		// we are going to need to count the spot where the text needs to be cut with scaletocutfrom*TMax //
		HybridScale = ResolutionScaling::UnScaleTextFromSize(scaletocutfrom*TMax);

		// new length to dots //
		dotslength = CalculateDotsSizeAtScale(scaletocutfrom*TMax);

		CalculatedTotalLength = CalculateTextLengthAndLastFitting(scaletocutfrom*TMax, CoordType, text, BoxToFit.X, Charindexthatfits, dotslength);

		Finallength = Float2(CalculatedTotalLength, GetHeight(scaletocutfrom*TMax, CoordType));
		return true;
	}

	// we can use adjusted scale to fit everything //
	HybridScale = EntirelyFitModifier;

	dotslength = CalculateDotsSizeAtScale(AdjustedScale);
	CalculatedTotalLength = CalculateTextLengthAndLastFitting(AdjustedScale, CoordType, text, BoxToFit.X, Charindexthatfits, dotslength);

	if(Charindexthatfits != text.size()-1){
		// something is definitely wrong //
		DEBUG_BREAK;
	}

	Finallength = Float2(CalculatedTotalLength, GetHeight(AdjustedScale, CoordType));

	return true;
}

float Leviathan::RenderingFont::CalculateTextLengthAndLastFitting(float TextSize, int CoordType, const wstring &text, const float &fitlength, 
	size_t & Charindexthatfits, float delimiterlength)
{
	// TextSize is most likely already adjusted with CoordType so don't adjust here //
	float CalculatedTotalLength = 0.f;

	bool FitsIfLastFits = false;
	float curneededlength = 0;

	// calculate length using the theoretical maximum size //
	for(size_t i = 0; i < text.size(); i++){
		// check is this whitespace //
		if(text[i] <= L' '){
			// white space //
			CalculatedTotalLength += 3.0f*TextSize;
		} else {
			// get size from letter index //
			int letterindex = ((int)text[i])-33; // no space character in letter data array //


			CalculatedTotalLength += 1.f*TextSize+FontData[letterindex].PixelWidth*TextSize;
		}

		bool Jumped = false;

textfittingtextstartofblocklabel:


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
			curneededlength = (CalculatedTotalLength+delimiterlength);
		}

		if(CoordType == GUI_POSITIONABLE_COORDTYPE_RELATIVE){

			curneededlength /= DataStore::Get()->GetWidth();
		}

		// would all this stuff fit //
		if(curneededlength <= fitlength){
			// check are we trying to fit last character //
			if(FitsIfLastFits){
				// last character was able to fit without delimiting characters //
				Charindexthatfits = i;

			} else {
				// this character would fit with truncation to the box //
				Charindexthatfits = i;
			}
		} else {
			// this character wouldn't fit if it had to be cut from here //
			FitsIfLastFits = true;

			// check is this last character, because then we need to go back and check without delimiter //
			if(i+1 >= text.size() && !Jumped){
				// set jumped so that we can't get stuck in an infinite loop //
				Jumped = true;
				goto textfittingtextstartofblocklabel;
			}
		}
	}

	// update total length //
	if(CoordType == GUI_POSITIONABLE_COORDTYPE_RELATIVE){

		CalculatedTotalLength /= DataStore::Get()->GetWidth();
	}

	return CalculatedTotalLength;
}

float Leviathan::RenderingFont::CalculateDotsSizeAtScale(const float &scale){
	return 3.f*(FontData[L'.'-33].PixelWidth*scale+1.f*scale);
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
	wstring fontgenfile = FileSystem::SearchForFile(FILEGROUP_OTHER, Name, L"ttf", true);

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
	wstringstream datafile(Convert::IntToWstring(FontHeight)+L" ");
	// character data count //
	datafile << FontData.size() << endl;

	// copy font data //
	for(size_t i = 0; i < FontData.size(); i++){
		
		// output data to stream //
		datafile << FontData[i]->CharCode << L" ";

		datafile << FontData[i]->CharacterGlyphIndex << " ";
		datafile << FontData[i]->PixelWidth << " ";
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
	writer << datafile;
	writer.close();


	Logger::Get()->Info(L"RenderingFont: WriteDataToFile: wrote font data file, font: "+Name+L", file: "+datafilepath);
	return true;
}
// ------------------------------------ //
ID3D11ShaderResourceView* Leviathan::RenderingFont::GetTexture(){
	return Textures->GetTexture();
}

bool Leviathan::RenderingFont::CheckFreeTypeLoad(){
	if(!FreeTypeLoaded){

		FT_Error error = FT_Init_FreeType(&FreeTypeLibrary);
		if(error){

			Logger::Get()->Error(L"RenderingFont: FreeTypeLoad failed", error, true);
			return false;
		}
		FreeTypeLoaded = true;
	}
	return true;
}
// ------------------ FontsCharacter ------------------ //
Leviathan::FontsCharacter::FontsCharacter(const int &charcode, const FT_UInt &glyphindex /*= 0*/) : TopLeft(0, 0), BottomRight(0, 0), PixelWidth(0){
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
