// ------------------- //
// This file is part of Leviathan engine. This file is based on Hieroglyph3 Rendering Engine file.
// Files are modified more or less. See below for Original license 
// ------------------- //

//--------------------------------------------------------------------------------
// This file is a portion of the Hieroglyph 3 Rendering Engine.  It is distributed
// under the MIT License, available in the root of this distribution and 
// at the following URL:
//
// http://www.opensource.org/licenses/mit-license.php
//
// Copyright (c) 2003-2010 Jason Zink 
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// Intersector
//
//--------------------------------------------------------------------------------
#ifndef LEVIATHAN_INTERSECTOR
#define LEVIATHAN_INTERSECTOR
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //

namespace Leviathan{

	class Intersector
	{
	public:
		virtual ~Intersector( );
	
		virtual bool Test();
		virtual bool Find();

	protected:
		Intersector( );
	};
};
//--------------------------------------------------------------------------------
#endif // Intersector_h
