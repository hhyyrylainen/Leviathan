#ifndef LEVIATHAN_RENDER_ACTION
#define LEVIATHAN_RENDER_ACTION
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#define RENDERACTION_DELETE -1
#define RENDERACTION_CREATE 0
#define RENDERACTION_HIDDEN 1
#define RENDERACTION_UPDATE 2
#define RENDERACTION_SHOW   3

#define RENDERACTION_SQUARE 10
#define RENDERACTION_TEXT 11

namespace Leviathan{

	class RenderAction : public Object{
	public:
		DLLEXPORT RenderAction::RenderAction();
		DLLEXPORT RenderAction::RenderAction(int type, vector<float>* coordinates, vector<Float4>* colors, wstring instruction, int special, int selfid, int saveslot, void* dataptr);
		DLLEXPORT RenderAction::RenderAction(int type, vector<float>* coordinates, vector<Float4>* colors, wstring instruction, int special, int selfid, int saveslot, void* dataptr, wstring extra);
        DLLEXPORT RenderAction::~RenderAction();

		int Type;
		int SpecsInstr;
		vector<float>* Coords;
		vector<Float4>* Colors;

		wstring Instuction;
		wstring Extrainstr;

		void* DataPtr;

		int FromID;
		int StoreSlot;

	private:

	};

}
#endif