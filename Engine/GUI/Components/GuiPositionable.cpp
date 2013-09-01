#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUIPOSITIONABLE
#include "GuiPositionable.h"
#endif
using namespace Leviathan;
using namespace Leviathan::Gui;
// ------------------------------------ //
DLLEXPORT Leviathan::Gui::Positionable::Positionable(){
	// let default constructors apply some junk to values //
	// actually no, set type //
	CoordType = GUI_POSITIONABLE_COORDTYPE_RELATIVE;
}

DLLEXPORT Leviathan::Gui::Positionable::Positionable(const Float2 &position, const Float2 &size) : Position(position), Size(size){
	// positions are definitely updated //
	PositionsUpdated = true;

	// set type //
	CoordType = GUI_POSITIONABLE_COORDTYPE_RELATIVE;
}

// ------------------------------------ //
void Leviathan::Gui::Positionable::_OnLocationOrSizeChange(){
	// default function, does nothing //
	return;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::Positionable::SetLocationData(const Float2 &position, const Float2 &size){
	// positions are updated now //
	PositionsUpdated = true;

	Position = position;
	Size = size;
	// notify this object //
	_OnLocationOrSizeChange();
}

DLLEXPORT void Leviathan::Gui::Positionable::SetLocationData(float x, float y, float width, float heigth){
	// positions are updated now //
	PositionsUpdated = true;

	Position.X = x;
	Position.Y = y;
	Size.X = width;
	Size.Y = heigth;
	// notify this object //
	_OnLocationOrSizeChange();
}

DLLEXPORT void Leviathan::Gui::Positionable::SetSize(const Float2 &size){
	// positions are updated now //
	PositionsUpdated = true;

	Size = size;
	// notify this object //
	_OnLocationOrSizeChange();
}

DLLEXPORT void Leviathan::Gui::Positionable::SetSize(float width, float heigth){
	// positions are updated now //
	PositionsUpdated = true;

	Size.X = width;
	Size.Y = heigth;
	// notify this object //
	_OnLocationOrSizeChange();
}

DLLEXPORT void Leviathan::Gui::Positionable::SetPosition(const Float2 &position){
	// positions are updated now //
	PositionsUpdated = true;

	Position = position;
	// notify this object //
	_OnLocationOrSizeChange();
}

DLLEXPORT void Leviathan::Gui::Positionable::SetPosition(float x, float y){
	// positions are updated now //
	PositionsUpdated = true;

	Position.X = x;
	Position.Y = y;
	// notify this object //
	_OnLocationOrSizeChange();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::Positionable::GetLocation(Float2 &posreceiver, Float2 &sizereceiver){
	// set values to receiving values //
	posreceiver = Position;
	sizereceiver = Size;
}

DLLEXPORT void Leviathan::Gui::Positionable::GetSize(Float2 &sizereceiver){
	// set values to receiving values //
	sizereceiver = Size;
}

DLLEXPORT void Leviathan::Gui::Positionable::GetPosition(Float2 &posreceiver){
	// set values to receiving values //
	posreceiver = Position;
}

// ------------------------------------ //


