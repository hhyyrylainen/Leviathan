#pragma once

// Might just as well include this to have everything //
#include <stdio.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <wchar.h>
#include <math.h>
#include <assert.h>

#ifdef _WIN32
#include <Windows.h>
#include <Windowsx.h>
#include <wincodec.h>
#else
// For making SIGINT work as debug break on linux //
#include <signal.h>
#endif

// Might have to only include this on linux //
#include <limits>

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <list>
#include <algorithm>
#include <string>
#include <vector>


using namespace std;

class FileGenerator{
public:


	static bool DoJSExtensionGeneration(string input, string output);



};
