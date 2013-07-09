#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SCALEABLEFREETYPEBITMAP
#include "ScaleableFreeTypeBitmap.h"
#endif
#include "..\..\..\..\..\..\..\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Samples\C++\Direct3D11\DDSWithoutD3DX11\DDS.h"
#include "DataStore.h"
using namespace Leviathan;
// ------------------------------------ //
// initialize to match size //
DLLEXPORT Leviathan::ScaleableFreeTypeBitmap::ScaleableFreeTypeBitmap(const int &initialwidth, const int &initialheight){

	MaxWidth = initialwidth;
	MaxHeight = initialheight;
}

DLLEXPORT Leviathan::ScaleableFreeTypeBitmap::~ScaleableFreeTypeBitmap(){
	// release memory //
	SAFE_DELETE_VECTOR(BitmapData);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ScaleableFreeTypeBitmap::RenderFTBitmapIntoThis(const int &xstart, const int &ystart, FT_Bitmap &bmap){
	// loop through the pixels and set them //
	for(int x = 0; x < bmap.width; x++){
		int fx = xstart+x;

		BitmapVerticalLine* line = GetXLine(fx);

		for(int y = 0; y < bmap.rows; y++){
			int fy = ystart+y;

			(*line->GetYvalue(fy)) = bmap.buffer[y*bmap.width+x];
		}
	}
}
// ------------------------------------ //
DLLEXPORT char* Leviathan::ScaleableFreeTypeBitmap::GenerateDDSToMemory(size_t &GeneratedSize){
	// calculate required size //
	char* buffer = NULL;

	int BitsPerPixel = 8;
	// MSDN tells that this is how pitch should be calculated //
	int Pitch = (MaxWidth * BitsPerPixel + 7)/8;

	// create header //
	DDS_HEADER header;
	DDS_HEADER_DXT10 dx10header;
	ZeroMemory(&header, sizeof(header));
	ZeroMemory(&dx10header, sizeof(dx10header));

	header.dwSize = sizeof(DDS_HEADER);
	//header.dwHeaderFlags = 0x1 | 0x2 | 0x4 | 0x1000;
	header.dwHeaderFlags = DDS_HEADER_FLAGS_TEXTURE;
	header.dwHeight = MaxHeight;
	header.dwWidth = MaxWidth;
	header.dwPitchOrLinearSize = Pitch;
	header.ddspf = DDSPF_DX10;
	header.dwSurfaceFlags = DDS_SURFACE_FLAGS_TEXTURE;

	// calculate total size //
	GeneratedSize = 0;
	GeneratedSize += 4; // magic chars //
	GeneratedSize += sizeof(header)/sizeof(char); // calculate how many characters it takes to represent dds header //
	GeneratedSize += sizeof(dx10header)/sizeof(char); // directx 10 and up header defining the specific format //
	GeneratedSize += BitsPerPixel*(MaxWidth*MaxHeight); // 3 bits(chars) per pixel that represent rgb values //

	// create the buffer //
	buffer = new char[GeneratedSize];

	size_t bufferindex = 0;
	// put magic //
	buffer[0] = 'D';
	buffer[1] = 'D';
	buffer[2] = 'S';
	buffer[3] = ' ';

	bufferindex = 4;

	// copy header //
	memcpy(&buffer[bufferindex], (char*)&header, sizeof(header));

	// put pointer to right pos //
	bufferindex += sizeof(header)/sizeof(char);

	// copy dx10 header //
	memcpy(&buffer[bufferindex], (char*)&dx10header, sizeof(dx10header));

	// put pointer to right pos //
	bufferindex += sizeof(dx10header)/sizeof(char);


	// copy pixel data //
	for(int y = 0; y < MaxHeight; y++){
		for(int x = 0; x < MaxWidth; x++){
			// write data and move to next //
			buffer[bufferindex] = GetPixelDataAtPos(x, y);
			bufferindex += 1;
		}
	}

	// just because it can be done, save to file //
	int inumber = 0;

	if(!DataStore::Get()->GetValueAndConvertTo<int>(L"Bitmaprenderingind",inumber)){
		// add new //
		DataStore::Get()->AddVar(new NamedVariableList(L"Bitmaprenderingind", new VariableBlock(inumber)));
	} else {
		// set new value //
		DataStore::Get()->SetValue(L"Bitmaprenderingind", new VariableBlock(inumber));
	}

	ofstream writer;
	writer.open(".\renderedtext"+Convert::ToString<int>(inumber)+".dds");

	writer.write(buffer, GeneratedSize);

	writer.close();

	// return buffer //
	return buffer;
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------ BitmapScanline ------------------ //
Leviathan::ScaleableFreeTypeBitmap::BitmapVerticalLine::BitmapVerticalLine(int xnumber) : VerticalLineData(){
	NthLineFromLeft = xnumber;
}
