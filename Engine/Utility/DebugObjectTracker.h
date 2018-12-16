// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Convert.h"

#include <unordered_map>

namespace Leviathan {

//! \brief This can check the "this" pointer that the class' constructor was called on it
//!
//! This should be able to catch cases were wrong type of pointer is passed to an object
//!
/*! Quick usage for copy-paste:
ThisPointerTypeChecker<std::remove_pointer_t<decltype(this)>>::NotifyConstructed(this);
ThisPointerTypeChecker<std::remove_pointer_t<decltype(this)>>::NotifyDestructed(this);
ThisPointerTypeChecker<std::remove_pointer_t<decltype(this)>>::CheckThis(this);
*/
template<class T>
class ThisPointerTypeChecker {
public:
    static void NotifyConstructed(T* object)
    {
        ConstructedObjectsOfType[object] = true;
    }

    static void NotifyDestructed(T* object)
    {
        ConstructedObjectsOfType[object] = false;
    }

    static void CheckThis(T* object)
    {
        const auto found = ConstructedObjectsOfType.find(object);

        if(found == ConstructedObjectsOfType.end() || found->second == false) {
            LOG_ERROR("ThisPointerTypeChecker: problem detected, pointer is wrong: " +
                      Convert::ToHexadecimalString(object));
            DEBUG_BREAK;
        }
    }

private:
    static std::unordered_map<T*, bool> ConstructedObjectsOfType;
};

template<class T>
std::unordered_map<T*, bool> ThisPointerTypeChecker<T>::ConstructedObjectsOfType{};


} // namespace Leviathan
