#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_TEXTURE_GENERATOR
#include "TextureGenerator.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
TextureGenerator::TextureGenerator(){

}
// ------------------------------------ //

ManagedTexture* TextureGenerator::GenerateCheckerBoard(int width, int height, unsigned int colours, int tilesperside, vector<Int3>& colors, ID3D11Device* dev){
	// lil hack, just call the other function //
	return GenerateCheckerBoard(width, height, colours, tilesperside, tilesperside, colors, dev);
}

ManagedTexture* TextureGenerator::GenerateCheckerBoard(int width, int height, unsigned int colours, int tilesperwidth, int tilesperheight, vector<Int3>& colors, ID3D11Device* dev){
	ManagedTexture* result = NULL;

	// basic checks to see if data is somewhat ok
	if((height % 2 != 0) | (width % 2 != 0)){

		Logger::Get()->Error(L"TextureGenerator: GenerateCheckerBoard: height and width MUST be power of 2 (divisable by 2) width : "+Convert::IntToWstring(width)+L" height: "+Convert::IntToWstring(height));
		return result;
	}
	// colour count //
	if(colours != colors.size()){
		Logger::Get()->Error(L"TextureGenerator: GenerateCheckerBoard: colours doesn't match colours vector's size vec: "+Convert::IntToWstring(colors.size())+L" param: "+Convert::IntToWstring(colours));
		return result;
	}
	// decrease colours so that it matches vector indexes //
	colours--;

	vector<vector<Int3>> Pixels;

	// generate actual colours //

	// resize vectors //
	Pixels.resize(height);
	// add vertical pixel space //
	for(int i = 0; i < height; i++){
		Pixels[i].resize(width);
	}

	int widthpixelspertile = width/tilesperwidth;
	int heightpixelspertile = height/tilesperheight;


	// go through each pixel and set it's color //
	for(int y = 0; y < height; y++){
		for(int x = 0; x < width; x++){
			int red = 255, green = 255, blue = 255;

			// get which height tile this pixel is //
			int htile = y/heightpixelspertile;
			int wtile = x/widthpixelspertile;

			int color = wtile;
			// add height tiles to this to get checkerboard //
			color += htile % 2;

			// move through colours until tile number reaches 0 //
			unsigned int colorindex = 0;
			while(color > 0){
				colorindex++;
				color--;
				// loop if goes too high //
				if(colorindex > colours)
					colorindex = 0;
			}

			// get data from input array //
			red = (int)(colors[colorindex][0]);
			green = (int)(colors[colorindex][1]);
			blue = (int)(colors[colorindex][2]);

			Pixels[y][x] = Int3(red,green,blue);
		}
	}


	int charsinbuffer = -1;
	unique_ptr<unsigned char> tempbuffer(DDSHandler::GenerateRGBDDSToMemory(Pixels,width,height/*, DDS_RGB*/, charsinbuffer));
	if(tempbuffer == NULL){

		Logger::Get()->Error(L"TextureGenerator: GenerateCheckerBoard: failed to write memory buffer: DDSHandler::GenerateRGBDDSToMemory(Pixels,width,height, DDS_RGB, charsinbuffer)");
		return result;
	}

	
	result = new ManagedTexture(tempbuffer.get(), charsinbuffer, IDFactory::GetID(), L"TextureGenerator::Checkerboard", dev, TEXTURETYPE_NORMAL);
	return result;
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //