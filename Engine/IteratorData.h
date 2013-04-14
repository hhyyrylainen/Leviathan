#ifndef LEVIATHAN_ITERATORDATA
#define LEVIATHAN_ITERATORDATA
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{

	class IteratorPositionData : public Object{
	public:
		Int2 Positions;

	};

	class IteratorNumberFindData : public Object{
	public:
		IteratorNumberFindData();

		// data //
		Int2 Positions;
		int DigitsFound;
		bool DecimalFound : 1;
		bool NegativeFound : 1;
	};

}
#endif