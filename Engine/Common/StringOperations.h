// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "../Utility/Convert.h"

#include <memory>
#include <string>
#include <vector>

namespace Leviathan {

constexpr int32_t DOT_CHARACTER = '.';
constexpr int32_t UNIVERSAL_PATHSEPARATOR = '/';
constexpr int32_t WINDOWS_PATHSEPARATOR = '\\';
constexpr int32_t SPACE_CHARACTER = ' ';

constexpr int32_t FIRST_NUMBER = '0';
constexpr int32_t LAST_NUMBER = '9';
constexpr int32_t DASH_CHARACTER = '-';
constexpr int32_t PLUS_SYMBOL = '+';

constexpr char WINDOWS_LINE_SEPARATOR[] = "\r\n";
constexpr char UNIVERSAL_LINE_SEPARATOR[] = "\n";

//! \brief Singleton class that has string processing functions
//!
//! Most functions work with any type of string, but it is recommended to only pass string
//! or wstring to avoid headaches.
//! \todo Get rid of ElementType and just use ints for everything and hope that it is large
//! enough
//! \todo Drop wstring support
class StringOperations {
public:
    template<typename CharType>
    static bool IsCharacterWhitespace(CharType character)
    {
        if((int)character <= 32)
            return true;

        if(IsLineTerminator(character))
            return true;

        return false;
    }

    template<typename CharType>
    static bool IsCharacterQuote(CharType character)
    {
        if(character == '"' || character == '\'')
            return true;

        return false;
    }

    template<typename CharType>
    static inline bool IsCharacterPathSeparator(CharType character)
    {
        if(character == UNIVERSAL_PATHSEPARATOR || character == WINDOWS_PATHSEPARATOR)
            return true;

        return false;
    }

    // Helper functions //

    template<class StringTypeN>
    static void MakeString(StringTypeN& str, const char* characters, size_t count)
    {
        // Skip the null terminator //
        str = StringTypeN(characters, count - 1);
    }

    template<class StringTypeN>
    static StringTypeN RepeatCharacter(int character, size_t count)
    {
        StringTypeN result;

        result.resize(count);

        for(size_t i = 0; i < count; ++i)
            result[i] = character;

        return result;
    }

    // ------------------ Path related operations ------------------ //
    template<class StringTypeN>
    static const StringTypeN RemoveExtension(const StringTypeN& filepath, bool delpath = true)
    {
        size_t startcopy = 0;
        size_t endcopy;

        size_t lastdot = filepath.find_last_of(DOT_CHARACTER);

        if(lastdot == StringTypeN::npos) {
            // no dot //
            endcopy = filepath.size() - 1;
        } else {
            endcopy = lastdot - 1;
        }

        // Potentially erase from beginning //
        if(delpath) {
            // Find last path character //
            size_t lastpath = 0;

            for(size_t i = 0; i < filepath.size(); i++) {
                if(filepath[i] == UNIVERSAL_PATHSEPARATOR ||
                    filepath[i] == WINDOWS_PATHSEPARATOR) {
                    // Currently last found path //
                    lastpath = i;
                }
            }

            if(lastpath != 0) {
                // Set start //
                startcopy = lastpath + 1;
            }
        }

        // Return empty if no data is valid //
        if(startcopy > endcopy || startcopy >= filepath.size() || endcopy >= filepath.size())
            return StringTypeN();

        // return the wanted part //
        return filepath.substr(startcopy, endcopy - startcopy + 1);
    }

    template<class StringTypeN>
    static const StringTypeN GetExtension(const StringTypeN& filepath)
    {
        size_t startcopy = 0;
        size_t endcopy = filepath.size() - 1;

        size_t lastdot = filepath.find_last_of(DOT_CHARACTER);

        if(lastdot == StringTypeN::npos) {
            // no dot //
            return StringTypeN();
        }

        startcopy = lastdot + 1;

        // Return empty if no data is valid //
        if(startcopy > endcopy || startcopy >= filepath.size() || endcopy >= filepath.size())
            return StringTypeN();

        // Return the extension //
        return filepath.substr(startcopy, endcopy - startcopy + 1);
    }

