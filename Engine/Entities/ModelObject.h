#ifndef LEVIATHAN_OBJECT_MODEL
#define LEVIATHAN_OBJECT_MODEL
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Entities\Bases\BaseObject.h"
#include "Entities\Bases\BaseInitializable.h"
#include "Entities\Bases\BaseRenderable.h"
#include "Entities\Bases\SmoothedPosition.h"
#include "Entities\Bases\BaseScalable.h"
#include "Utility\MultiFlag.h"

namespace Leviathan{ namespace GameObject{

#define MODEL_ERROR_LOADDATAFAIL	450

	class Model : public BaseObject, public BaseRenderable, public BaseInitable, public SmoothedPosition, public BaseScalable{
	public:
		DLLEXPORT Model::Model();
		DLLEXPORT Model::~Model();

		DLLEXPORT bool Init();
		DLLEXPORT void Release();

		DLLEXPORT virtual bool Render(Graphics* renderer, int mspassed);

		DLLEXPORT void SetModelToLoad(const wstring &file);

	protected:

		// ------------------------ //
		bool Inited;

		wstring ModelPath;
	};

}}
#endif