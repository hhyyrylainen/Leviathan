#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include <memory>
#include <iostream>


#include "OGRE/OgreQuaternion.h"
#include "OGRE/OgreColourValue.h"
#include "OGRE/OgreVector3.h"
#include "OGRE/OgreVector4.h"

namespace Leviathan{
#error fix this file
		DLLEXPORT Float3(const Ogre::Vector3 &vec){
			// copy values //
			X = vec.x;
			Y = vec.y;
			Z = vec.z;
		}

		DLLEXPORT inline operator Ogre::Vector3() const{
			return Ogre::Vector3(X, Y, Z);
		}

		DLLEXPORT Float4(const Ogre::Quaternion &quat){
			// copy values //
			X = quat.x;
			Y = quat.y;
			Z = quat.z;
			W = quat.w;
		}

		DLLEXPORT inline operator Ogre::Quaternion() const{

			return Ogre::Quaternion(W, X, Y, Z);
		}

		DLLEXPORT inline operator Ogre::ColourValue() const{

			return Ogre::ColourValue(X, Y, Z, W);
		}
		DLLEXPORT inline operator Ogre::Vector4() const{

			return Ogre::Vector4(X, Y, Z, W);
		}

}


