#ifndef LEVIATHAN_KEY
#define LEVIATHAN_KEY
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //

enum KEYSPECIAL{ KEYSPECIAL_NONE = 0, KEYSPECIAL_SHIFT, KEYSPECIAL_ALT, KEYSPECIAL_CTRL, KEYSPECIAL_SHIFT_ALT, KEYSPECIAL_SHIFT_CTRL, KEYSPECIAL_ALT_CTRL , KEYSPECIAL_ALL};

namespace Leviathan{

	// base class to have pointers and house the lookup table for ease of use //
	class BaseKey{
	public:
		virtual inline ~BaseKey(){
		}

	protected:
		static map<int, wstring> KeySpecialStringRepresentationMap;
	};


	template<class T>
	class Key : public BaseKey{
	public:
		DLLEXPORT inline Key<T>(){
			Extras = KEYSPECIAL_NONE;
			Character = (T)L"!";
		}
		DLLEXPORT inline Key<T>(const T &character, const KEYSPECIAL &additional){
			Extras = additional;
			Character = character;
		}
		DLLEXPORT inline ~Key(){
		}

		DLLEXPORT inline bool Match(const Key<T> &other, bool strict = false) const{
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

				DeConstructSpecial(this->Extras, Shift, Alt, Ctrl);
				DeConstructSpecial(other.Extras, Shift2, Alt2, Ctrl2);

				if(((Shift == Shift2) || Shift == 0) && (Alt == Alt2 || Alt == 0) && (Ctrl == Ctrl2 || Ctrl == 0)){
					return true;
				}
				return false;
			}
			return true;
		}

		DLLEXPORT inline wstring GenerateWstringMessage(const int &style = 0){
			// create a string that represents this key //
			if(style == 0){
				// debug string //
				UINT translated = MapVirtualKey((UINT)Character, MAPVK_VK_TO_CHAR);

				wstring resultstr = L"Key["+Convert::ToWstring<wchar_t>((wchar_t)Character)+L":"+Convert::ToWstring(translated)+L"]=";

				resultstr += Convert::ToWstring<T>(Character);
				resultstr += L"(0x"+Convert::ToHexadecimalWstring<T>(Character)+L")+";

				resultstr += KeySpecialStringRepresentationMap[Extras];
				resultstr += L" :trsld_";
				// don't want to add a null terminator to the string //
				if(translated != 0)
					resultstr += Convert::ToWstring<wchar_t>(wchar_t(translated));
				resultstr += L"!";

				return resultstr;
			}
			// various styles from which a key can be easily parsed //
			DEBUG_BREAK;
			return L"error";
		}

		DLLEXPORT bool Match(const T &chara, const KEYSPECIAL &additional, bool strict = false) const{
			return Match(Key<T>(chara, additional), strict);
		}

		DLLEXPORT bool Match(const T &chara) const{
			if(chara != this->Character)
				return false;
			return true;
		}


		DLLEXPORT T GetCharacter() const{
			return Character;
		}
		DLLEXPORT KEYSPECIAL GetAdditional() const{
			return Extras;
		}


		DLLEXPORT void Set(const T &character, const KEYSPECIAL &additional){
			Character = character;
			Extras = additional;
		}

		DLLEXPORT void SetAdditional(const KEYSPECIAL &additional){

			Extras = additional;
		}

	private:
		KEYSPECIAL Extras;
		T Character;


	public:
		DLLEXPORT static KEYSPECIAL ConstructSpecial(bool Shift, bool Alt, bool Ctrl){
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

		DLLEXPORT static void DeConstructSpecial(KEYSPECIAL keyspes, bool &Shift, bool &Alt, bool &Ctrl){
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
		}
	};

}
#endif