// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include <atomic>
#include <iostream>
#include <typeinfo>

namespace Leviathan{

template<class T>
class InstanceCounter{
public:
    
    InstanceCounter(){

        ++InstanceCount;
        std::cout << typeid(T).name() << " created. total alive: " << InstanceCount
            << std::endl;
    }

    ~InstanceCounter(){

        --InstanceCount;
        std::cout << typeid(T).name() << " destroyed. total alive: " << InstanceCount
            << std::endl;
    }

protected:

    static std::atomic<int32_t> InstanceCount;
};

template<class T>
std::atomic<int32_t> InstanceCounter<T>::InstanceCount { 0 };

}

#ifdef LEAK_INTO_GLOBAL
using Leviathan::InstanceCounter;
#endif // LEAK_INTO_GLOBAL