    template<class StringTypeN>
    static const StringTypeN ChangeExtension(
        const StringTypeN& filepath, const StringTypeN& newext)
    {
        size_t startcopy = 0;
        size_t endcopy = filepath.size() - 1;

        size_t lastdot = filepath.find_last_of(DOT_CHARACTER);

        if(lastdot != StringTypeN::npos) {
            // dot found //
            endcopy = lastdot;

        } else {
            // No dot, so just append it //
            return filepath + newext;
        }

        // Return empty if no data is valid //
        if(startcopy > endcopy || startcopy >= filepath.size() || endcopy >= filepath.size())
            return StringTypeN();

        // Return the extension //
        return filepath.substr(startcopy, endcopy - startcopy + 1) + newext;
    }

    template<class StringTypeN>
    static const StringTypeN RemovePath(const StringTypeN& filepath)
    {
        size_t startcopy = 0;
        size_t endcopy = filepath.size() - 1;

        // Find last path character //
        size_t lastpath = 0;

        for(size_t i = 0; i < filepath.size(); i++) {
            if(filepath[i] == UNIVERSAL_PATHSEPARATOR ||
                filepath[i] == WINDOWS_PATHSEPARATOR) {
                // Currently last found path //
                lastpath = i;
            }
        }

        if(lastpath != 0) {
            // Set start //
            startcopy = lastpath + 1;
        }

        // Return empty if no data is valid //
        if(startcopy > endcopy || startcopy >= filepath.size() || endcopy >= filepath.size())
            return StringTypeN();


        // return the wanted part //
        return filepath.substr(startcopy, endcopy - startcopy + 1);
    }

    //! \brief Returns the path part of a path+filename
    template<class StringTypeN>
    static const StringTypeN GetPath(const StringTypeN& filepath)
    {
        size_t startcopy = 0;
        size_t endcopy = filepath.size() - 1;

        // Find last path character //
        size_t lastpath;
        bool found = false;

        for(size_t i = 0; i < filepath.size(); i++) {
            if(filepath[i] == UNIVERSAL_PATHSEPARATOR ||
                filepath[i] == WINDOWS_PATHSEPARATOR) {
                // Currently last found path //
                lastpath = i;
                found = true;
            }
        }

        if(!found) {
            // Set start //
            return StringTypeN();
        }

        // Set up copy //
        endcopy = lastpath;


        // Return empty if no data is valid //
        if(startcopy > endcopy || startcopy >= filepath.size() || endcopy >= filepath.size())
            return StringTypeN();


        // return the wanted part //
        return filepath.substr(startcopy, endcopy - startcopy + 1);
    }

    //! \brief Returns the protocol from a URL
    //! \example URLProtocol("http://google.fi/index.html") = "http"
    DLLEXPORT static std::string URLProtocol(const std::string& url);

    //! \brief Returns the base hostname from a URL
    //! \example BaseHostName("http://google.fi/index.html") = "http://google.fi/"
    DLLEXPORT static std::string BaseHostName(const std::string& url);

    //! \brief Returns the path of an url
    //!
    //! This returns everything after what BaseHostName would have returned
    //! \param stripoptions If true then options are also removed. They are marked with a
    //! question mark like "example.com/file?token=1234". The question mark is also stripped
    //! \example URLPath("http://google.fi/index.html") = "index.html"
    DLLEXPORT static std::string URLPath(const std::string& url, bool stripoptions = true);

    //! \brief Combines a URL with another (relative) URL
    //! \example CombineURL("http://google.fi/index.html", "img.jpg") =
    //! "http://google.fi/img.jpg"
    DLLEXPORT static std::string CombineURL(
        const std::string& first, const std::string& second);

    //! \brief Removes multiple hostnames from an address
    //!
    //! \example RemovePartsBeforeAbsoluteURLParts(
    //! "http://example.com//a.example.com/img.png") = "http://a.example.com/img.png"
    DLLEXPORT static std::string RemovePartsBeforeAbsoluteURLParts(const std::string& url);

    //! \brief Returns true if string looks like a top level domain
    DLLEXPORT static bool IsURLDomain(const std::string& str);


