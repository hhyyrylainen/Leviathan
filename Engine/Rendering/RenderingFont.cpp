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
	//Fontdata = NULL;
	Textures = NULL;
}
RenderingFont::~RenderingFont(){

}

bool RenderingFont::FreeTypeLoaded = false;
FT_Library RenderingFont::FreeTypeLibrary = FT_Library();
// ------------------------------------ //
bool RenderingFont::Init(ID3D11Device* dev, wstring FontFile){

	Name = FileSystem::RemoveExtension(FontFile, true);

	// load texture //
	if(!LoadTexture(dev, FontFile)){
		Logger::Get()->Error(L"Failed to init Font, load texture failed", true);
		return false;
	}

	// load character data //
	if(!LoadFontData(dev, FontFile)){
		Logger::Get()->Error(L"Failed to init Font, LoadFontData failed", true);
		return false;
	}
	return true;
}
void RenderingFont::Release(){
	// release FreeType objects //
	FT_Done_Face(FontsFace);

	SAFE_RELEASE(Textures);
	//SAFE_DELETE_ARRAY(Fontdata);
	FontData.clear();
}
// ------------------------------------ //
DLLEXPORT float Leviathan::RenderingFont::CountLength(const wstring &sentence, float heightmod, int Coordtype){
	// if it is non absolute and translate size is true, scale height by window size // 
	if(Coordtype == GUI_POSITIONABLE_COORDTYPE_RELATIVE){

		heightmod = ResolutionScaling::ScaleTextSize(heightmod);
	}

	float length = 0;
	for(size_t i = 0; i < sentence.size(); i++){
		int letterindex = ((int)sentence[i]) - 33; // no space character in letter data array //

		if(letterindex < 1){
			// space move pos over //
			length += 3.0f*heightmod;

		} else {
			length += heightmod*1.0f + (FontData[letterindex].size*heightmod);
		}
	}

	if(Coordtype == GUI_POSITIONABLE_COORDTYPE_RELATIVE)
		return length/DataStore::Get()->GetWidth();

	return length;
}
DLLEXPORT float Leviathan::RenderingFont::GetHeight(float heightmod, int Coordtype){
	if(Coordtype != GUI_POSITIONABLE_COORDTYPE_RELATIVE)
		return FontHeight*heightmod;
	// if it is relative, scale height by window size // 
	heightmod = ResolutionScaling::ScaleTextSize(heightmod);

	// scale from screen height to promilles //
	return FontHeight*heightmod/DataStore::Get()->GetHeight();
 }
