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
		IteratorFindUntilData() : Positions(-1, -1), FoundEnd(false){}

		Int2 Positions;
		bool FoundEnd;
	};


	class IteratorNumberFindData : public Object{
	public:
		IteratorNumberFindData() : Positions(-1, -1), DigitsFound(0), DecimalFound(false), NegativeFound(false){
		}

		// data //
		Int2 Positions;
		int DigitsFound;
		bool DecimalFound;
		bool NegativeFound;
	};

	class IteratorAssignmentData : public Object{
	public:
		IteratorAssignmentData() : Positions(-1, -1) : SeparatorFound(false){
		}


		Int2 Positions;
		bool SeparatorFound;
	};

	class IteratorCharacterData : public Object{
	public:
		IteratorCharacterData(int chartouse) : CharacterToUse(chartouse){
		}

		int CharacterToUse;
	};

}
#endif