    //! \brief Changes all line separators to Windows line separators
    template<class StringTypeN>
    static const StringTypeN ChangeLineEndsToWindows(const StringTypeN& input)
    {
        StringTypeN results;

        // This is the line ending sequence //
        StringTypeN separator;
        MakeString(separator, WINDOWS_LINE_SEPARATOR, sizeof(WINDOWS_LINE_SEPARATOR));

        // Try to find path strings and replace them //
        StartEndIndex copyparts;

        for(size_t i = 0; i < input.size(); i++) {
            if(input[i] == UNIVERSAL_LINE_SEPARATOR[0]
                // Previous character wasn't the first character of a windows line separator.
                // If it was this is already the correct line separator and should be ignored
                && (i == 0 || input[i - 1] != WINDOWS_LINE_SEPARATOR[0])) {
                // Found a line separator //
                // Copy the current thing //
                if(copyparts.Start && copyparts.End)
                    results += input.substr(copyparts.Start, copyparts.Length());

                copyparts.Reset();

                results += separator;

                continue;
            }

            if(!copyparts.Start)
                copyparts.Start = i;

            // Change the end copy //
            copyparts.End = i;
        }

        if(copyparts.End && copyparts.Start)
            results += input.substr(copyparts.Start, copyparts.Length());

        return results;
    }

    //! \brief Changes all line separators to universal line separators
    template<class StringTypeN>
    static const StringTypeN ChangeLineEndsToUniversal(const StringTypeN& input)
    {
        StringTypeN results;

        // This is the line ending sequence //
        StringTypeN separator;
        MakeString(separator, UNIVERSAL_LINE_SEPARATOR, sizeof(UNIVERSAL_LINE_SEPARATOR));

        // Try to find path strings and replace them //
        size_t copystart = 0;
        size_t copyend = 0;

        for(size_t i = 0; i < input.size(); i++) {
            if(input[i] == WINDOWS_LINE_SEPARATOR[0] && i + 1 < input.size() &&
                input[i + 1] == WINDOWS_LINE_SEPARATOR[1]) {
                // Found a line separator //
                // Copy the current thing //
                if(copyend >= copystart && copystart - copyend > 1)
                    results += input.substr(copystart, copyend - copystart + 1);


                results += separator;

                copystart = i + 2 < input.size() ? i + 2 : i;
                copyend = copystart;

                i += 2;

                continue;
            }

            // Change the end copy //
            copyend = i;
        }

        if(copyend >= copystart && copystart - copyend > 1)
            results += input.substr(copystart, copyend - copystart + 1);


        return results;
    }

    //! \brief Splits a string on line separators
    template<class StringTypeN>
    static size_t CutLines(const StringTypeN& input, std::vector<StringTypeN>& output)
    {
        if(input.empty())
            return 0;

        size_t copystart = 0;
        size_t copyend = 0;

        for(size_t i = 0; i < input.size(); i++) {
            // Check is at a line separator
            bool windows = input[i] == WINDOWS_LINE_SEPARATOR[0] && i + 1 < input.size() &&
                           input[i + 1] == WINDOWS_LINE_SEPARATOR[1];

            if(windows || (input[i] == UNIVERSAL_LINE_SEPARATOR[0])) {
                // Check that previous character wasn't the beginning of
                // a windows line separator. If it was this is already added
                // and should be ignored
                if(i > 0 && input[i - 1] == WINDOWS_LINE_SEPARATOR[0]) {

                    // Skip adding this
                    continue;
                }

                // Found a line separator //
                // Copy the current thing //
                if(copyend >= copystart && copystart - copyend > 1) {

                    output.push_back(input.substr(copystart, copyend - copystart + 1));

                } else {
                    // There was an empty line //
                    output.push_back(StringTypeN());
                }

                copystart = windows ? i + 2 : i + 1;
                copyend = 0;

                continue;
            }

            copyend = i;
        }

        if(copyend >= copystart)
            output.push_back(input.substr(copystart, copyend - copystart + 1));

        return output.size();
    }


