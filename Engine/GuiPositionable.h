#ifndef LEVIATHAN_GUIPOSITIONABLE
#define LEVIATHAN_GUIPOSITIONABLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
namespace Leviathan{ namespace Gui{


#define GUI_POSITIONABLE_COORDTYPE_RELATIVE		0

	class Positionable{
	public:
		DLLEXPORT Positionable::Positionable();
		DLLEXPORT Positionable::Positionable(const Float2 &position, const Float2 &size);

		// getting functions for positions //
		DLLEXPORT void GetPosition(Float2 &posreceiver);
		DLLEXPORT void GetSize(Float2 &sizereceiver);
		DLLEXPORT void GetLocation(Float2 &posreceiver, Float2 &sizereceiver);


		// setting functions for positions //
		DLLEXPORT void SetPosition(float x, float y);

		DLLEXPORT void SetSize(float width, float heigth);

		DLLEXPORT void SetLocationData(float x, float y, float width, float heigth);

		DLLEXPORT void SetPosition(const Float2 &position);

		DLLEXPORT void SetSize(const Float2 &size);

		DLLEXPORT void SetLocationData(const Float2 &position, const Float2 &size);

	protected:
		// function that is called when positions get updated //
		virtual void _OnLocationOrSizeChange();

		// position data //
		Float2 Position;
		Float2 Size;
		// types for these coordinates //
		int CoordType;

		bool PositionsUpdated;
	};

}}
#endif