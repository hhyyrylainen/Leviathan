#ifndef LEVIATHAN_DDSHANDLER
#define LEVIATHAN_DDSHANDLER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //

namespace Leviathan{

	class DDSHandler : public Object{
	public:


		DLLEXPORT static void WriteDDSFromGrayScale(wstring& file, vector<vector<unsigned char>> &data, int width, int height/*, DWORD datatype*/);
		DLLEXPORT static unsigned char* GenerateRGBDDSToMemory(vector<vector<Int3>>& data, int width, int height, /*DWORD datatype,*/ int& resultedamount);

	private:
		DLLEXPORT DDSHandler();
		DLLEXPORT ~DDSHandler();
	};

}
#endif
