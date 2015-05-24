#pragma once
#ifndef _WIN32
#error Don't include WindowsInclude.h on platforms that aren't windows
#endif //_WIN32

// We need to disable this warning to stop stdint and instsafe causing errors //
#pragma warning (disable: 4005)

// This fixes including wincoded.h
#include <d2d1.h>
#include <wincodec.h>
#include <SDKDDKVer.h>

#include <Windows.h>
#include <Windowsx.h>

// Some undefines //
#undef GetNextSibling
#undef GetFirstChild

#undef min
#undef max
