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
		IteratorPositionData(int val1, int val2) : Positions(val1, val2){

		}
		IteratorPositionData() : Positions(){}

		Int2 Positions;

	};

	class IteratorFindUntilData : public Object{
	public:
		IteratorFindUntilData(int val1, int val2) : Positions(val1, val2), FoundEnd(false){

		}
		IteratorFindUntilData() : Positions(), FoundEnd(false){}

		Int2 Positions;
		bool FoundEnd;
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

	class IteratorAssignmentData : public Object{
	public:
		Int2 Positions;

		bool SeparatorFound : 1;
	};

	class IteratorCharacterData : public Object{
	public:
		IteratorCharacterData(wchar_t chartouse) : CharacterToUse(chartouse){
		}

		wchar_t CharacterToUse;
	};

}
#endif