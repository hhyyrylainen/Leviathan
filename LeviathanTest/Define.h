#include "LeviathanMainDll.h"
#include "Application/Application.h"
#include "resource.h"

using namespace Leviathan;
#define GAME_VERSION VERSION
#define GAME_VERSIONS VERSIONS

#define BREAKONFAIL


#ifdef BREAKONFAIL
#define TESTFAIL {Failed = true; QUICK_ERROR_MESSAGE; DEBUG_BREAK;}
#else
#define TESTFAIL {Failed = true; QUICK_ERROR_MESSAGE;}
#endif // BREAKONFAIL


// ------------------------------------ //
