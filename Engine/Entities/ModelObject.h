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

		DLLEXPORT virtual bool Render(Graphics* renderer, int mspassed);

		DLLEXPORT bool VerifyResourcesLoaded(Graphics* renderer);

		DLLEXPORT void SetTexturesToLoad(vector<shared_ptr<wstring>> files, MultiFlag flags);
		DLLEXPORT void SetModelToLoad(const wstring &file);

		DLLEXPORT bool Init();
		DLLEXPORT void Release();

		// animations //

		// animated textures //

		// static utility //
		DLLEXPORT static int GetFlagFromTextureTypeName(const wstring &name);
		DLLEXPORT static wstring TextureFlagToTypeName(int flag);
	protected:

		void CheckTextures();
		void CheckModelObject();
		// ------------------------ //
		bool Inited;

		wstring ModelPath;
		vector<shared_ptr<wstring>> TexturePath;
		vector<int> TextureIDS;
		vector<int> TextureTypes;


		// pointer to model object //

		// animation data //
	};

}}
#endif