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

//----------------------------------------------------------------------------------------------------
// Vector2f
//----------------------------------------------------------------------------------------------------
#ifndef LEVIATHAN_VECTOR2f
#define LEVIATHAN_VECTOR2f
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //

namespace Leviathan{
	class Vector2f
	{
	public:
		Vector2f( );
		Vector2f( float x, float y );
		Vector2f( const Vector2f& Vector );

		// vector operations
		void Clamp( );
		void MakeZero( );
		void Normalize( );
		float Magnitude( );

		// assignment
		Vector2f& operator= ( const Vector2f& Vector );

		// accessors
		float operator[] ( int iPos ) const;
		float& operator[] ( int iPos );

		// boolean comparison
		bool operator== ( const Vector2f& Vector ) const;
		bool operator!= ( const Vector2f& Vector ) const;

		// arithmetic operations
		Vector2f operator+ ( const Vector2f& Vector ) const;
		Vector2f operator- ( const Vector2f& Vector ) const;
		Vector2f operator* ( float fScalar ) const;
		Vector2f operator/ ( float fScalar ) const;
		Vector2f operator- ( ) const;

		// arithmetic updates
		Vector2f& operator+= ( const Vector2f& Vector );
		Vector2f& operator-= ( const Vector2f& Vector );
		Vector2f& operator*= ( float fScalar );
		Vector2f& operator/= ( float fScalar );

	public:
		float x;
		float y;
	};
};
//----------------------------------------------------------------------------------------------------
#endif // Vector2f_h
