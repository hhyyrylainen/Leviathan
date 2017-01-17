#pragma once
// ------------------------------------ //
#include "../Common/Types.h"

namespace Leviathan {

class IteratorPositionData {
public:
    //IteratorPositionData(size_t val1, size_t val2) : Positions(val1, val2) {

    //}
    IteratorPositionData(){
    }

    StartEndIndex Positions;

};

class IteratorFindUntilData {
public:
    IteratorFindUntilData() {
    }

    StartEndIndex Positions;
    bool FoundEnd = false;
    bool NewLineBreak = false;
};


class IteratorNumberFindData {
public:
    IteratorNumberFindData() :
        DigitsFound(0), DecimalFound(false), NegativeFound(false) {

    }

    // data //
    StartEndIndex Positions;
    int DigitsFound;
    bool DecimalFound;
    bool NegativeFound;
};

class IteratorAssignmentData {
public:
    IteratorAssignmentData() : SeparatorFound(false) {
    }


    StartEndIndex Positions;
    bool SeparatorFound;
};

class IteratorCharacterData {
public:
    IteratorCharacterData(int chartouse) : CharacterToUse(chartouse) {
    }

    int CharacterToUse;
};

template<class StrType>
class IteratorUntilSequenceData {
public:
    IteratorUntilSequenceData(const StrType &finduntil) :
        StringToMatch(finduntil), CurMatchedIndex(0), EndFound(false) {

    }

    StrType StringToMatch;
    size_t CurMatchedIndex;
    StartEndIndex Positions;
    bool EndFound;
};

class IteratorNestingLevelData {
public:
    IteratorNestingLevelData() : NestingLevel(0) {
    }

    StartEndIndex Positions;
    int NestingLevel;
};

}


