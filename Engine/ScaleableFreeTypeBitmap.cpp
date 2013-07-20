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

	// reserve space according to it //
	BitmapData.reserve(MaxWidth);
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
DLLEXPORT char* Leviathan::ScaleableFreeTypeBitmap::GenerateDDSToMemory(size_t &GeneratedSize, int &baselineinimage){
	// calculate required size //
	char* buffer = NULL;

	// we need to update the bitmap "stats" //
	UpdateStats();


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

	const DDS_PIXELFORMAT ThisFormat =
	{ sizeof(DDS_PIXELFORMAT), DDS_ALPHA, 0, 8, 0xff, 0xff, 0xff, 0xff};

	//header.ddspf = DDSPF_DX10;
	header.ddspf = ThisFormat;
	//header.dwMipMapCount = 1;
	//header.dwSurfaceFlags = DDS_SURFACE_FLAGS_TEXTURE;

	// set up dx10 header part //
	//dx10header.dxgiFormat = DXGI_FORMAT_R8_UNORM;
	//dx10header.resourceDimension = D3D11_RESOURCE_DIMENSION_TEXTURE2D;
	//dx10header.arraySize = 1;


	// calculate total size //
	GeneratedSize = 0;
	GeneratedSize += 4; // magic chars //
	GeneratedSize += sizeof(header)/sizeof(char); // calculate how many characters it takes to represent dds header //
	//GeneratedSize += sizeof(dx10header)/sizeof(char); // directx 10 and up header defining the specific format //
	GeneratedSize += (MaxWidth*MaxHeight)*((BitsPerPixel/8)/sizeof(char)); // 3 bits(chars) per pixel that represent rgb values //

	// create the buffer //
	buffer = new char[GeneratedSize];
	// zero out the buffer (just so that if something is missed it would be zeros //
	ZeroMemory(buffer, sizeof(char)*GeneratedSize);

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
	//memcpy(&buffer[bufferindex], (char*)&dx10header, sizeof(dx10header));

	// put pointer to right pos //
	//bufferindex += sizeof(dx10header)/sizeof(char);

	size_t pixeldatastart = bufferindex;

	// copy pixel data //
	//for(int x = 0; x < MaxWidth; x++){
	//	// get current vertical line //
	//	BitmapVerticalLine* line = GetXLineIfExists(x);
	//	if(line == NULL){
	//		// no data on line, we can skip because we have zeroed out the buffer //
	//		continue;
	//	}

	//	for(int y = MinYValue; y < MaxYValue; y++){
	//		// write data and move to next //
	//		size_t accessspot = pixeldatastart+x+(y-MinYValue)*MaxWidth;
	//		if(accessspot >= GeneratedSize)
	//			DEBUG_BREAK;

	//		buffer[accessspot] = line->GetColourAtY(y);
	//		//bufferindex = accessspot;
	//		bufferindex++;
	//	}
	//}

	for(int y = 0; y < MaxHeight; y++){
		for(int x = 0; x < MaxWidth; x++){
			// get current vertical line //
			BitmapVerticalLine* line = GetXLine(x);

			int ryval = MinYValue+y;
			// write data and move to next //
			if(line){
				buffer[bufferindex] = line->GetColourAtY(ryval);
			} else {
				buffer[bufferindex] = 0;
			}
			bufferindex++;
		}
	}

	if(bufferindex != GeneratedSize){

		DEBUG_BREAK;
	}

	// set the return value of the baseline //

	// the actual baseline field tells the baseline height from 0 min y, it needs to be adjusted //
	if(MinYValue < 0){
		baselineinimage = BaseLineFromBitmapTop+abs(MinYValue);
	} else {
		baselineinimage = BaseLineFromBitmapTop+MinYValue;
	}

	// just because it can be done, save to file //
	int inumber = 1;

	if(!DataStore::Get()->GetValueAndConvertTo<int>(L"Bitmaprenderingind", inumber)){
		// add new //
		DataStore::Get()->AddVar(new NamedVariableList(L"Bitmaprenderingind", new VariableBlock(inumber)));
	} else {
		// set new value //
		DataStore::Get()->SetValue(L"Bitmaprenderingind", new VariableBlock(inumber+1));
	}

	ofstream writer;

	string file = ".\\renderedtext_"+Convert::ToString<int>(inumber)+".dds";

	writer.open(file, ios::binary);
	if(!writer.is_open()){
		// cannot write //

		DEBUG_BREAK;

	}

	writer.write(buffer, GeneratedSize);

	writer.close();

	// return buffer //
	return buffer;
}

