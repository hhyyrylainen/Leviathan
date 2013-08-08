#ifndef LEVIATHAN_GUI_TEXTLABEL
#define LEVIATHAN_GUI_TEXTLABEL
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "AutoUpdateable.h"
#include "AnimateableGui.h"
#include "GuiPositionable.h"
#include "BaseGuiObject.h"
#include "GuiBaseGraphicalComponent.h"
#include "RenderableGuiObject.h"
#include "BaseGuiEventable.h"

#define TEXTLABEL_OBJECTFLAGS (GUIOBJECTHAS_BASE | GUIOBJECTHAS_POSITIONABLE | GUIOBJECTHAS_RENDERABLE | GUIOBJECTHAS_EVENTABLE | GUIOBJECTHAS_ANIMATEABLE)
#define TEXTLABEL_GRAPHICALCOMPONENT_COUNT		2

namespace Leviathan{ namespace Gui{

	class GuiManager;

	class TextLabel : public BaseGuiObject, public AutoUpdateableObject, public GuiAnimateable, public RenderableGuiObject, 
		public Positionable, public BaseGuiEventable
	{
	public:
		DLLEXPORT TextLabel::TextLabel(GuiManager* owner, const int &id, const bool &hidden, const Float2 &position, Float2 &size, 
			const int &autoadjust, shared_ptr<ScriptScript> script = NULL);
		DLLEXPORT virtual TextLabel::~TextLabel();

		DLLEXPORT bool Init(const wstring &text, const wstring &font, const Float4 &textcolor, const float &textsize, const Float2 &padding, 
			const float &textscalecut, shared_ptr<NamedVariableList> backgroundgen, vector<shared_ptr<VariableBlock>>* listenindexes = NULL);

		DLLEXPORT void Render(Graphics* graph);

		DLLEXPORT int OnEvent(Event** pEvent);

		// this is passed to animation manager for handling //
		DLLEXPORT int AnimationTime(int mspassed); 
		DLLEXPORT void AnimationFinish();

		DLLEXPORT void SetValue(const int &semanticid, const float &val);
		DLLEXPORT float GetValue(const int &semanticid) const;

		// static loading method //
		DLLEXPORT static bool LoadFromFileStructure(GuiManager* owner, vector<BaseGuiObject*> &tempobjects, vector<Int2> &idmappairs, 
			ObjectFileObject& dataforthis);

		// public proxies //
		REFERENCECOUNTED_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(TextLabel);
		GUIANIMATEABLE_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(TextLabel);

	protected:
		void _SetHiddenStates(bool states);
		virtual int _RunAnimationTimeDefault(GuiManager* owner, const int &mspassed);
		void SizeAdjust();
		// function that gets called when size or location changes (used to call autoadjust) //
		virtual void _OnLocationOrSizeChange();
		// ------------------------- //
		Float2 TextPadding;

		int AutoAdjust;
		int TextAdjustMode;

		float TextWantedCutSize;
	};

}}
#endif