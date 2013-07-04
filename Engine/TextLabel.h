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
#include "ObjectFileObject.h"

#define TEXTLABEL_AUTOFETCH_MODE_NONE		0
#define TEXTLABEL_AUTOFETCH_MODE_ID			1
#define TEXTLABEL_AUTOFETCH_MODE_NAME		2

namespace Leviathan{ namespace Gui{

	class TextLabel : public AutoUpdateableObject, public GuiAnimateable{
	public:
		DLLEXPORT TextLabel::TextLabel(int id);
		DLLEXPORT TextLabel::~TextLabel();

		DLLEXPORT bool Init(int xpos, int ypos, int width, int height, const wstring &text, const Float4 &color1, const Float4 &color2, 
			const Float4 &textcolor, float textsize = 1.0f, bool autoadjust = true, const wstring &font = L"Arial", int autofetchid = -1, 
			const wstring &autofetchname = L"");

		DLLEXPORT void Release();
		DLLEXPORT void Render(Graphics* graph);

		DLLEXPORT void UpdateColours(const Float4 &color1, const Float4 &color2, const Float4 &textcolor);
		DLLEXPORT void Update(int xpos, int ypos, int width = -1, int height = -1, bool autoadjust = true , const wstring &text = L"");

		DLLEXPORT void SetHiddenState(bool hidden);

		DLLEXPORT void SizeAdjust();

		DLLEXPORT int AnimationTime(int mspassed); // this is passed to animation manager for handling

		DLLEXPORT void AnimationFinish();
		DLLEXPORT void QueueAction(shared_ptr<AnimationAction> act);

		DLLEXPORT int OnEvent(Event** pEvent);

		DLLEXPORT void SetValue(const int &semanticid, const float &val);
		DLLEXPORT float GetValue(const int &semanticid) const;

		// static loading method //
		DLLEXPORT static bool LoadFromFileStructure(vector<BaseGuiObject*> &tempobjects, vector<Int2> &idmappairs, ObjectFileObject& dataforthis);


		//DLLEXPORT static void QueueActionForObject(TextLabel* object, AnimationAction* action); // removed from base class
		// public to allow animation manager to remove finished animations //
		vector<shared_ptr<AnimationAction>> AnimationQueue;
	private:
		void _SetHiddenStates(bool states);
		// ------------------------- //
		wstring LText;
		int X;
		int Y;
		int Width;
		int Height;

		int RelativedPadding;
		int RelativedYPadding;

		float RelaTivedTextMod;

		int TextPadding;
		int TextPaddingY;

		bool Updated : 1;
		bool BridgeCreated : 1;
		bool QuadCreated : 1;
		bool TextCreated : 1;
		bool TextHidden : 1;
		bool OldHidden : 1;
		bool AutoAdjust : 1;
		// automatically getting data //
		int AutoFetch;
		//bool AreAbsolutePos : 1;

		int TextID;
		int TextLength;

		// font name //
		wstring Font;

		// text size modifier //
		float TextMod;

		// rendering bridge //
		shared_ptr<RenderBridge> RBridge;

		Float4 TextColour;
		// gradient background colours //
		Float4 Colour1;
		Float4 Colour2;
	};

}}
#endif