void Leviathan::ScaleableFreeTypeBitmap::UpdateStats(){
	// first calculate width //
	// get the vertical line that has largest distance from beginning //
	MaxXValue = 0;

	// calculate height by getting minimum y and maximum y values //
	MinYValue = 0;
	MaxYValue = 0;

	for(size_t i = 0; i < BitmapData.size(); i++){
		if(BitmapData[i]->NthLineFromLeft > MaxXValue){
			// set new max value //
			MaxXValue = BitmapData[i]->NthLineFromLeft;
		}
		if(BitmapData[i]->LineStart < MinYValue){
			// set new minimum value //
			MinYValue = BitmapData[i]->LineStart;
		}
		// check does this have data past max y (if it does set max y) //
		const int curmaximumyindex = BitmapData[i]->LineStart+BitmapData[i]->VerticalLineData.size()-1;
		if(curmaximumyindex > MaxYValue){

			MaxYValue = curmaximumyindex;
		}
	}
	// calculate width from max x //
	MaxWidth = 1+MaxXValue;

	// calculate height from min and max y values //
	MaxHeight = MaxYValue-MinYValue;
	//if((MinYValue < 0) != (MaxYValue < 0)){

	//	MaxHeight = 1+abs(MinYValue)+abs(MaxYValue);
	//} else {
	//	// same sign, subtract one from the other //
	//	MaxHeight = 1+abs(MinYValue-MaxYValue);
	//}
}
// ------------------------------------ //
DLLEXPORT int Leviathan::ScaleableFreeTypeBitmap::GetHeightOfLastLine(){
	// get size of last line (or zero) //
	if(BitmapData.size() == 0){

		return 0;
	} else {

		return BitmapData.back()->VerticalLineData.size();
	}
}

void Leviathan::ScaleableFreeTypeBitmap::RemoveEmptyBits(){
	// find and remove empty lines, and on lines that aren't empty remove preceding and trailing empty bits and adjust start x to accommodate //
	// make sure that min x and others are up to date //
	UpdateStats();

	int FirstXToHaveData = MaxXValue;
	int LastXToHaveData = 0;


	for(size_t i = 0; i < BitmapData.size(); i++){
		// trim this line //
		if(BitmapData[i]->LineTrimming() == SCALEABLEBITMAP_INTERNAL_TRIM_RESULT_TRIMMED){
			// set as various things //
			int curx = BitmapData[i]->NthLineFromLeft;

			// check can we set new values //
			if(curx < FirstXToHaveData)
				FirstXToHaveData = curx;

			if(curx > LastXToHaveData)
				LastXToHaveData = curx;
		}
	}
	
	// erase lines that are before FirstXToHaveData or after LastXToHaveData //
	for(size_t i = 0; i < BitmapData.size(); i++){
		if(BitmapData[i]->NthLineFromLeft < FirstXToHaveData || BitmapData[i]->NthLineFromLeft > LastXToHaveData){
			// outside wanted range, erase //
			BitmapData.erase(BitmapData.begin()+i);
		}
	}


	// TODO: determine if we want to adjust x values of lines (it can mess up font rendering as advance value is calculated with empty space before 
	// character (if there is any)



}

