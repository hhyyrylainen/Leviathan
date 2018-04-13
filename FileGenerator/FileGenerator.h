#pragma once

// Might just as well include this to have everything //
#include <stdio.h>

// C RunTime Header Files
#include <assert.h>
#include <malloc.h>
#include <math.h>
#include <memory.h>
#include <stdlib.h>
#include <wchar.h>

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

#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <vector>


using namespace std;

class FileGenerator {
public:
    static bool DoJSExtensionGeneration(
        std::string input, std::string output, const std::string& name);
};
