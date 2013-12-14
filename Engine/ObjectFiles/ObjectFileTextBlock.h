#ifndef LEVIATHAN_OBJECTFILE_TEXTBLOCK
#define LEVIATHAN_OBJECTFILE_TEXTBLOCK
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //

namespace Leviathan{

	class ObjectFileTextBlock : public Object{
	public:
		DLLEXPORT ObjectFileTextBlock();
		DLLEXPORT ObjectFileTextBlock(const wstring& name);
		DLLEXPORT ~ObjectFileTextBlock();

		wstring Name;
		vector<wstring*> Lines; // for storing plain text //


	};

}
#endif
