#ifndef LEVIATHAN_OBJECT_MANAGER
#define LEVIATHAN_OBJECT_MANAGER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "MultiFlag.h"
#include "BaseObject.h"
#include "BaseRenderable.h"

namespace Leviathan{

	class ObjectManager : public EngineComponent{
	public:
		DLLEXPORT ObjectManager::ObjectManager();
		DLLEXPORT ObjectManager::~ObjectManager();

		DLLEXPORT bool Init();
		DLLEXPORT void Release();

		DLLEXPORT vector<BaseRenderable*>* GetRenderableObjects();

		DLLEXPORT bool AddObject(BaseObject* obj);
		DLLEXPORT bool AddObject(shared_ptr<BaseObject> obj);

		DLLEXPORT bool Delete(int id);
		DLLEXPORT bool DeleteByIndex(unsigned int index);

		DLLEXPORT int SearchGetIndex(int id);
		DLLEXPORT const shared_ptr<BaseObject>& Search(int id);

		DLLEXPORT const shared_ptr<BaseObject>& Get(unsigned int index);

		DLLEXPORT inline int GetObjectCount();
	private:
		// funcs //
		void UpdateArrays();



		// ----------------- //
		vector<shared_ptr<BaseObject>> Objects;
		// part of Objects vector //
		vector<BaseRenderable*> RenderObjects;

		static shared_ptr<BaseObject> Nullptr;

		bool Updated;
	};

}
#endif