// ------------------------------------ //
bool RenderingFont::CreateFontData(wstring texture, wstring texturedatafile){
	// get path to bmp version of font texture //
	wstring toload = FileSystem::GetFontFolder()+FileSystem::ChangeExtension(texture, L"bmp");

	typedef struct
	{
		BYTE red;
		BYTE green;
		BYTE blue;
	} RGBTriplet;
	int Width = 0;
	int Height = 0;
	long Size = 0;

	BYTE* buffer = FileSystem::LoadBMP(&Width, &Height, &Size, toload.c_str());
	if(buffer == NULL){

		return false;
	}
	RGBTriplet* image = (RGBTriplet*) FileSystem::ConvertBMPToRGBBuffer(buffer, Width, Height);
	if(image == NULL){

		return false;
	}
	SAFE_DELETE_ARRAY(buffer);


	// data loaded //
	wstring validchars = Misc::GetValidCharacters();
	
	//int count = validchars.size();
	int charindex = 1;
	bool emptyrow = false;

	FontType* Fontdata = new FontType[validchars.size()-1];
	if(!Fontdata)
		return false;

	//Fontdata[0].height = Height;
	//Fontdata[0].left = 0;
	//Fontdata[0].right = 0;
	//Fontdata[0].size = 3; // do not write space to file //

	FontHeight = Height;

	bool* emptyrows;
	emptyrows = new bool[Width];

	for(int windex = 0; windex < Width; windex++){
		for(int hindex = 0; hindex < Height; hindex++){
			// check if row empty //
			// get colors //
			int index = windex + ((Width)*hindex);
			if(image[index].red != 255 || image[index].green != 255 || image[index].blue != 255){
				emptyrow = false;
				break;
			}
			
			emptyrow = true;
		}
		if(emptyrow){
			// empty row //
			emptyrow = false;
			emptyrows[windex] = true;
		} else {
			emptyrows[windex] = false;
		}
	}
	// stop between empty rows //
	bool skipping = false;
	for(int i = 0; i < Width; i++){
		if(emptyrows[i]){
			skipping = false;
			continue;
		}
		// non emptyrow //
		if(skipping)
			continue;
		skipping = true;

		// character found! //
		// set data //

		// sample data to doubles //
		Fontdata[charindex].left = ((float)i)/Width;

		// look for end of full space //
		int endspot = i;
		while(!emptyrows[endspot]){
			endspot++;


		}
		//endspot--;
		Fontdata[charindex].right = ((float)(endspot))/Width;
		//Fontdata[charindex].height = Height;
		Fontdata[charindex].size = endspot-i+1;

		charindex++;

		continue;
		
	}



	// save //
	
	wofstream writer;
	writer.open(texturedatafile);
	// output height //
	writer << Height << endl;

	for(unsigned int i = 0; i < validchars.size(); i++){
		writer << (i+32) << L" ";
		writer << (wchar_t)(i+32) << L" ";
		writer << Fontdata[i].left << L" ";
		writer << Fontdata[i].right << L" ";
		writer << Fontdata[i].size << endl;
		//writer << Fontdata[i].height << endl;
	}
	writer.close();

	// release data //



	SAFE_DELETE_ARRAY(image);
	delete[] Fontdata; // required later
	delete[] emptyrows;
	return true;

}
bool RenderingFont::LoadFontData(ID3D11Device* dev, wstring file){

	// is data already in memory?//
	if(FontData.size() > 10){
		// it is //
		Logger::Get()->Info(L"Font data already in memory, skipping load", true);
		return true;
	}

	// check is there font data //
	wstring texturedatafile = FileSystem::GetFontFolder()+FileSystem::ChangeExtension(file, L"levfd");
	if(!FileSystem::FileExists(texturedatafile)){
		Logger::Get()->Info(L"Font data file doesn't exist, creating...", true);
		// create data since it doesn't exist //
		if(!CreateFontData(file, texturedatafile)){
			Logger::Get()->Info(L"LoadFontData: data file was missing and no bmp map, regenerating texture", true);
			if(!LoadTexture(dev, file, true)){
				Logger::Get()->Error(L"LoadFontData: no data file found and generating it from font file failed", true);
				return false;
			}
			//return true;
		}
		//return true;
	}

	// create font data buffer //
	//int chars = Misc::GetValidCharacters().size()-1;
	int chars = 233;
	//Fontdata = new FontType[chars];
	FontData.resize(chars);
	//if(!Fontdata)
	//	return false;


	// load data //
	wifstream reader;
	reader.open(texturedatafile);
	if(reader.fail())
		return false;
	// read in coordinates //
//	wchar_t Buff[150];
//	wstring curline = L"";
	int index = 0;
	wchar_t readc = L'y';
	// read in height //
	reader >> FontHeight;
	while(reader.good()){
		reader.get(readc);
		while(readc != L' ' && reader.good()){
			reader.get(readc);
		}
		reader.get(readc);
		if(readc == L' ')
			readc = L'y';
		while(readc != L' ' && reader.good()){
			reader.get(readc);
		}

		reader >> FontData[index].left;
		reader >> FontData[index].right;
		reader >> FontData[index].size;
		//reader >> Fontdata[index].height;	// already loaded
		index++;
		if(index >= chars)
			break;
	}
	reader.close();

	return true;
}
// ------------------------------------ //
bool RenderingFont::LoadTexture(ID3D11Device* dev, wstring file, bool forcegen){
	// check does file exist //
	if((!FileSystem::FileExists(FileSystem::GetFontFolder()+file)) | forcegen){
		Logger::Get()->Info(L"No font texture found: generating...", false);
		// try to generate one //
		if(!_VerifyFontFTDataLoaded()){
			Logger::Get()->Error(L"LoadTexture failed: could not generate new texture", true);
			return false;
		}

		int fontsizemultiplier = DataStore::Get()->GetFontSizeMultiplier();

		FT_Error errorcode = FT_Set_Pixel_Sizes(FontsFace, 0, 32*fontsizemultiplier);
		if(errorcode){
			Logger::Get()->Error(L"FontGenerator: FreeType: set pixel size failed", true);
			return false;
		}

		int TotalWidth = 0;
		int Height = 2+(32*fontsizemultiplier);

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

		// write font data file //
		wstring datafilepath = FileSystem::ChangeExtension(FileToUse, L"levfd");

		// generate file content //
		wstring datafile = L"";

		// need to undo height //

		datafile += Convert::IntToWstring((Height-2)/fontsizemultiplier)+L"\n";

		for(int i = 33; i <= 255; i++){
			Int2 curvals = CharStartEnd[i-33];

			datafile += Convert::IntToWstring(i)+L" ";
			datafile += Convert::ToWstring(((wchar_t)i));
			datafile += L" ";

			// count percentage from beginning //
			double Dist = curvals[0]/(double)TotalWidth;
			double Distend = curvals[1]/(double)TotalWidth;

			datafile += Convert::ToWstring(Dist)+L" ";
			datafile += Convert::ToWstring(Distend)+L" ";


			datafile += Convert::IntToWstring((curvals[1]-curvals[0])/fontsizemultiplier)+L"\n";
		}

		if(!FileSystem::WriteToFile(datafile, datafilepath)){
			Logger::Get()->Error(L"FontGenerator: could not write font datafile! ", GetLastError(),true);
			return false;
		}


		//writer.close();
		Logger::Get()->Info(L"FontGenerator: wrote font data file "+datafilepath, false);

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

ID3D11ShaderResourceView* RenderingFont::GetTexture(){
	return Textures->GetTexture();
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

				float AbsolutedWidth = FontData[letterindex].size*textmodifier;
				float AbsolutedHeight = FontHeight*textmodifier;

				vertexptr[index].position = D3DXVECTOR3(drawx, drawy, 0.0f); // top left
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].left, 0.0f);
				index++;

				vertexptr[index].position = D3DXVECTOR3(drawx + AbsolutedWidth, drawy - AbsolutedHeight, 0.0f); // bottom right
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].right, 1.0f);
				index++;

				vertexptr[index].position = D3DXVECTOR3(drawx, drawy - AbsolutedHeight, 0.0f); // bottom left
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].left, 1.0f);
				index++;
				// second triangle //
				vertexptr[index].position = D3DXVECTOR3(drawx, drawy, 0.0f); // top left
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].left, 0.0);
				index++;

				vertexptr[index].position = D3DXVECTOR3(drawx + AbsolutedWidth , drawy, 0.0f); // top right
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].right, 0.0f);
				index++;

				vertexptr[index].position = D3DXVECTOR3(drawx + AbsolutedWidth, drawy - AbsolutedHeight, 0.0f); // bottom right
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].right, 1.0f);
				index++;

				// update location //
				drawx += textmodifier*1.0f + (FontData[letterindex].size*textmodifier);

			} else {

				// first triangle //
				vertexptr[index].position = D3DXVECTOR3(drawx, drawy, 0.0f); // top left
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].left, 0.0);
				index++;

				vertexptr[index].position = D3DXVECTOR3(drawx + (FontData[letterindex].size*textmodifier) , drawy - (FontHeight*textmodifier), 0.0f); // bottom right
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].right, 1.0f);
				index++;

				vertexptr[index].position = D3DXVECTOR3(drawx, drawy - (FontHeight*textmodifier), 0.0f); // bottom left
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].left, 1.0f);
				index++;
				// second triangle //
				vertexptr[index].position = D3DXVECTOR3(drawx, drawy, 0.0f); // top left
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].left, 0.0);
				index++;

				vertexptr[index].position = D3DXVECTOR3(drawx + (FontData[letterindex].size*textmodifier) , drawy, 0.0f); // top right
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].right, 0.0f);
				index++;

				vertexptr[index].position = D3DXVECTOR3(drawx + (FontData[letterindex].size*textmodifier), drawy - (FontHeight*textmodifier), 0.0f); // bottom right
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].right, 1.0f);
				index++;

				// update location //
				drawx += textmodifier*1.0f + (FontData[letterindex].size*textmodifier);
			}
		}
	}
	return true;
}

DLLEXPORT bool Leviathan::RenderingFont::AdjustTextSizeToFitBox(const float &Size, const Float2 &BoxToFit, const wstring &text, int CoordType, 
	size_t &Charindexthatfits, float &EntirelyFitModifier, float &HybridScale, Float2 &Finallength, float scaletocutfrom)
{
	// calculate theoretical max height //
	float TMax = GetHeight(1.f, CoordType)/BoxToFit.Y;


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

DLLEXPORT bool Leviathan::RenderingFont::RenderSentenceToTexture(const int &TextureID, const float &sizemodifier, const wstring &text, Float2 &RenderedToBox){
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
	int baseline = (32*sizemodifier)/3;


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
		int BitmapPosY = baseline-slot->bitmap_top;

		// render the bitmap to the result bitmap //
		bitmap.RenderFTBitmapIntoThis(BitmapPosX, BitmapPosY, charimg);

		// advance position //
		PenPosition += slot->advance.x >> 6;
	}
	// determine based on parameters what to do //
	// use the method to create DDS file to memory //
	size_t MemorySize = 0;

	unsigned char* FileInMemory = bitmap.GenerateDDSToMemory(MemorySize);

	ID3D11ShaderResourceView* tempview = NULL;

	HRESULT hr = D3DX11CreateShaderResourceViewFromMemory(Graphics::Get()->GetRenderer()->GetDevice(), FileInMemory, MemorySize, NULL, NULL, 
		&tempview, NULL);
	if(FAILED(hr)){

		DEBUG_BREAK;
		return NULL;
	}

	// load texture from that file and add it to texture id //
	Graphics::Get()->GetTextureManager()->AddVolatileGenerated(TextureID, L"RenderingFont", tempview; 

	SAFE_DELETE(FileInMemory);

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
		if(text[i] < L' '){
			// white space //
			CalculatedTotalLength += 3.0f*TextSize;
		} else {
			// get size from letter index //
			int letterindex = ((int)text[i])-33; // no space character in letter data array //


			CalculatedTotalLength += 1.f*TextSize+FontData[letterindex].size*TextSize;
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
	return 3.f*(FontData[L'.'-33].size*scale+1.f*scale);
}

bool Leviathan::RenderingFont::_VerifyFontFTDataLoaded(){
	// verify FreeType 2 //
	if(!CheckFreeTypeLoad()){
		// can't do anything //
		return false;
	}

	// load file matching this font //
	wstring fontgenfile = FileSystem::SearchForFile(FILEGROUP_OTHER, Name, L"ttf", true);

	// look for it in registry //
	if(fontgenfile.size() == 0){
		// set name to arial because we can't find other fonts //
		name = L"Arial";

		if(name == L"Arial"){
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
		Logger::Get()->Error(L"FontGenerator: FreeType: unkown format!", true);
		return false;
	} else if (errorcode) {

		Logger::Get()->Error(L"FontGenerator: FreeType: cannot open file!", true);
		return false;
	}

	return true;
}


// ------------------------------------ //
bool RenderingFont::CheckFreeTypeLoad(){
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