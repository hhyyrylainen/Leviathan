#ifndef LEVIATHAN_RENDERING_GUIRBLOB
#define LEVIATHAN_RENDERING_GUIRBLOB
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "GUI\Components\GuiPositionable.h"
#include "Rendering\RenderingPassInfo.h"
#include "OgreOverlayContainer.h"
#include "OverlayMaster.h"

namespace Leviathan{

	// some forward declarations //
	class Graphics;
	class RenderBridge;

	class RenderingGBlob : public Object{
		friend RenderBridge;
	public:
		DLLEXPORT RenderingGBlob(const int &id, const int &relativez, const int &slotid, const bool &hidden);
		DLLEXPORT virtual ~RenderingGBlob();

		// for automatic rendering //
		DLLEXPORT virtual bool RenderWithOverlay(const int &basezorder, Ogre::OverlayContainer* bridgecontainer, Rendering::OverlayMaster* rendering) = 0;
		DLLEXPORT virtual void SetVisibility(const bool &visible);

		int RelativeZ;

		DLLEXPORT inline string GetNameForSingleObject(){
			return "RendBlob_"+Convert::ToString(ID);
		}

	protected:

		// allows blobs to set their overlay elements hidden //
		virtual void _VisibilityChanged() = 0;
		// ------------------------------------ //
		int SlotID;
		int ID;
		bool Hidden;
	};

	// ------------------ Specialized rendering blobs ------------------ //
	class ColorQuadRendBlob: public RenderingGBlob{
	public:
		DLLEXPORT ColorQuadRendBlob::ColorQuadRendBlob(Graphics* graph, const int &id, const int &relativez, const int &slotid, const bool &hidden);
		DLLEXPORT virtual ColorQuadRendBlob::~ColorQuadRendBlob();

		DLLEXPORT virtual bool RenderWithOverlay(const int &basezorder, Ogre::OverlayContainer* bridgecontainer, Rendering::OverlayMaster* rendering);


		DLLEXPORT void Update(Graphics* graph, const int &relativez, const Float2 &xypos, const Float4 &color, const Float4 &color2, const Float2 &size, 
			int colortranstype, int coordinatetype = GUI_POSITIONABLE_COORDTYPE_RELATIVE);
	protected:

		virtual void _VisibilityChanged();
		// ------------------------------------ //

		// panel which background is used for the gradient //
		Ogre::OverlayContainer* Panel;

		bool Added;
		Ogre::OverlayContainer* OwningContainer;
	};

	class TextRendBlob: public RenderingGBlob{
	public:
		DLLEXPORT TextRendBlob::TextRendBlob(Graphics* graph, const int &id, const int &relativez, const int &slotid, const bool &hidden);
		DLLEXPORT virtual TextRendBlob::~TextRendBlob();

		DLLEXPORT void Update(int relativez, const Float2 &xypos, const Float4 &color, float sizemod, const wstring &text, 
			const wstring &font, int coordtype = GUI_POSITIONABLE_COORDTYPE_RELATIVE, bool fittobox = false, 
			const Float2 box = (Float2)0, const float &adjustcutpercentage = 0.4f);

		DLLEXPORT virtual bool RenderWithOverlay(const int &basezorder, Ogre::OverlayContainer* bridgecontainer, Rendering::OverlayMaster* rendering);



	protected:

		virtual void _VisibilityChanged();
		// ------------------------------------ //
		// text are which is used to render this text //
		Ogre::TextAreaOverlayElement* Text;

		bool Added;
		Ogre::OverlayContainer* OwningContainer;
	};

}
#endif