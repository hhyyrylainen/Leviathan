#ifndef LEVIATHAN_GUIBASICTEXT
#define LEVIATHAN_GUIBASICTEXT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "GuiBaseGraphicalComponent.h"
#include "GuiPositionable.h"

namespace Leviathan{ namespace Gui{

#define GUI_BASICTEXT_MODE_JUSTRENDER		0
#define GUI_BASICTEXT_MODE_TRYTOAUTOFIT		1

	class GuiBasicText : public BaseGraphicalComponent, public Positionable{
	public:
		DLLEXPORT GuiBasicText::GuiBasicText(int slot, int zorder);
		DLLEXPORT virtual GuiBasicText::~GuiBasicText();

		// initialize function used to create the various backgrounds //
		// init variant for gradient //
		DLLEXPORT bool Init(const Float4 &colour, const wstring &text, const wstring &font, float textmodifier, bool expensivetext = false, 
			int adjustmode = GUI_BASICTEXT_MODE_JUSTRENDER, const float &cutfromscale = 0.4f);


		DLLEXPORT bool UpdateText(const wstring &texttoset, bool expensivetext = false, int adjustmode = GUI_BASICTEXT_MODE_JUSTRENDER);

		// release //
		DLLEXPORT virtual void Release(RenderBridge* bridge);

		// rendering function which updates the rendering bridge when needed //
		DLLEXPORT virtual void Render(RenderBridge* bridge, Graphics* graph);

		

		// returns (and or calculates text length) //
		DLLEXPORT bool GetTextLength(float &lengthreceiver, float &heightreceiver);

	protected:
		// if text is wanted to be auto scaled to fit box this function does that //
		void _CheckTextAdjustment();
		virtual void _OnLocationOrSizeChange();

		// ------------------------------------ //

		Float4 PrimaryTextColour;

		int TextAdjustMode;

		// type flag //
		bool IsExpensiveText;
		bool NeedsAdjusting;

		wstring Text;
		wstring Font;

		float TextLength;
		float TextHeigth;
		float TextCutScale;

		float TextModifier;

		int TextID;

		bool OldExpensiveState;
	};

}}
#endif