#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_KEY
#include "Key.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
Key::Key(){
	Extras = KEYSPECIAL_NONE;
	Character = L'\n';
}
Key::Key(wchar_t Char, KEYSPECIAL additional){
	Extras = additional;
	Character = Char;
}
Key::~Key(){

}
// ------------------------------------ //
bool Key::Match(Key &other, bool strict){
	if(other.Character != this->Character)
		return false;
	if(other.Extras == KEYSPECIAL_ALL)
		return true;
	if(strict){
		if(other.Extras != this->Extras)
			return false;
	} else {
		bool Shift = false;
		bool Alt = false;
		bool Ctrl = false;
		bool Shift2 = false;
		bool Alt2 = false;
		bool Ctrl2 = false;

		DeConstructSpecial(other.Extras, Shift, Alt, Ctrl);
		DeConstructSpecial(this->Extras, Shift2, Alt2, Ctrl2);

		if((Shift == Shift2) && (Alt == Alt2) && (Ctrl == Ctrl2)){
			return true;
		}
		return false;
	}
	return true;
}
bool Key::Match(wchar_t chara){
	if(chara != this->Character)
		return false;
	return true;
}
bool Key::Match(wchar_t chara, KEYSPECIAL additional, bool strict){
	if(chara != this->Character)
		return false;
	if(additional == KEYSPECIAL_ALL)
		return true;
	if(strict){
		if(additional != this->Extras)
			return false;
	} else {
		bool Shift = false;
		bool Alt = false;
		bool Ctrl = false;
		bool Shift2 = false;
		bool Alt2 = false;
		bool Ctrl2 = false;

		DeConstructSpecial(additional, Shift, Alt, Ctrl);
		DeConstructSpecial(this->Extras, Shift2, Alt2, Ctrl2);

		if((Shift == Shift2) && (Alt == Alt2) && (Ctrl == Ctrl2)){
			return true;
		}
	}
	return true;
}
// ------------------------------------ //
wchar_t Key::GetCharacter(){
	return Character;
}
KEYSPECIAL Key::GetAdditional(){
	return Extras;
}
// ------------------------------------ //
void Key::Set(wchar_t character, KEYSPECIAL additional){
	Character = character;
	Extras = additional;
}
// ------------------------------------ //
KEYSPECIAL Key::ConstructSpecial(bool Shift, bool Alt, bool Ctrl){
	if((!Shift) && (!Alt) && (!Ctrl))
		return KEYSPECIAL_NONE;
	if((Shift) && (Alt) && (Ctrl))
		return KEYSPECIAL_ALL;

	if((Shift) && (!Alt) && (!Ctrl))
		return KEYSPECIAL_SHIFT;
	if((!Shift) && (Alt) && (!Ctrl))
		return KEYSPECIAL_ALT;
	if((!Shift) && (!Alt) && (Ctrl))
		return KEYSPECIAL_CTRL;

	if((Shift) && (Alt) && (!Ctrl))
		return KEYSPECIAL_SHIFT_ALT;
	if((!Shift) && (Alt) && (Ctrl))
		return KEYSPECIAL_ALT_CTRL;
	if((Shift) && (!Alt) && (Ctrl))
		return KEYSPECIAL_SHIFT_CTRL;

	return KEYSPECIAL_NONE;
}
void Key::DeConstructSpecial(KEYSPECIAL keyspes, bool &Shift, bool &Alt, bool &Ctrl){
	if(keyspes == KEYSPECIAL_NONE){
		Shift = false;
		Alt = false;
		Ctrl = false;
	}
	if(keyspes == KEYSPECIAL_ALL){
		Shift = true;
		Alt = true;
		Ctrl = true;
	}

	if(keyspes == KEYSPECIAL_SHIFT){
		Shift = true;
		Alt = false;
		Ctrl = false;
	}
	if(keyspes == KEYSPECIAL_ALT){
		Shift = false;
		Alt = true;
		Ctrl = false;
	}
	if(keyspes == KEYSPECIAL_CTRL){
		Shift = false;
		Alt = false;
		Ctrl = true;
	}


	if(keyspes == KEYSPECIAL_SHIFT_ALT){
		Shift = true;
		Alt = true;
		Ctrl = false;
	}
	if(keyspes == KEYSPECIAL_SHIFT_CTRL){
		Shift = true;
		Alt = false;
		Ctrl = true;
	}
	if(keyspes == KEYSPECIAL_ALT_CTRL){
		Shift = false;
		Alt = true;
		Ctrl = true;
	}


	return;
}