DLLEXPORT void Leviathan::ScaleableFreeTypeBitmap::MakeSizeDividableBy2(){
	// make sure that width and height are dividable by 2 //
	UpdateStats();

	// check //
	if(MaxWidth % 2 != 0){
		// needs to add one to width //
		// access max width line to create new line //
		GetXLine(MaxWidth);
	}
	if(MaxHeight % 2 != 0){
		// needs to add one to height //
		// access y elements from first line until reached MaxHeight+1 //

		BitmapVerticalLine* line = GetXLine(0);

		while((int)(line->LineStart+line->VerticalLineData.size()) <= MaxHeight){

			line->VerticalLineData.push_back(0);
		}
	}

}

DLLEXPORT bool Leviathan::ScaleableFreeTypeBitmap::RenderOtherIntoThis(ScaleableFreeTypeBitmap* img, const int &StartX, const int &StartY){
	// other needs to have valid calculated values //
	img->UpdateStats();

	// loop over image that is copied and copy data to right positions //
	for(int x = 0; x <= img->MaxXValue; x++){

		BitmapVerticalLine* sourceline = img->GetXLine(x);

		// calculate destination x //
		int DestX = StartX+x;

		BitmapVerticalLine* targetline = GetXLine(DestX);


		targetline->CopyDataFromOther(*sourceline, StartY);
	}
	return true;
}

DLLEXPORT bool Leviathan::ScaleableFreeTypeBitmap::OutPutToFile1And0(const wstring &file){
	// write 1s and 0s to a file //
	ofstream writer;
	writer.open(file);

	if(!writer.is_open()){
		return false;
	}

	this->UpdateStats();

	// go through in horizontal lines and print characters //
	for(int y = 0; y < MaxHeight; y++){
		for(int x = 0; x < MaxWidth; x++){
			// get current vertical line //
			BitmapVerticalLine* line = GetXLine(x);

			int ryval = MinYValue+y;
			// write data and move to next //

			writer << (char)(line->GetColourAtY(ryval) != 0 ? 35: 48);
		}
		// line change //
		writer << endl;
	}


	// close and return true //
	writer.close();
	return true;
}

// ------------------------------------ //

// ------------------ BitmapScanline ------------------ //
Leviathan::ScaleableFreeTypeBitmap::BitmapVerticalLine::BitmapVerticalLine(int xnumber, int expectedy) : VerticalLineData(){
	NthLineFromLeft = xnumber;
	LineStart = 0;

	// reserve data for vector //
	VerticalLineData.reserve(expectedy);
}

int Leviathan::ScaleableFreeTypeBitmap::BitmapVerticalLine::LineTrimming(){
	// remove empty preceding indexes //
	if(VerticalLineData.size() == 0)
		return SCALEABLEBITMAP_INTERNAL_TRIM_RESULT_NODATA;

	while(VerticalLineData.size() != 0 && VerticalLineData[0] == 0){
		// increase start index and erase first //
		LineStart++;

		VerticalLineData.erase(VerticalLineData.begin());
		// check if we erased everything //
		if(VerticalLineData.size() == 0)
			return SCALEABLEBITMAP_INTERNAL_TRIM_RESULT_NODATA;
	}

	// erase from back //
	while(VerticalLineData.back() == 0){

		VerticalLineData.erase(VerticalLineData.begin()+VerticalLineData.size()-1);
		// check if we erased everything //
		if(VerticalLineData.size() == 0)
			return SCALEABLEBITMAP_INTERNAL_TRIM_RESULT_NODATA;
	}
	// trimmed //
	return SCALEABLEBITMAP_INTERNAL_TRIM_RESULT_TRIMMED;
}

void Leviathan::ScaleableFreeTypeBitmap::BitmapVerticalLine::CopyDataFromOther(const BitmapVerticalLine &other, const int &YAdd /*= 0*/){
	// we need to store original line start //
	const int origstart = LineStart;

	// loop other's elements and calculate index in this and copy //
	for(size_t i = 0; i < other.VerticalLineData.size(); i++){
		int tindex = i+(other.LineStart-origstart)+YAdd;

		(*GetYvalue(tindex)) = other.VerticalLineData[i];
	}
}
