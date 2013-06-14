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
#include "DataBlock.h"

namespace Leviathan{ namespace Gui{

	class TextLabel : public AutoUpdateableObject, public GuiAnimateable{
	public:
		DLLEXPORT TextLabel::TextLabel(int id);
		DLLEXPORT TextLabel::~TextLabel();

		DLLEXPORT bool Init(int xpos, int ypos, /*bool isposabsolute,*/ int width, int height, wstring text, Float4 color1, Float4 color2, 
			Float4 textcolor, float textsize = 1.0f,
			bool autoadjust = true,wstring font = L"font", int autofetchid = -1);
		DLLEXPORT void Release(Graphics* graph);

		DLLEXPORT void Render(Graphics* graph);

		DLLEXPORT void UpdateColours(Float4 color1, Float4 color2, Float4 textcolor);
		DLLEXPORT void Update(int xpos, int ypos, int width = -1, int height = -1, bool autoadjust = true ,wstring text = L"");
		DLLEXPORT void SetHiddenState(bool hidden);

		DLLEXPORT void SizeAdjust();

		//DLLEXPORT virtual void StartMonitoring(int valueid, bool nonid, wstring = L"");
		//DLLEXPORT virtual void StopMonitoring();

		//DLLEXPORT virtual bool OnUpdate(int newval, bool isint, wstring val);


		DLLEXPORT int AnimationTime(int mspassed); // this is passed to animation manager for handling

		DLLEXPORT void AnimationFinish();
		DLLEXPORT void QueueAction(AnimationAction* act);

		DLLEXPORT int OnEvent(Event** pEvent);

		//DLLEXPORT void RegisterForEvent(EVENT_TYPE toregister);
		//DLLEXPORT void UnRegister(EVENT_TYPE from, bool all = false);


		//DLLEXPORT static ScriptCaller* GetCallerForObjectType(TextLabel* customize);

		DLLEXPORT void SetValue(int semanticid, float val);
		DLLEXPORT float GetValue(int semanticid);

		vector<AnimationAction*> Queue;

		//static ScriptCaller* StaticCall;

		//DLLEXPORT static void QueueActionForObject(TextLabel* object, AnimationAction* action); // removed from base class

	private:

		//void CalculateRelativePositions();

		// ------------------------- //

		wstring LText;
		int X;
		int Y;
		int Width;
		int Height;

		int RelativedX;
		int RelativedY;
		int RelativedWidth;
		int RelativedHeight;

		int RelativedPadding;
		int RelativedYPadding;

		float RelaTivedTextMod;

		int TextPadding;
		int TextPaddingY;

		bool Updated;
		bool BridgeCreated;
		bool QuadCreated;
		bool TextCreated;
		bool TextHidden;
		bool OldHidden;

		bool AreAbsolutePos;

		int TextID;
		int TextLength;
		wstring Font;

		bool AutoAdjust;
		float TextMod;

		bool AutoFetch;

		// rendering bridge //
		shared_ptr<RenderBridge> RBridge;


		//int* SpesData;

		Float4 TextColour;
		Float4 Colour1;
		Float4 Colour2;
	};

}}
#endif