    // ------------------ General string operations ------------------ //
    template<class StringTypeN>
    static bool CutString(const StringTypeN& strtocut, const StringTypeN& separator,
        std::vector<StringTypeN>& vec)
    {
        // scan the input and gather positions for string copying //
        std::vector<StartEndIndex> CopyOperations;
        bool PositionStarted = false;

        for(size_t i = 0; i < strtocut.length(); i++) {
            if(!PositionStarted) {
                PositionStarted = true;
                // add new position index //
                CopyOperations.push_back(StartEndIndex(i));
            }

            if(strtocut[i] == separator[0]) {
                // Found a possible match //
                // test further //
                size_t modifier = 0;
                bool WasMatch = false;
                while(strtocut[i + modifier] == separator[modifier]) {
                    // check can it increase without going out of bounds //
                    if((strtocut.length() > i + modifier + 1) &&
                        (separator.length() > modifier + 1)) {
                        // increase modifier to move forward //
                        modifier++;
                    } else {
                        // check is it a match
                        if(modifier + 1 == separator.length()) {
                            // found match! //

                            // end old string to just before this position //
                            CopyOperations.back().End = i; /*-1;
                                                           not this because we would have to
                                                           add 1 in the copy phase anyway */

                            PositionStarted = false;
                            // skip separator //
                            WasMatch = true;
                            break;
                        }
                        break;
                    }
                }

                // skip the separator amount of characters, if it was found //
                if(WasMatch)
                    // -1 here so that first character of next string won't be missing,
                    // because of the loop incrementation
                    i += separator.length() - 1;
            }
        }

        // Return empty string if there is nothing here //
        if(CopyOperations.empty()) {

            vec.push_back(StringTypeN());
            return false;
        }

        // make sure final position has end //
        if(!CopyOperations.back().End)
            CopyOperations.back().End = strtocut.length();

        if(CopyOperations.size() < 2) {

            vec.push_back(strtocut.substr(CopyOperations.front().Start,
                static_cast<size_t>(CopyOperations.front().End) -
                    static_cast<size_t>(CopyOperations.front().Start)));

            // would be just one string, for legacy
            // (actually we don't want caller to think it got cut) reasons we return nothing //
            return false;
        }

        // length-1 is not used here, because it would have to be added in copy phase to the
        // substring length, and we didn't add that earlier...

        // make space //
        vec.reserve(CopyOperations.size());

        // loop through positions and copy substrings to result vector //
        for(size_t i = 0; i < CopyOperations.size(); i++) {
            // copy using std::wstring method for speed //
            vec.push_back(strtocut.substr(
                CopyOperations[i].Start, static_cast<size_t>(CopyOperations[i].End) -
                                             static_cast<size_t>(CopyOperations[i].Start)));
        }

        // cutting succeeded //
        return true;
    }

    template<class StringTypeN>
    static int CountOccuranceInString(const StringTypeN& data, const StringTypeN& lookfor)
    {
        int count = 0;

        for(size_t i = 0; i < data.length(); i++) {
            if(data[i] == lookfor[0]) {
                // Found a possible match //
                // test further //
                size_t modifier = 0;
                bool WasMatch = false;
                while(data[i + modifier] == lookfor[modifier]) {
                    // check can it increase without going out of bounds //
                    if((data.length() > i + modifier + 1) &&
                        (lookfor.length() > modifier + 1)) {
                        // increase modifier to move forward //
                        modifier++;
                    } else {
                        // check is it a match
                        if(modifier + 1 == lookfor.length()) {
                            // found match! //
                            count++;
                            WasMatch = true;
                            break;
                        }
                        break;
                    }
                }
                // skip the separator amount of characters, if it was found //
                if(WasMatch)
                    // -1 here so that first character of next string won't be missing,
                    // because of the loop incrementation
                    i += lookfor.length() - 1;
            }
        }

        return count;
    }

