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

	enum BACKGROUNDTYPE{BACKGROUNDTYPE_NONE, BACKGROUNDTYPE_GRADIENT};


	struct GradientBackgroundData{
		GradientBackgroundData(const Float4 &colour1, const Float4 &colour2, int type);

		Float4 Colour1;
		Float4 Colour2;
		// type of gradient, matches the one in coloured quad //
		int GradientType;

		// flag to determine if it is created //
		bool RBridgeObjectCreated;
	};


	class ObjectBackground : public BaseGraphicalComponent, public Positionable{
	public:
		DLLEXPORT ObjectBackground::ObjectBackground(int slot, int zorder);
		DLLEXPORT virtual ObjectBackground::~ObjectBackground();

		// initialize function used to create the various backgrounds //
		// init variant for gradient //
		DLLEXPORT bool Init(const Float4 &colour1, const Float4 &colour2, int gradient);


		DLLEXPORT bool UpdateGradient(const Float4 &colour1, const Float4 &colour2, int gradient);

		// release //
		DLLEXPORT virtual void Release(RenderBridge* bridge);

		// rendering function which updates the rendering bridge when needed //
		DLLEXPORT virtual void Render(RenderBridge* bridge, Graphics* graph);

		
	protected:
		// used to detect background's repositioning and update the rendering part afterwards //
		virtual void _OnLocationOrSizeChange();
		void _UnAllocateAllBackgroundData();
		// ------------------------------------ //

		// holders of possible data //
		BACKGROUNDTYPE WhichType;

		GradientBackgroundData* GData;
	};

}}
#endif