#ifndef LEVIATHAN_KEY
#define LEVIATHAN_KEY
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //

enum KEYSPECIAL{ KEYSPECIAL_NONE = 0, KEYSPECIAL_SHIFT, KEYSPECIAL_ALT, KEYSPECIAL_CTRL, KEYSPECIAL_SHIFT_ALT, KEYSPECIAL_SHIFT_CTRL, KEYSPECIAL_ALT_CTRL , KEYSPECIAL_ALL};

namespace Leviathan{

	class Key : public Object{
	public:
		DLLEXPORT Key::Key();
		DLLEXPORT Key::Key(wchar_t Char, KEYSPECIAL additional);
		DLLEXPORT Key::~Key();

		DLLEXPORT bool Match(Key &other, bool strict);
		DLLEXPORT bool Match(wchar_t chara);
		DLLEXPORT bool Match(wchar_t chara, KEYSPECIAL additional, bool strict);

		DLLEXPORT static KEYSPECIAL ConstructSpecial(bool Shift, bool Alt, bool Ctrl); 
		DLLEXPORT static void DeConstructSpecial(KEYSPECIAL keyspes, bool &Shift, bool &Alt, bool &Ctrl); 

		DLLEXPORT wchar_t GetCharacter();
		DLLEXPORT KEYSPECIAL GetAdditional();

		DLLEXPORT void Set(wchar_t character, KEYSPECIAL additional);

	private:
		KEYSPECIAL Extras;
		wchar_t Character;
	};

}
#endif