    template<class StringTypeN>
    static StringTypeN Replace(
        const StringTypeN& data, const StringTypeN& toreplace, const StringTypeN& replacer)
    {
        // We construct an output string from the wanted bits //
        StringTypeN out;

        if(toreplace.size() < 1) {
            // Don't replace anything //
            return data;
        }

        PotentiallySetIndex copystart;
        PotentiallySetIndex copyend;

        // loop through data and copy final characters to out string //
        for(size_t i = 0; i < data.size(); i++) {
            // check for replaced part //
            if(data[i] == toreplace[0]) {
                // check for match //
                bool IsMatch = false;
                for(size_t checkind = 0;
                    (checkind < toreplace.size()) && (checkind < data.size()); checkind++) {
                    if(data[i + checkind] != toreplace[checkind]) {
                        // didn't match //
                        break;
                    }
                    // check is final iteration //
                    if(!((checkind + 1 < toreplace.size()) && (checkind + 1 < data.size()))) {
                        // is a match //
                        IsMatch = true;
                        break;
                    }
                }

                if(IsMatch || toreplace.size() == 1) {

                    if(copystart && !copyend)
                        copyend = copystart;

                    // First add proper characters //
                    if(copystart && copyend)
                        out += data.substr(copystart,
                            static_cast<size_t>(copyend) - static_cast<size_t>(copystart) + 1);

                    copystart.ValueSet = false;
                    copyend.ValueSet = false;

                    // it is a match, copy everything in replacer and
                    // add toreplace length to i
                    out += replacer;

                    i += toreplace.length() - 1;
                    continue;
                }
            }

            // non matching character mark as to copy //
            if(!copystart) {
                copystart = i;
            } else {
                copyend = i;
            }
        }

        // Copy rest to out //
        if(copystart && copyend)
            out += data.substr(
                copystart, static_cast<size_t>(copyend) - static_cast<size_t>(copystart) + 1);

        // Return finished string //
        return out;
    }


    template<class StringTypeN>
    static StringTypeN ReplaceSingleCharacter(
        const StringTypeN& data, int toreplace, int replacer = ' ')
    {
        // Copy the string and then modify it //
        StringTypeN out(data);
        ReplaceSingleCharacterInPlace(out, toreplace, replacer);

        return out;
    }

    template<class StringTypeN>
    static void ReplaceSingleCharacterInPlace(
        StringTypeN& data, int toreplace, int replacer = ' ')
    {
        for(auto iter = data.begin(); iter != data.end(); ++iter) {
            if((*iter) == toreplace)
                (*iter) = replacer;
        }
    }

    template<class StringTypeN>
    static StringTypeN RemoveCharacters(const StringTypeN& data, const StringTypeN& toremove)
    {
        StringTypeN out;
        out.reserve(data.size());

        for(auto iter = data.begin(); iter != data.end(); ++iter) {

            // Check does it contain //
            bool ignore = false;

            for(auto iter2 = toremove.begin(); iter2 != toremove.end(); ++iter2) {

                if((*iter2) == (*iter)) {

                    ignore = true;
                    break;
                }
            }

            if(ignore)
                continue;

            out.push_back(*iter);
        }

        return out;
    }

    template<class StringTypeN>
    static StringTypeN RemoveFirstWords(const StringTypeN& data, int amount)
    {
        size_t firstpos = 0;
        // Find the copy start position //
        int spaces = 0;
        int words = 0;

        for(size_t i = 0; i < data.length(); i++) {
            if(data[i] == SPACE_CHARACTER) {
                spaces++;
                continue;
            }
            if(spaces > 0) {
                words++;
                if(words == amount) {
                    // This is the spot we want to start from //
                    firstpos = i;
                    break;
                }
                spaces = 0;
            }
        }

        if(firstpos == 0) {
            // Didn't remove anything? //
            return data;
        }

        // Generate sub string from start to end //
        return data.substr(firstpos, data.size() - firstpos);
    }

    template<class StringTypeN>
    static StringTypeN RemovePrefix(const StringTypeN& data, const StringTypeN& prefix)
    {
        if(data.find(prefix) == 0) {
            return data.substr(prefix.size());
        } else {
            return data;
        }
    }

