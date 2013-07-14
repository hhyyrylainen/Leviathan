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

#define SCALEABLEBITMAP_INTERNAL_TRIM_RESULT_NODATA		0
#define SCALEABLEBITMAP_INTERNAL_TRIM_RESULT_TRIMMED	1

namespace Leviathan{

	class ScaleableFreeTypeBitmap : public Object{
		struct BitmapVerticalLine{
			BitmapVerticalLine(int xnumber, int expectedy);


			inline int LineTrimming();


			inline UCHAR* GetYvalue(const int &y){
				// calculate actual y value in the vector //
				int yactual = y-LineStart;


				// check for negativity //
				if(yactual < 0){
					// we need to adjust this lines' start //
					int shiftamount = abs(yactual);
					// change start //
					LineStart -= shiftamount;

					// insert empty elements to beginning //
					VerticalLineData.insert(VerticalLineData.begin(), (size_t)shiftamount, (UCHAR)0);
					// actual y value should now be the first index //
					yactual = 0;
				}

				// check valid size //
				if((int)VerticalLineData.size() <= yactual){
					// needs to resize to allow more elements //
					VerticalLineData.resize(yactual+1, 0);
				}

				// return reference to right index //
				return &VerticalLineData[yactual];
			}

			inline UCHAR GetColourAtY(const int &y){
				if(VerticalLineData.size() == 0)
					// doesn't have any data //
					return 0;

				// return the value from the vector or 0 if outside range //
				const int yactual = y-LineStart;

				if(yactual < 0 || yactual >= (int)VerticalLineData.size()){
					// outside range //
					return 0;
				}
				return VerticalLineData[yactual];
			}

			void CopyDataFromOther(const BitmapVerticalLine &other);	

			vector<UCHAR> VerticalLineData;
			int NthLineFromLeft;
			int LineStart;
		};
	public:
		// the resulting image in DXGI_FORMAT_R8_UNORM format and initially zeroed out //
		DLLEXPORT ScaleableFreeTypeBitmap::ScaleableFreeTypeBitmap(const int &initialwidth, const int &initialheight);
		DLLEXPORT ScaleableFreeTypeBitmap::~ScaleableFreeTypeBitmap();

		DLLEXPORT bool RenderOtherIntoThis(ScaleableFreeTypeBitmap* img, const int &StartX, const int &StartY);
		DLLEXPORT void RenderFTBitmapIntoThis(const int &xstart, const int &ystart, FT_Bitmap &bmap);
		DLLEXPORT char* GenerateDDSToMemory(size_t &GeneratedSize, int &baselineinimage);

		DLLEXPORT int GetHeightOfLastLine();

		DLLEXPORT void MakeSizeDividableBy2();

		DLLEXPORT inline UCHAR GetPixelDataAtPos(const int &x, const int &y){

			BitmapVerticalLine* line = GetXLineIfExists(x);

			return line != NULL ? line->GetColourAtY(y): 0;
		}

		DLLEXPORT inline BitmapVerticalLine* GetXLine(const int &x){
			// find the right line //
			for(size_t i = 0; i < BitmapData.size(); i++){
				if(BitmapData[i]->NthLineFromLeft == x)
					return BitmapData[i];
			}
			// add new line //
			BitmapData.push_back(new BitmapVerticalLine(x, GetHeightOfLastLine()));
			return BitmapData.back();
		}

		DLLEXPORT inline BitmapVerticalLine* GetXLineIfExists(const int &x){
			// find the right line //
			for(size_t i = 0; i < BitmapData.size(); i++){
				if(BitmapData[i]->NthLineFromLeft == x)
					return BitmapData[i];
			}
			// not found, return NULL //
			return NULL;
		}

		DLLEXPORT inline void SetBaseLine(const int &baseline){
			BaseLineFromBitmapTop = baseline;
		}

		DLLEXPORT inline void CopySizeToVal(Int2 &RenderedToBox){
			// copy values (don't care if they haven't been calculated) //
			RenderedToBox.X = MaxWidth;
			RenderedToBox.Y = MaxHeight;
		}

		// calculates various things about the bitmap like min y max y and such //
		DLLEXPORT void UpdateStats();

		// removes empty spaces reducing the total size //
		DLLEXPORT void RemoveEmptyBits();

		// data getting functions //
		DLLEXPORT inline int GetWidth(){
			// hopefully it is calculated //
			return MaxWidth;
		}
		DLLEXPORT inline int GetHeight(){

			return MaxHeight;
		}

		DLLEXPORT inline int GetBaseLineHeight(){
			// math stolen form rendering to memory function //
			if(MinYValue < 0){
				return BaseLineFromBitmapTop+abs(MinYValue);
			} else {
				return BaseLineFromBitmapTop+MinYValue;
			}
		}

	private:

		// ------------------------------------ //
		vector<BitmapVerticalLine*> BitmapData;

		int MinYValue;
		int MaxYValue;
		int MaxXValue;

		int MaxWidth;
		int MaxHeight;

		int BaseLineFromBitmapTop;
	};

}
#endif