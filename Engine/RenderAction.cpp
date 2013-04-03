#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RENDER_ACTION
#include "RenderAction.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
RenderAction::RenderAction(){
	Type = -1;
	SpecsInstr = -1;
	
	Instuction = L"";
	Extrainstr = L"";
	DataPtr = NULL;
}
RenderAction::RenderAction(int type, vector<float>* coordinates, vector<Float4>* colors, wstring instruction, int special, int selfid, int saveslot, void* dataptr){
	Type = type;
	SpecsInstr = special;
	
	Instuction = instruction;

	Coords = coordinates;
	Colors = colors;

	FromID = selfid;
	StoreSlot = saveslot;
	Extrainstr = L"";

	DataPtr = dataptr;
}

RenderAction::RenderAction(int type, vector<float>* coordinates, vector<Float4>* colors, wstring instruction, int special, int selfid, int saveslot, void* dataptr, wstring extra){
	Type = type;
	SpecsInstr = special;
	
	Instuction = instruction;

	Coords = coordinates;
	Colors = colors;

	FromID = selfid;
	StoreSlot = saveslot;
	Extrainstr = L"";

	DataPtr = dataptr;

	Extrainstr = extra;

}

RenderAction::~RenderAction(){
	// delete vectors //

}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //