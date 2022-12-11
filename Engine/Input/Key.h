// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/StringOperations.h"
#include "Exceptions.h"
#include "GUI/KeyMapping.h"
#include "Iterators/StringIterator.h"

enum KEYSPECIAL {
    KEYSPECIAL_SHIFT = 0x1,
    KEYSPECIAL_ALT = 0x2,
    KEYSPECIAL_CTRL = 0x4,
    KEYSPECIAL_SUPER = 0x8,
    KEYSPECIAL_CAPS = 0x10
    // = 0x20
    // 0x40
    // 0x80 // first byte full
    // 0x100
    // 0x200
    // 0x400
    // 0x800
    // 0x1000
    // 0x2000
    // 0x4000
    // 0x8000 // second byte full (int range might stop here(
    // 0x10000
    // 0x20000
    // 0x40000
    // 0x80000
    // 0x100000
    // 0x200000
    // 0x400000
    // 0x800000 // third byte full
    // 0x1000000
    // 0x2000000
    // 0x4000000
    // 0x8000000
    // 0x10000000
    // 0x20000000
    // 0x40000000
    // 0x80000000 // fourth byte full (will need QWORD here)
    // 0x100000000
};

namespace Leviathan {

template<class T>
class Key {
public:
    inline Key<T>()
    {
        Extras = 0;
        Character = (T)0;
    }
    inline Key<T>(const T& character, const short& additional)
    {
        Extras = additional;
        Character = character;
    }
    inline ~Key() {}

    inline bool Match(const Key<T>& other, bool strict = false) const
    {
        if(other.Character != this->Character)
            return false;
        if(strict) {
            if(other.Extras != this->Extras)
                return false;
        } else {

            if((Extras & KEYSPECIAL_SHIFT) && !(other.Extras & KEYSPECIAL_SHIFT)) {
                return false;
            }
            if((Extras & KEYSPECIAL_ALT) && !(other.Extras & KEYSPECIAL_ALT)) {
                return false;
            }
            if((Extras & KEYSPECIAL_CTRL) && !(other.Extras & KEYSPECIAL_CTRL)) {
                return false;
            }
        }
        return true;
    }

    inline std::string GenerateStringMessage(const int& style = 0)
    {
        // create a string that represents this key //
        if(style == 0) {
            // debug string //
            std::string resultstr = "Key[" + Convert::ToString((char)Character) + "]=";

            resultstr += Convert::ToString<T>(Character);
            resultstr += "(0x" + Convert::ToHexadecimalString<T>(Character) + ")+";

            return resultstr;
        }
        // various styles from which a key can be easily parsed //
        DEBUG_BREAK;
        return "error";
    }

    inline bool Match(const T& chara, const short& additional, bool strict = false) const
    {
        return Match(Key<T>(chara, additional), strict);
    }

    inline bool Match(const T& chara) const
    {
        if(chara != this->Character)
            return false;
        return true;
    }


    T GetCharacter() const
    {
        return Character;
    }
    short GetAdditional() const
    {
        return Extras;
    }

    void Set(const T& character, const short& additional)
    {
        Character = character;
        Extras = additional;
    }

    void SetAdditional(const short& additional)
    {

        Extras = additional;
    }

    static void DeConstructSpecial(short keyspes, bool& Shift, bool& Alt, bool& Ctrl)
    {

        if(keyspes & KEYSPECIAL_SHIFT)
            Shift = true;
        if(keyspes & KEYSPECIAL_ALT)
            Alt = true;
        if(keyspes & KEYSPECIAL_CTRL)
            Ctrl = true;
    }

    //! \See https://wiki.libsdl.org/SDL_Keycode for key names
    static Key<T> GenerateKeyFromString(const std::string& representation)
    {
        if(representation.size() == 0) {
            // empty, nothing to do //
            return Key<T>((T)0, 0);
        }

        StringIterator itr(representation);

        std::unique_ptr<std::string> str;

        // Allow plus key
        if(representation[0] == '+') {
            itr.MoveToNext();
            str = std::make_unique<std::string>("+");
        } else {

            str = itr.GetUntilNextCharacterOrAll<std::string>('+');
        }

        if(!str) {

            throw Leviathan::InvalidArgument("Key: can't parse: " + representation);
        }

        T character;

        // Only upcase single letters as long key codes have also lowercase letters
        if(str->length() <= 1) {

            auto converted = StringOperations::ToUpperCase<std::string>(*str);

            character = Leviathan::KeyMapping::ConvertStringToKeyCode(converted);

        } else {

            // Detect things like "Keypad +" and make them work
            const auto pos = itr.GetPosition();
            if((pos + 1 >= representation.length() && pos < representation.length() &&
                   representation[pos] == '+') ||
                // This is things like "Keypad ++SHIFT"
                (pos + 1 <= representation.length() && representation[pos] == '+' &&
                    representation[pos + 1] == '+')) {

                character = Leviathan::KeyMapping::ConvertStringToKeyCode(*str + "+");
            } else {

                character = Leviathan::KeyMapping::ConvertStringToKeyCode(*str);
            }
        }

        short special = 0;

        while((str = itr.GetUntilNextCharacterOrAll<std::string>('+')) && (str->size() > 0)) {

            if(StringOperations::CompareInsensitive(*str, std::string("ALT"))) {
                special |= KEYSPECIAL_ALT;
            }
            if(StringOperations::CompareInsensitive(*str, std::string("SHIFT"))) {
                special |= KEYSPECIAL_SHIFT;
            }
            if(StringOperations::CompareInsensitive(*str, std::string("CTRL"))) {
                special |= KEYSPECIAL_CTRL;
            }
            if(StringOperations::CompareInsensitive(*str, std::string("WIN")) ||
                StringOperations::CompareInsensitive(*str, std::string("META")) ||
                StringOperations::CompareInsensitive(*str, std::string("SUPER")) ||
                StringOperations::CompareInsensitive(*str, std::string("GUI"))) {
                special |= KEYSPECIAL_SUPER;
            }
        }

        return Key<T>(character, special);
    }

    std::string GenerateStringFromKey()
    {
        // First the actual key value //
        auto resultstr = Leviathan::KeyMapping::ConvertKeyCodeToString(Character);

        // Add special modifiers //
        if(Extras & KEYSPECIAL_ALT)
            resultstr += "+ALT";
        if(Extras & KEYSPECIAL_CTRL)
            resultstr += "+CTRL";
        if(Extras & KEYSPECIAL_SHIFT)
            resultstr += "+SHIFT";
        if(Extras & KEYSPECIAL_SUPER)
            resultstr += "+META";
        // Result is done //
        return resultstr;
    }

private:
    short Extras;
    T Character;
};

//! Helper for checking multiple keys
template<typename KeyType>
inline bool MatchesAnyKeyInSet(const std::vector<Key<KeyType>> keys, const KeyType& chara,
    const short& additional, bool strict = false)
{
    for(const auto& key : keys)
        if(key.Match(chara, additional, strict))
            return true;
    return false;
}

// This is the most likely type //
typedef Key<int32_t> GKey;

} // namespace Leviathan
