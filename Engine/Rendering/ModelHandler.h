#ifndef LEVIATHAN_RENDERMODELHANDLER
#define LEVIATHAN_RENDERMODELHANDLER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Model.h"
#include "BumpModel.h"

namespace Leviathan{

	class ModelHandler : public EngineComponent{
	public:
		DLLEXPORT ModelHandler::ModelHandler();
        DLLEXPORT ModelHandler::~ModelHandler();


		DLLEXPORT bool Init();
		DLLEXPORT void Release();

		DLLEXPORT bool AddModel(BaseModel* model, int shadernumber, int& returnindex);
		DLLEXPORT void ChangeShader(int returnindex, int newshaderindex);
		DLLEXPORT void RemoveModel(int returnindex);
		DLLEXPORT BaseModel* GetModel(int returnindex);


		DLLEXPORT int GetModelCount() { return Models.size(); };
		DLLEXPORT int GetShaderIndex(int returnindex) { return Modelshadernumber[returnindex]; };

	private:
		vector<BaseModel*> Models;
		vector<int> Modelshadernumber;

	};

}
#endif