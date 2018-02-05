// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#include "Define.h"
// ------------------------------------ //
#include <vector>

namespace Leviathan {
//! used to store function's parameter info
struct FunctionParameterInfo {
    DLLEXPORT FunctionParameterInfo(int id, int sizes) :
        FunctionID(id), ParameterTypeIDS(sizes), ParameterDeclarations(sizes),
        MatchingDataBlockTypes(sizes), ReturnMatchingDataBlock(-1)
    {
    }

    int FunctionID;

    std::vector<int> ParameterTypeIDS;

    // TODO: check are these four still used
    std::vector<std::string> ParameterDeclarations;
    std::vector<int> MatchingDataBlockTypes;
    std::string ReturnTypeDeclaration;
    int ReturnMatchingDataBlock;

    int ReturnTypeID;
};
} // namespace Leviathan