    template<class StringTypeN>
    static StringTypeN StitchTogether(
        const std::vector<StringTypeN*>& data, const StringTypeN& separator)
    {
        StringTypeN ret;
        bool first = true;
        // reserve space //
        int totalcharacters = 0;

        // This might be faster than not reserving space //
        for(size_t i = 0; i < data.size(); i++) {
            totalcharacters += data[i]->length();
        }

        totalcharacters += separator.length() * data.size();

        // By reserving space we don't have to allocate more memory
        // during copying which might be faster
        ret.reserve(totalcharacters);

        for(size_t i = 0; i < data.size(); i++) {
            if(!first)
                ret += separator;
            ret += *data[i];
            first = false;
        }

        return ret;
    }

    template<class StringTypeN>
    static StringTypeN StitchTogether(
        const std::vector<StringTypeN>& data, const StringTypeN& separator)
    {
        StringTypeN ret;
        bool first = true;
        // reserve space //
        int totalcharacters = 0;

        // This might be faster than not reserving space //
        for(size_t i = 0; i < data.size(); i++) {
            totalcharacters += data[i].length();
        }

        totalcharacters += separator.length() * data.size();

        // By reserving space we don't have to allocate more memory
        // during copying which might be faster
        ret.reserve(totalcharacters);

        for(size_t i = 0; i < data.size(); i++) {
            if(!first)
                ret += separator;
            ret += data[i];
            first = false;
        }

        return ret;
    }


    template<class StringTypeN>
    static StringTypeN StitchTogether(
        const std::vector<std::shared_ptr<StringTypeN>>& data, const StringTypeN& separator)
    {
        StringTypeN ret;
        bool first = true;
        // reserve space //
        int totalcharacters = 0;

        // This might be faster than not reserving space //
        for(size_t i = 0; i < data.size(); i++) {
            totalcharacters += data[i]->length();
        }
        totalcharacters += separator.length() * data.size();

        // By reserving space we don't have to allocate more memory during
        // copying which might be faster
        ret.reserve(totalcharacters);

        for(size_t i = 0; i < data.size(); i++) {
            if(!first)
                ret += separator;
            ret += *data[i].get();
            first = false;
        }

        return ret;
    }

    template<class StringTypeN>
    static void RemovePreceedingTrailingSpaces(StringTypeN& str)
    {
        StartEndIndex CutPositions;

        // search the right part of the string //
        for(size_t i = 0; i < str.size(); i++) {
            if(!IsCharacterWhitespace(str[i])) {
                if(!CutPositions.Start) {
                    // beginning ended //
                    CutPositions.Start = i;
                } else {
                    // set last pos as this //
                }
                continue;
            }
            if(!CutPositions.Start) {
                // still haven't found a character //
                continue;
            }
            // check is this last character //
            size_t a = str.size() - 1;
            bool found = false;
            for(; a > i; a--) {
                if(!IsCharacterWhitespace(str[a])) {
                    // there is still valid characters //
                    found = true;
                    break;
                }
            }
            if(found) {
                // skip to the found non-space character //
                i = a - 1;
                continue;
            }
            // end found //
            CutPositions.End = i - 1;
            break;
        }

        if(!CutPositions.Start) {
            // nothing in the string //
            str.clear();
            return;
        }
        if(!CutPositions.End) {
            if(!CutPositions.Start) {
                // just the first character required //
                CutPositions.End = CutPositions.Start;
            } else {
                // no need to cut from the end //
                CutPositions.End = str.length() - 1;
            }
        }

        // set the wstring as it's sub string //
        str = str.substr(CutPositions.Start, CutPositions.Length());
    }

    template<class StringTypeN>
    static bool CompareInsensitive(const StringTypeN& data, const StringTypeN& second)
    {
        if(data.size() != second.size())
            return false;

        for(unsigned int i = 0; i < data.size(); i++) {
            if(data[i] != second[i]) {

                // Check are they different case //
                if(97 <= data[i] && data[i] <= 122) {

                    if(data[i] - 32 != second[i]) {

                        return false;
                    }
                } else if(97 <= second[i] && second[i] <= 122) {

                    if(second[i] - 32 != data[i]) {

                        return false;
                    }
                } else {

                    return false;
                }
            }
        }

        return true;
    }

