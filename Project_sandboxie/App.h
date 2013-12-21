#ifndef SANDBOXIE_APP
#define SANDBOXIE_APP
// ------------------------------------ //
#ifndef SANDBOXIE_INCLUDE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace SandBoxie{

	class App : public Leviathan::LeviathanApplication{
	public:
		App();

		void CustomizeEnginePostLoad();
	};

}
// ------------------------------------ //

#endif
