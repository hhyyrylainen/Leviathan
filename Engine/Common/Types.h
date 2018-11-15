// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //

#include "StartEndIndex.h"
#include "VectorTypes/Float2.h"
#include "VectorTypes/Float3.h"
#include "VectorTypes/Float4.h"
#include "VectorTypes/Int2.h"
#include "VectorTypes/Int3.h"

#include "OgreVector4.h"

#ifdef LEVIATHAN_USING_OGRE
//! \brief Newton compatible matrix orthogonal check
//! \exception InvalidArgument if the matrix isn't orthogonal
//! \warning The matrix needs to be transposed with PrepareOgreMatrixForNewton
//! \todo Fix this. This doesn't work because I used Float3 here instead of Float4 as that
//! doesn't have Cross or Dot and this seems to claim that not even an identity matrix is
//! orthogonal
void ThrowIfMatrixIsNotOrthogonal(const Ogre::Matrix4& matrix, float tol = 1.0e-4f);
#endif

#ifdef LEAK_INTO_GLOBAL
using Leviathan::Float2;
using Leviathan::Float3;
using Leviathan::Float4;
using Leviathan::Int2;
using Leviathan::Int3;
#endif