    template<class StringTypeN>
    static bool StringStartsWith(const StringTypeN& data, const StringTypeN& tomatch)
    {
        return data.find(tomatch) == 0;
    }

    template<class StringTypeN>
    static bool StringEndsWith(const StringTypeN& data, const StringTypeN& tomatch)
    {
        if(data.size() < tomatch.size())
            return false;

        if(data.empty())
            return true;

        for(size_t i = data.size() - 1, checkIndex = 0; checkIndex < tomatch.size();
            --i, ++checkIndex) {

            if(data[i] != tomatch[tomatch.size() - 1 - checkIndex])
                return false;

            if(i == 0)
                return true;
        }

        return true;
    }

    template<class StringTypeN>
    static bool IsStringNumeric(const StringTypeN& data)
    {
        for(size_t i = 0; i < data.size(); i++) {
            if((data[i] < FIRST_NUMBER || data[i] > LAST_NUMBER) &&
                data[i] != DASH_CHARACTER && data[i] != DOT_CHARACTER &&
                data[i] != PLUS_SYMBOL) {
                return false;
            }
        }
        return true;
    }

    //! \todo Make this work with any unicode characters
    template<class StringTypeN>
    static StringTypeN ToUpperCase(const StringTypeN& data)
    {
        StringTypeN result;
        result.reserve(data.size());

        for(size_t i = 0; i < data.size(); i++) {

            // Not actually unicode decoding...
            auto const codepoint = data[i];

            if(97 <= codepoint && codepoint <= 122) {

                result.push_back(codepoint - 32);

            } else {

                result.push_back(codepoint);
            }
        }

        return result;
    }

    template<class StringTypeN>
    static StringTypeN Indent(size_t numspaces)
    {
        if(!numspaces)
            return StringTypeN();

        return StringTypeN(numspaces, static_cast<int>(' '));
    }

    //! \brief Appends spaces number of spaces to each line in str and returns the result
    template<class StringTypeN>
    static StringTypeN IndentLines(const StringTypeN& str, size_t spaces)
    {
        const auto indentstr = Indent<StringTypeN>(spaces);

        StringTypeN result;
        result.reserve(str.size());

        StartEndIndex currentcut;

        for(size_t i = 0; i < str.size(); ++i) {

            // Check for line change //
            if(IsLineTerminator(str[i])) {

                result += indentstr;

                if(currentcut.Start)
                    result += str.substr(
                        currentcut.Start, i - static_cast<size_t>(currentcut.Start));

                result += "\n";
                currentcut = StartEndIndex();

                // Multi character line terminator //
                if(i + 1 < str.length() && IsLineTerminator(str[i], str[i + 1]))
                    ++i;
            }

            if(!currentcut.Start && !IsCharacterWhitespace(str[i])) {

                // Started a line //
                currentcut.Start = i;
            }
        }

        if(currentcut.Start) {

            currentcut.End = str.size();

            result += indentstr + str.substr(currentcut.Start, currentcut.Length());
        }

        return result;
    }

    template<class StringTypeN>
    static StringTypeN RemoveEnding(const StringTypeN& str, const StringTypeN& ending)
    {
        const auto pos = str.rfind(ending);

        if(pos != StringTypeN::npos && str.length() - pos == ending.length())
            return str.substr(0, pos);

        return str;
    }

    //! \returns True if a character is a line terminating character
    static bool IsLineTerminator(int32_t codepoint)
    {
        if(codepoint == '\r' || codepoint == '\n' ||
            // Unicode newlines //
            codepoint == 0x0085 || codepoint == 0x2028 || codepoint == 0x2029 ||
            codepoint == 0x000B || codepoint == 0x000C) {
            return true;
        }

        return false;
    }

    //! \returns True if two characters are a line terminating sequence
    static bool IsLineTerminator(int32_t codepoint1, int32_t codepoint2)
    {
        if(codepoint1 == '\r' && codepoint2 == '\n') {
            return true;
        }

        return false;
    }

private:
    StringOperations() = delete;
    ~StringOperations() = delete;
};


template<>
DLLEXPORT void StringOperations::MakeString(
    std::wstring& str, const char* characters, size_t count);

} // namespace Leviathan
