#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_DDSHANDLER
#include "DDsHandler.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DDSHandler::DDSHandler(){

}
DDSHandler::~DDSHandler(){

}
// ------------------------------------ //
void DDSHandler::WriteDDSFromGrayScale(wstring& file, vector<vector<unsigned char>> &data, int width, int height/*, DWORD datatype*/){

	ofstream writer;
	writer.open(Convert::WstringToString(file), ios::binary);

	//vector<char> FileBytes;

	int BitsPerPixel = 24;
	int Pitch = (width * BitsPerPixel + 7)/8;

	// magic number //
	char magic[] = {'D', 'D', 'S', ' '};

	writer.write(magic, 4);

	DDS_HEADER header;
	ZeroMemory(&header, sizeof(header));
	header.dwSize = sizeof(DDS_HEADER);
	header.dwFlags = 0x1 | 0x2 | 0x4 | 0x1000;
	header.dwHeight = height;
	header.dwWidth = width;
	header.dwPitchOrLinearSize = Pitch;
	header.ddspf = DDSPF_R8G8B8;
	header.dwCaps = DDS_SURFACE_FLAGS_TEXTURE;
	//header.dwCaps2 = 

	writer.write((char*)&header, sizeof(header));

	//for(int x = 0; x < width; x++){
	//	for(int y = 0; y < height; y++){
	//		int value = data[y][x];
	//		char rgb[] = {value, value, value};

	//		writer.write(rgb, sizeof(char)*3);
	//	}
	//}

	//BYTE** Image;
	//Image = new BYTE*[data.size()];
	//for(int i = 0; i < data.size(); i++){
	//	Image[i] = new BYTE[data[0].size()*3];
	//}
	BYTE Color[3];


	unsigned int y = 0;
	unsigned int x = 0;
	for(y = 0; y < data.size(); y++){
		for(x = 0; x < data[y].size(); x++){
			int red = 255, green = 255, blue = 255;
			
			// sample grayscale to rgb
			red = (int)data[y][x];
			green = (int)data[y][x];
			blue = (int)data[y][x];

			Color[0] = (BYTE)red;
			Color[1] = (BYTE)green;
			Color[2] = (BYTE)blue;

			writer.write((char*)&Color, sizeof(BYTE)*3);
		}
	}

	//for(y = 0; y < data.size(); y++){
	//	writer.write((char*)Image[y], sizeof(Image[y])/sizeof(Image[y][0]));
	//}
	//// put 2 empty blocks //
	//char rgb[] = {0, 0, 0};

	//writer.write(rgb, sizeof(char)*3);

	//writer.write(FileBytes._Myfirst, FileBytes.size());
	writer.close();

	//// release byte array //
	//for(int i = 0; i < data.size(); i++){
	//	delete Image[i];
	//}
	//delete Image;
	//return;
}
// ------------------------------------ //
unsigned char* DDSHandler::GenerateRGBDDSToMemory(vector<vector<Int3>>& data, int width, int height, /*DWORD datatype,*/ int& resultedamount){
	
	unsigned char* buffer = NULL;

	int BitsPerPixel = 24;
	int Pitch = (width * BitsPerPixel + 7)/8;

	// magic number //
	//char magic[] = {'D', 'D', 'S', ' '};


	DDS_HEADER header;
	ZeroMemory(&header, sizeof(header));
	header.dwSize = sizeof(DDS_HEADER);
	header.dwFlags = 0x1 | 0x2 | 0x4 | 0x1000;
	header.dwHeight = height;
	header.dwWidth = width;
	header.dwPitchOrLinearSize = Pitch;
	header.ddspf = DDSPF_R8G8B8;
	header.dwCaps = DDS_SURFACE_FLAGS_TEXTURE;

	// calculate total size //
	int size = 0;
	size += 4; // magic chars //
	size += sizeof(header)/sizeof(unsigned char); // calculate how many characters it takes to represent dds header //
	size += 3*(width*height); // 3 bits(chars) per pixel that represent rgb values //

	// return value for size //
	resultedamount = size;
	buffer = new unsigned char[size];

	int bufferindex = 0;
	// put magic //
	buffer[0] = 'D';
	buffer[1] = 'D';
	buffer[2] = 'S';
	buffer[3] = ' ';

	bufferindex = 4;

	// copy header //
	memcpy(&buffer[bufferindex], (char*)&header, sizeof(header));

	// put pointer to right pos //
	bufferindex += sizeof(header)/sizeof(unsigned char);

	unsigned int y = 0;
	unsigned int x = 0;
	for(y = 0; y < data.size(); y++){
		for(x = 0; x < data[y].size(); x++){
			//int red = 255, green = 255, blue = 255;
			//
			//// sample gray scale to rgb
			//red = data[y][x][0];
			//green = data[y][x][1];
			//blue = data[y][x][2];

			//Color[0] = (BYTE)red;
			//Color[1] = (BYTE)green;
			//Color[2] = (BYTE)blue;
			for(int thrind = 0; thrind < 3; thrind++){

				buffer[bufferindex] = (unsigned char)data[y][x][2-thrind]; // goes through all 3 values in Int3
				bufferindex++;
			}

		}
	}
	// secretly save it //
	//ofstream writer;
	//writer.open("out.dds", ios::binary);

	//writer.write((char*)buffer, size);

	//writer.close();

	return buffer;
}
// ------------------------------------ //

// ------------------------------------ //