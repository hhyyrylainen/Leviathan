#ifndef LEVIATHAN_OBJECT_MODEL
#define LEVIATHAN_OBJECT_MODEL
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "MultiFlag.h"
#include "BaseObject.h"
#include "BaseModelDataObject.h"
#include "BaseInitializable.h"
#include "BaseRenderable.h"
#include "SmoothedPosition.h"
#include "BaseScalable.h"
#include "SkeletonRig.h"
#include "AnimationBlock.h"

//#include <Assimp/scene.h>

namespace Leviathan{ namespace GameObject{

#define MODEL_TEXTURETYPE_NORMAL	FLAG_GOBJECT_MODEL_TEXTURETYPE_NORMAL
#define MODEL_TEXTURETYPE_BUMP		FLAG_GOBJECT_MODEL_TEXTURETYPE_BUMP
#define MODEL_TEXTURETYPE_LIGHT		FLAG_GOBJECT_MODEL_TEXTURETYPE_LIGHT
#define MODEL_TEXTURETYPE_BLENDMAP	FLAG_GOBJECT_MODEL_TEXTURETYPE_BLENDMAP

#define MODEL_NEEDED_SHADER_ERROR			-1
#define MODEL_NEEDED_SHADER_MULTITEXTURE	50
#define MODEL_NEEDED_SHADER_TEXTURE			60
#define MODEL_NEEDED_SHADER_LIGHTSHADER		70
#define MODEL_NEEDED_SHADER_BUMPSHADER		80
#define MODEL_NEEDED_SHADER_GRADIENTSHADER	90
//#define MODEL_NEEDED_SHADER_

#define MODEL_ERROR_LOADDATAFAIL	450


	class Model : public BaseObject, public BaseRenderable, public BaseInitable, public SmoothedPosition, public BaseScalable{
	public:
		DLLEXPORT Model::Model();
		DLLEXPORT Model::~Model();

		DLLEXPORT bool Render(Graphics* renderer, int mspassed, const RenderingPassInfo &info, D3DXMATRIX &ViewMatrix, D3DXMATRIX &ProjectionMatrix, D3DXMATRIX &WorldMatrix, D3DXMATRIX &TranslateMatrix, Float3 CameraPos);

		DLLEXPORT bool VerifyResourcesLoaded(Graphics* renderer);

		DLLEXPORT void SetTexturesToLoad(vector<shared_ptr<wstring>> files, MultiFlag flags);
		DLLEXPORT void SetModelToLoad(wstring &file, MultiFlag flags);
		DLLEXPORT bool Init();
		DLLEXPORT void Release();

		DLLEXPORT void SetPos(int x, int y, int z);
		DLLEXPORT void SetOrientation(int pitch, int yaw, int roll);

		DLLEXPORT bool ReloadModel(wstring &file, MultiFlag flags, Graphics* loadnow = NULL);

		// model functions //
		DLLEXPORT int GetIndexCount();

		DLLEXPORT wstring GetModelTypeName();
		DLLEXPORT SkeletonRig* GetSkeleton();

		// animations //
		// skeletal //
		DLLEXPORT bool StartPlayingAnimation(shared_ptr<AnimationBlock> Block, bool Smoothtonew = false);
		DLLEXPORT void StopPlayingAnimations(bool KeepCurrentPose = false);
		DLLEXPORT void FreezeAnimations();
		DLLEXPORT bool UnFreezeAnimations();

		DLLEXPORT bool VerifySkeletonPlayingAnimations();

		// animated textures //

		// static utility //
		DLLEXPORT static int GetFlagFromTextureTypeName(wstring &name);
		DLLEXPORT static wstring TextureFlagToTypeName(int flag);
	protected:
		void PosUpdated();
		void OrientationUpdated();

	//private:
		wstring ModelPath;
		vector<shared_ptr<wstring>> TexturePath;
		vector<int> TextureIDS;
		vector<int> TextureTypes;

		int NeededShader;
		bool TotallyErrored;
		int LastError;

		MODELOBJECT_MODEL_TYPE MType;

		// pointer to model object //
		BaseModelDataObject* ModelDataContainer;

		// animation data //
		shared_ptr<AnimationMasterBlock> CurrentlyPlaying;
		// not required yet //
		shared_ptr<AnimationMasterBlock> SmoothOutTo;

		// ------------------------ //
		bool Inited;

		void ProcessModelFlags(MultiFlag* flags);
		void CheckTextures();

		// model functions //
		void CreateModelObject();

		void ReleaseModel();

		bool InitBuffers(ID3D11Device* device, bool allowrecurse = true);
		void RenderBuffers(ID3D11DeviceContext* devcont);

		bool LoadRenderModel(wstring* file /* ,Graphics* graph*/);


		// static part //
		static bool VecGen;
		static vector<shared_ptr<Flag>> DefFlags;
	};

}}
#endif