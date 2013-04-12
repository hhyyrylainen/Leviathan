#ifndef LEVIATHAN_IDFACTORY
#define LEVIATHAN_IDFACTORY
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{

	class IDFactory : public Object{
	public:
		DLLEXPORT IDFactory::IDFactory();
		DLLEXPORT IDFactory::~IDFactory();


		DLLEXPORT static int GetID();
		DLLEXPORT static int GetSystemID();

	private:
		static void Wait();

		// -------------- //
		static int SystemID;
		static int GlobalID;

		static bool Busy;


	};

}
#endif