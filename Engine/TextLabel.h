#ifndef LEVIATHAN_GUI_TEXTLABEL
#define LEVIATHAN_GUI_TEXTLABEL
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ResolutionScaling.h"
#include "AutoUpdateable.h"
#include "AnimateableGui.h"
#include "ObjectFileObject.h"
#include "GuiPositionable.h"
#include "GuiBaseGraphicalComponent.h"

namespace Leviathan{ namespace Gui{

	class TextLabel : public AutoUpdateableObject, public GuiAnimateable, public Leviathan::Gui::Positionable{
	public:
		DLLEXPORT TextLabel::TextLabel(int id, const Float2 &position, Float2 &size, int autoadjust);
		DLLEXPORT TextLabel::~TextLabel();

		DLLEXPORT bool Init(const wstring &text, const wstring &font, const Float4 &textcolor, float textsize, 
			const Float4 &color1, const Float4 &color2, vector<shared_ptr<VariableBlock>>* listenindexes = NULL);
		DLLEXPORT void Release();


		DLLEXPORT void Render(Graphics* graph);


		DLLEXPORT bool UpdateBackgroundColours(const Float4 &colour1, const Float4 &colour2, int gradienttype);
		DLLEXPORT bool UpdateText(const wstring &text, bool isexpensive = false);


		DLLEXPORT void SetHiddenState(bool hidden);
		DLLEXPORT void SizeAdjust();

		DLLEXPORT int OnEvent(Event** pEvent);

		DLLEXPORT int AnimationTime(int mspassed); // this is passed to animation manager for handling
		DLLEXPORT void AnimationFinish();
		DLLEXPORT void QueueAction(shared_ptr<AnimationAction> act);

		DLLEXPORT void SetValue(const int &semanticid, const float &val);
		DLLEXPORT float GetValue(const int &semanticid) const;

		// static loading method //
		DLLEXPORT static bool LoadFromFileStructure(vector<BaseGuiObject*> &tempobjects, vector<Int2> &idmappairs, ObjectFileObject& dataforthis);

		//DLLEXPORT static void QueueActionForObject(TextLabel* object, AnimationAction* action); // removed from base class
		// public to allow animation manager to remove finished animations //
		vector<shared_ptr<AnimationAction>> AnimationQueue;
	protected:
		void _SetHiddenStates(bool states);
		// function that gets called when size or location changes (used to call autoadjust) //
		virtual void _OnLocationOrSizeChange();
		// ------------------------- //
		float TextPadding;
		float TextPaddingY;

		bool Updated : 1;
		bool BridgeCreated : 1;
		bool OldHidden : 1;

		int AutoAdjust;
		int TextAdjustMode;

		Float2 TextWantedCoordinates;
		Float2 TextAreaSize;

		// graphical components used to easily reuse graphical parts //
		vector<BaseGraphicalComponent*> GComponents;

		//bool AreAbsolutePos : 1;

		// rendering bridge //
		shared_ptr<RenderBridge> RBridge;
	};

}}
#endif