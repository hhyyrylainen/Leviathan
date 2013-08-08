#ifndef LEVIATHAN_GUIOBJECTBACKGROUND
#define LEVIATHAN_GUIOBJECTBACKGROUND
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "GuiBaseGraphicalComponent.h"
#include "GuiPositionable.h"

namespace Leviathan{ namespace Gui{

	// data holders for different background types //

	enum BACKGROUNDTYPE{BACKGROUNDTYPE_NONE, BACKGROUNDTYPE_GRADIENT, BACKGROUNDTYPE_IMAGE};

	class ObjectBackground;

	// base class for all background data objects, to avoid having multiple types of pointers //
	class BaseBackgroundData{
	public:
		BaseBackgroundData();
		virtual ~BaseBackgroundData();

		virtual RenderingGBlob* CreateBlobForThis(ObjectBackground* vars) = 0;
		virtual void RenderThis(RenderBridge* bridge, Graphics* graph, const size_t index, ObjectBackground* vars) = 0;
		// updates this object based on the variables, currentindex is first index in variables that haven't been processed to get here //
		virtual void Update(NamedVariableList* variables, int currentindex) = 0;

		inline BACKGROUNDTYPE GetType(){
			return ThisTYPE;
		}

	protected:

		// flag to determine if it is created //
		bool RBridgeObjectCreated;
		// type of background, for verifying that background matches with data //
		BACKGROUNDTYPE ThisTYPE;
	};

	// class for creating new instance of specific background data //
	template<BACKGROUNDTYPE BGType>
	class BackgroundDataInstanceCreator{
	public:
		static BaseBackgroundData* CreateInstance(){
			return NULL;
		}
	};


	class GradientBackgroundData : public BaseBackgroundData{
	public:
		GradientBackgroundData();
		~GradientBackgroundData();

		void Set(const Float4 &colour1, const Float4 &colour2, int type);

		virtual RenderingGBlob* CreateBlobForThis(ObjectBackground* vars);
		virtual void RenderThis(RenderBridge* bridge, Graphics* graph, const size_t index, ObjectBackground* vars);
		virtual void Update(NamedVariableList* variables, int currentindex);

	protected:

		// this background type's data //
		Float4 Colour1;
		Float4 Colour2;
		// type of gradient, matches the one in coloured quad //
		int GradientType;
	};

	template<>
	class BackgroundDataInstanceCreator<BACKGROUNDTYPE_GRADIENT>{
	public:
		static BaseBackgroundData* CreateInstance(){
			return new GradientBackgroundData();
		}
	};


	class ObjectBackground : public BaseGraphicalComponent, public Positionable{
		// background data objects are friends for accessing variables //
		friend GradientBackgroundData;
	public:
		DLLEXPORT ObjectBackground::ObjectBackground(int slot, int zorder);
		DLLEXPORT virtual ObjectBackground::~ObjectBackground();

		// initialize function used to create the various backgrounds //
		DLLEXPORT bool Init(shared_ptr<NamedVariableList> variables);
		// HINT: you can skip init if you don't want to use complex types, you can directly call any of the Update methods and it *should* work fine //

		// update function for generic backgrounds //
		DLLEXPORT void Update(NamedVariableList* variables);
		// specific type update functions //
		DLLEXPORT void Update(const Float4 &colour1, const Float4 &colour2, const int &gradient);

		// release (that isn't required since all objects on RenderBridge are destroyed when it is deleted) //
		DLLEXPORT virtual void Release(RenderBridge* bridge);

		// rendering function which updates the rendering bridge when needed //
		DLLEXPORT virtual void Render(RenderBridge* bridge, Graphics* graph);
		
	protected:
		// used to detect background's repositioning and update the rendering part afterwards //
		virtual void _OnLocationOrSizeChange();
		void _UnAllocateAllBackgroundData();

		template<BACKGROUNDTYPE BType>
		void _EnsureBackgroundDataType(){
			if(WhichType != BType){
				// delete old ones //
				_UnAllocateAllBackgroundData();
				// allocate new //

				BData = BackgroundDataInstanceCreator<BType>::CreateInstance();
				// and set type //
				WhichType = BType;
			}
		}

		template<BACKGROUNDTYPE BackType>
		void _Render(){
			// deliberate compile error for finding use of this function instantiation //
			return false;
		}
		// ------------------------------------ //
		// holders of possible data //
		BACKGROUNDTYPE WhichType;

		bool NeedsToUnAllocateRBData;

		// pointer that can hold any type of background data object //
		BaseBackgroundData* BData;
	};

}}
#endif