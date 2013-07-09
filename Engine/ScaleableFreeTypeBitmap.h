#ifndef LEVIATHAN_SCALEABLEFREETYPEBITMAP
#define LEVIATHAN_SCALEABLEFREETYPEBITMAP
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include <ft2build.h>
#include FT_FREETYPE_H

namespace Leviathan{

	class ScaleableFreeTypeBitmap : public Object{
		struct BitmapVerticalLine{
			BitmapVerticalLine(int xnumber);


			inline UCHAR* GetYvalue(const int &y){



				return NULL;
			}

			vector<UCHAR> VerticalLineData;
			int NthLineFromLeft;
		};
	public:
		// the resulting image in DXGI_FORMAT_R8_UNORM format and initially zeroed out //
		DLLEXPORT ScaleableFreeTypeBitmap::ScaleableFreeTypeBitmap(const int &initialwidth, const int &initialheight);
		DLLEXPORT ScaleableFreeTypeBitmap::~ScaleableFreeTypeBitmap();


		DLLEXPORT void RenderFTBitmapIntoThis(const int &xstart, const int &ystart, FT_Bitmap &bmap);
		DLLEXPORT char* GenerateDDSToMemory(size_t &GeneratedSize);


		DLLEXPORT inline UCHAR GetPixelDataAtPos(const int &x, const int &y){


		}

		DLLEXPORT inline BitmapVerticalLine* GetXLine(const int &x){
			// find the right line //
			for(size_t i = 0; i < BitmapData.size(); i++){
				if(BitmapData[i]->NthLineFromLeft == x)
					return BitmapData[i];
			}
			// add new line //
			BitmapData.push_back(new BitmapVerticalLine(x));
			return BitmapData.back();
		}

	private:
		vector<BitmapVerticalLine*> BitmapData;

		int MaxWidth;
		int MaxHeight;
	};

}
#endif