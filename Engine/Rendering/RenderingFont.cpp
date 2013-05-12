#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_FONT
#include "RenderingFont.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "DDsHandler.h"

#include "FileSystem.h"

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
	SAFE_RELEASE(Textures);
	//SAFE_DELETE_ARRAY(Fontdata);
	FontData.clear();
}
// ------------------------------------ //
int RenderingFont::CountLenght(wstring &sentence, float heightmod, bool IsAbsolute, bool TranslateSize){
	// if it is non absolute and translate size is true, scale height by window size // 
	if(!IsAbsolute && TranslateSize){

		heightmod = ResolutionScaling::ScaleTextSize(heightmod);
	}

	float lenght = 0;
	for(unsigned int i = 0; i < sentence.size(); i++){
		int letterindex = ((int)sentence[i]) - 33; // no space character in letter data array //

		if(letterindex < 1){
			// space move pos over //
			if(IsAbsolute){
				lenght += 3.0f*heightmod;
			} else {
				lenght += 3.0f*heightmod;
			}
		} else {
			if(IsAbsolute){
				lenght += heightmod*1.0f + (FontData[letterindex].size*heightmod);
			} else {
				lenght += heightmod*1.0f + (FontData[letterindex].size*heightmod);
			}
		}
	}
	// round to nearest integer //
	lenght += 0.5f;

	//lenght += 3.0f*heightmod;
	if(IsAbsolute)
		return (int)lenght;
	// scale from screen lenght to promilles //
	//Logger::Get()->Info(L"CountedLenght: "+Convert::IntToWstring(lenght)+L" promille lenght "+Convert::IntToWstring(ResolutionScaling::GetPromilleFactor()*((float)lenght/DataStore::Get()->GetWidth())), false);
	return (int)(ResolutionScaling::GetPromilleFactor()*((float)lenght/DataStore::Get()->GetWidth()));
}
 int RenderingFont::GetHeight(float heightmod, bool IsAbsolute, bool TranslateSize){
	if(IsAbsolute)
		return (int)(FontHeight*heightmod+0.5f);
	// if it is non absolute and translate size is true, scale height by window size // 
	if(TranslateSize){

		heightmod = ResolutionScaling::ScaleTextSize(heightmod);
	}

	// scale from screen height to promilles //
	return (int)(ResolutionScaling::GetPromilleFactor()*((float)(FontHeight*heightmod+0.5f)/DataStore::Get()->GetHeight()));
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
			Logger::Get()->Info(L"LoadFontData: data file was missing and no bmp map, regerating texture", true);
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
		if(!CheckFreeTypeLoad()){
			Logger::Get()->Error(L"LoadTexture failed: could not generate new texture", true);
			return false;
		}
		wstring name = FileSystem::RemoveExtension(file, true);
		wstring fontgenfile = L"";
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

		} else {
			fontgenfile = FileSystem::GetFontFolder()+FileSystem::ChangeExtension(file, L"ttf");
		}
		if(!FileSystem::FileExists(fontgenfile)){
			Logger::Get()->Error(L"LoadTexture failed: could not generate new texture: no text definition file", true);
			return false;
		}

		FT_Face fontface;
		FT_Error errorcode = FT_New_Face(this->FreeTypeLibrary, Convert::WstringToString(fontgenfile).c_str(), 0, &fontface);

		if(errorcode == FT_Err_Unknown_File_Format ){
			Logger::Get()->Error(L"FontGenerator: FreeType: unkown format!", true);
			return false;
		} else if (errorcode) {

			Logger::Get()->Error(L"FontGenerator: FreeType: cannot open file!", true);
			return false;
		}
		// get desktop size //
		//int height = GetSystemMetrics(SM_CYSCREEN);
		//int width = GetSystemMetrics(SM_CXSCREEN);

		//errorcode = FT_Set_Char_Size(fontface, 0, 16*64, height, width);
		//if(errorcode){
		//	Logger::Get()->Error(L"FontGenerator: FreeType: set char size failed", true);
		//	return false;
		//}
		errorcode = FT_Set_Pixel_Sizes(fontface, 0, 32);
		if(errorcode){
			Logger::Get()->Error(L"FontGenerator: FreeType: set pixel size failed", true);
			return false;
		}

		int TotalWidth = 0;
		int Height = 35;

		//int pen_x = 0;//, pen_y = 0;

		// "image" //
		vector<vector<unsigned char>> Grayscale;
		Grayscale.resize(Height);
		
		// TODO: write font data generation + saving //
		vector<Int2> CharStartEnd;

		wchar_t chartolook = L'A';
		for(int i = 33; i <= 255; i++){ // really should start from 33 to <= 255
			chartolook = (wchar_t)i;

			int Index = FT_Get_Char_Index(fontface, chartolook);

			errorcode = FT_Load_Glyph(fontface, Index, FT_LOAD_DEFAULT);
			if(errorcode){
				Logger::Get()->Error(L"FontGenerator: FreeType: failed to load glyph number "+Convert::IntToWstring(i)+L" char "+chartolook, true);
				return false;
			}

			if(fontface->glyph->format != FT_GLYPH_FORMAT_BITMAP){
				errorcode = FT_Render_Glyph(fontface->glyph, FT_RENDER_MODE_NORMAL);
				if(errorcode){
					Logger::Get()->Error(L"FontGenerator: FreeType: failed to render glyph "+Convert::IntToWstring(i)+L" char "+chartolook, true);
					return false;
				}
			}
			FT_Bitmap charimg = fontface->glyph->bitmap;
			
			
			//int imgindex = 0;
			int YBearing = fontface->glyph->metrics.horiBearingY/64;
			//int size = charimg.rows*charimg.width;
			if((int)Grayscale.size() < charimg.rows)
				Grayscale.resize(charimg.rows);
			if(Height < charimg.rows)
				Height = charimg.rows;

			CharStartEnd.push_back(Int2(TotalWidth, TotalWidth+charimg.width));
			TotalWidth += charimg.width;

			// baseline is 
			int baseline = Height/3;
			//int glyphtop = Height-(baseline-(charimg.rows-YBearing));
			int glyphtop = Height-(baseline+YBearing);
			// little padding on top //
			glyphtop += 2;

			//glyphtop
			//glyphtop /= 2;
			for(int x = 0; x < charimg.width; x++){
				for(int y = 0; y < Height; y++){
					// set empty //
					Grayscale[y].push_back(0);

					// try to get correct color //
					if((y >= glyphtop) && (y-glyphtop < charimg.rows)){
						Grayscale[y].back() = charimg.buffer[(y-glyphtop)*charimg.width+x];
					}
				}
				//for(int y = glyphtop;(y < charimg.rows+glyphtop) && (y < Height); y++){
				//	// copy glyph //
				//	imgindex++;
				//	Grayscale[y].back() = (charimg.buffer[imgindex]);
				//}
			}
			// add empty row //
			for(int a = 0; a < Height; a++){
				Grayscale[a].push_back(0);
			}
			TotalWidth += 1;
			// increase pos //
			//pen_x += fontface->glyph->advance.x >> 6;

		}
		FT_Done_Face(fontface);

		wstring FileToUse = FileSystem::GetFontFolder()+file;

		DDSHandler::WriteDDSFromGrayScale(FileToUse, Grayscale, TotalWidth, Height/*, DDS_RGB*/);
		// magic number //
		//char magic[] = {'D', 'D', 'S', ' '};

		// write font data file //
		wstring datafilepath = FileSystem::GetFontFolder()+FileSystem::ChangeExtension(file, L"levfd");
		//wofstream writer;
		//writer.open(datafile);

		// generate file content //
		wstring datafile = L"";

		datafile += Convert::IntToWstring(Height)+L"\n";

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


			datafile += Convert::IntToWstring(curvals[1]-curvals[0])+L"\n";
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
void RenderingFont::BuildVertexArray(void* vertices, wstring text, float drawx, float drawy, float heightmod, bool IsAbsolute, bool TranslateSize){
	// if it is non absolute and translate size is true, scale height by window size // 
	if(!IsAbsolute && TranslateSize){

		heightmod = ResolutionScaling::ScaleTextSize(heightmod);
	}

	VertexType* vertexptr;

	vertexptr = (VertexType*)vertices;

	int index = 0;
	if(!FontData.size()){
		Logger::Get()->Error(L"Trying to render font which doesn't have Fontdata");
		Release();
		SAFE_DELETE_ARRAY(vertices);
		return;
	}

	//int startx = drawx;

	// draw letters to vertices //
	int letters = text.size();
	for(int i = 0; i < letters; i++){
		int letterindex = ((int)text[i]) - 33; // no space character in letter data array //

		if(letterindex < 1){
			// space move pos over //
			if(IsAbsolute){
				drawx += 3.0f*heightmod;
			} else {
				//drawx += ResolutionScaling::ScalePromilleToFactorX((((3.0f*heightmod)/DataStore::Get()->GetWidth())*1000.f)/
				//	ResolutionScaling::GetXScaleFactor());
				drawx += 3.0f*heightmod;
			}
		} else {
			if(IsAbsolute){
				// first triangle //
				vertexptr[index].position = D3DXVECTOR3(drawx, drawy, 0.0f); // top left
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].left, 0.0);
				index++;
	
				vertexptr[index].position = D3DXVECTOR3(drawx + (FontData[letterindex].size*heightmod) , drawy - (FontHeight*heightmod), 0.0f); // bottom right
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].right, 1.0f);
				index++;
	
				vertexptr[index].position = D3DXVECTOR3(drawx, drawy - (FontHeight*heightmod), 0.0f); // bottom left
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].left, 1.0f);
				index++;
				// second triangle //
				vertexptr[index].position = D3DXVECTOR3(drawx, drawy, 0.0f); // top left
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].left, 0.0);
				index++;
	
				vertexptr[index].position = D3DXVECTOR3(drawx + (FontData[letterindex].size*heightmod) , drawy, 0.0f); // top right
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].right, 0.0f);
				index++;
	
				vertexptr[index].position = D3DXVECTOR3(drawx + (FontData[letterindex].size*heightmod), drawy - (FontHeight*heightmod), 0.0f); // bottom right
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].right, 1.0f);
				index++;
	
				// update location //
				drawx += heightmod*1.0f + (FontData[letterindex].size*heightmod);
			} else {
				// first triangle //

				// coords should already been absoluted //
				//int AbsolutedX = drawx/ResolutionScaling::GetXScaleFactor();
				//int AbsolutedY = drawy/ResolutionScaling::GetYScaleFactor();
				int AbsolutedX = (int)drawx;
				int AbsolutedY = (int)drawy;
				//int AbsolutedWidth = ((ResolutionScaling::ScalePromilleToFactorX(((FontData[letterindex].size*heightmod)/DataStore::Get()->GetWidth()
				//	)*1000.f))/ResolutionScaling::GetXScaleFactor());
				//int AbsolutedHeight = ((ResolutionScaling::ScalePromilleToFactorY(((FontHeight*heightmod)/DataStore::Get()->GetHeight()
				//	)*1000.f))/ResolutionScaling::GetYScaleFactor());
				int AbsolutedWidth = (int)(FontData[letterindex].size*heightmod);
				int AbsolutedHeight = (int)(FontHeight*heightmod);

				vertexptr[index].position = D3DXVECTOR3((FLOAT)AbsolutedX, (FLOAT)AbsolutedY, 0.0f); // top left
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].left, 0.0f);
				index++;

				vertexptr[index].position = D3DXVECTOR3((FLOAT)(AbsolutedX + AbsolutedWidth) , (FLOAT)(AbsolutedY - AbsolutedHeight), 0.0f); // bottom right
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].right, 1.0f);
				index++;

				vertexptr[index].position = D3DXVECTOR3((FLOAT)(AbsolutedX), (FLOAT)(AbsolutedY - AbsolutedHeight), 0.0f); // bottom left
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].left, 1.0f);
				index++;
				// second triangle //
				vertexptr[index].position = D3DXVECTOR3((FLOAT)AbsolutedX, (FLOAT)AbsolutedY, 0.0f); // top left
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].left, 0.0);
				index++;

				vertexptr[index].position = D3DXVECTOR3((FLOAT)(AbsolutedX + AbsolutedWidth) , (FLOAT)AbsolutedY, 0.0f); // top right
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].right, 0.0f);
				index++;

				vertexptr[index].position = D3DXVECTOR3((FLOAT)(AbsolutedX + AbsolutedWidth), (FLOAT)(AbsolutedY - AbsolutedHeight), 0.0f); // bottom right
				vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex].right, 1.0f);
				index++;

				// update location //
				//drawx += ResolutionScaling::ScalePromilleToFactorX((((heightmod*1.0f + (FontData[letterindex].size*heightmod))/
				//	DataStore::Get()->GetWidth())*1000.f)/ResolutionScaling::GetXScaleFactor());
				drawx += heightmod*1.0f + (FontData[letterindex].size*heightmod);
			}
		}
	}

	//Logger::Get()->Info(L"SentenceRendered lenght: "+Convert::IntToWstring(drawx-startx), false);
}

// ------------------------------------ //

bool RenderingFont::CheckFreeTypeLoad(){
	if(!FreeTypeLoaded){

		FT_Error error = FT_Init_FreeType(&FreeTypeLibrary);
		if(error){

			Logger::Get()->Error(L"Font: FreeTypeLoad failed", error, true);
			return false;
		}
		FreeTypeLoaded = true;
	}
	return true;
}