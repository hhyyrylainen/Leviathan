// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
// Basically include this file in your program to add specific symbols that should trigger
// Nvidia or AMD graphics card to be preferred over intel integrated graphics
// See:
// https://stackoverflow.com/questions/16823372/forcing-machine-to-use-dedicated-graphics-card

extern "C" {
#ifdef _WIN32
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
#else
// There's no evidence that these do anything on other platforms but these are included just in
// case
unsigned long NvOptimusEnablement = 0x00000001;
int AmdPowerXpressRequestHighPerformance = 1;
#endif //_WIN32
}
