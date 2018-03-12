// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
//! \file Provides functions for converting between Newton dynamics types and Ogre /
//! Leviathan types
#pragma once
// ------------------------------------ //
#include "Common/Types.h"

#include <Newton.h>
#include "OgreMatrix4.h"

namespace Leviathan{

DLLEXPORT inline Ogre::Matrix4 NewtonMatrixToOgre(const dFloat* const matrix){
    // Need to transpose as well
    // needs to convert from d3d style matrix to OpenGL style matrix //
    return Ogre::Matrix4(
        matrix[0], matrix[4], matrix[8], matrix[12],
        matrix[1], matrix[5], matrix[9], matrix[13],
        matrix[2], matrix[6], matrix[10], matrix[14],
        matrix[3], matrix[7], matrix[11], matrix[15]
    );
}

//! \usage Pass to newton like this: const auto& prep = PrepareOgreMatrixForNewton(matrix);
//! NewtonMethod(&prep[0])
DLLEXPORT inline Ogre::Matrix4 PrepareOgreMatrixForNewton(const Ogre::Matrix4 &matrix){
    // Need to just transpose
    return matrix.transpose();
}

//! \brief Grabs the translation from a newton matrix without transposing
DLLEXPORT inline Float3 ExtractNewtonMatrixTranslation(const float (&matrix)[16]){
    // Hopefully this is right. Reference:
    // https://stackoverflow.com/questions/10094634/4x4-matrix-last-element-significance
    return Float3(matrix[12], matrix[13], matrix[14]);
}

}



