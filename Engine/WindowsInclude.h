#pragma once
#ifndef _WIN32
#error Don't include WindowsInclude.h on platforms that aren't windows
#endif //_WIN32

#if UE_BUILD_DEBUG == 1 || UE_BUILD_DEVELOPMENT == 1 || UE_BUILD_TEST == 1 || UE_BUILD_SHIPPING == 1
#include "AllowWindowsPlatformTypes.h"
#endif //LEVIATHAN_UE_PLUGIN

// This fixes including wincoded.h
#include <d2d1.h>
#include <wincodec.h>
#include <SDKDDKVer.h>

#include <Windows.h>
#include <Windowsx.h>

#include <shlobj.h>


#if UE_BUILD_DEBUG == 1 || UE_BUILD_DEVELOPMENT == 1 || UE_BUILD_TEST == 1 || UE_BUILD_SHIPPING == 1
#include "HideWindowsPlatformTypes.h"
#else
// Some undefines //
#undef GetNextSibling
#undef GetFirstChild
#undef GetObject
#undef GetCurrentTime

#undef min
#undef max

// We need to disable this warning to stop stdint and instsafe causing errors //
#pragma warning (disable: 4005)

#endif //LEVIATHAN_UE_PLUGIN


