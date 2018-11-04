#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include <string>
#include <vector>


#include "../ReferenceCounted.h"
#include "Common/DataStoring/DataBlock.h"
#include "Common/ThreadSafe.h"


#ifndef ALTERNATIVE_EXCEPTIONS_FATAL
#include "Exceptions.h"
#endif
#ifdef SFML_PACKETS
#include "SFML/Network/Packet.hpp"
#endif // SFML_PACKETS
#include "ErrorReporter.h"

namespace Leviathan {

//! \brief hosts one or more VariableBlocks keeping only one name for all of them
//! \todo Make this reference counted
//! \todo Make all methods throw exceptions on invalid operations
class NamedVariableList {
    friend NamedVars;

public:
    DLLEXPORT NamedVariableList();
    DLLEXPORT NamedVariableList(const NamedVariableList& other);
    //! Creates an empty block with no values
    DLLEXPORT NamedVariableList(const std::string& name);
    DLLEXPORT NamedVariableList(const std::string& name, VariableBlock* value1);
    DLLEXPORT NamedVariableList(const std::string& name, const VariableBlock& val);
#ifdef LEVIATHAN_USING_ANGELSCRIPT
    DLLEXPORT NamedVariableList(ScriptSafeVariableBlock* const data);
#endif // LEVIATHAN_USING_ANGELSCRIPT

#ifdef SFML_PACKETS
    //! \brief For receiving NamedVariableLists through the network
    DLLEXPORT NamedVariableList(sf::Packet& packet);

    //! \brief For passing NamedVariableLists to other instances through the network
    DLLEXPORT void AddDataToPacket(sf::Packet& packet) const;

#endif // SFML_PACKETS

    //! \warning the vector will be wiped clean after creating new variable
    DLLEXPORT NamedVariableList(
        const std::string& name, std::vector<VariableBlock*> values_willclear);

    DLLEXPORT NamedVariableList(const std::string& line, LErrorReporter* errorreport,
        std::map<std::string, std::shared_ptr<VariableBlock>>* predefined = NULL);

    DLLEXPORT NamedVariableList(const std::string& name, const std::string& valuestr,
        LErrorReporter* errorreport,
        std::map<std::string, std::shared_ptr<VariableBlock>>* predefined = NULL);

    //! \brief Helper function for constructing values
    DLLEXPORT bool ConstructValuesForObject(const std::string& variablestr,
        LErrorReporter* errorreport,
        std::map<std::string, std::shared_ptr<VariableBlock>>* predefined);

    //! \brief Handles a found bracket expression "[...]" parsing it recursively
    //! into values
    //! \return false on parse error
    DLLEXPORT bool RecursiveParseList(std::vector<VariableBlock*>& resultvalues,
        std::unique_ptr<std::string> expression, LErrorReporter* errorreport,
        std::map<std::string, std::shared_ptr<VariableBlock>>* predefined);

    DLLEXPORT ~NamedVariableList();

    //! \brief Returns true if this is valid
    inline bool IsValid() const
    {
        return Name.length() > 0;
    }


    // ------------------------------------ //
    DLLEXPORT void SetValue(const VariableBlock& value1);
    DLLEXPORT void SetValue(VariableBlock* value1);
    DLLEXPORT void SetValue(const std::vector<VariableBlock*>& values);
    DLLEXPORT void SetValue(const int& nindex, const VariableBlock& valuetoset);
    DLLEXPORT void SetValue(const int& nindex, VariableBlock* valuetoset);

    //! \brief Adds a new value
    DLLEXPORT void PushValue(std::unique_ptr<VariableBlock>&& value);

    DLLEXPORT VariableBlock* GetValueDirect();
    DLLEXPORT VariableBlock& GetValue();
    DLLEXPORT VariableBlock* GetValueDirect(size_t nindex);
    DLLEXPORT VariableBlock& GetValue(size_t nindex);
    DLLEXPORT std::vector<VariableBlock*>& GetValues();

    DLLEXPORT size_t GetVariableCount() const;


    DLLEXPORT int GetCommonType() const;
    template<class DBT>
    inline bool CanAllBeCastedToType() const
    {
        if(Datas.size() == 0)
            return false;

        for(size_t i = 0; i < Datas.size(); i++) {
            // check this //
            if(!Datas[i]->IsConversionAllowedNonPtr<DBT>()) {
                return false;
            }
        }
        // all passed, can be casted //
        return true;
    }
    template<class DBT>
    inline bool CanAllBeCastedToType(const int& startindex, const int& endindex) const
    {
        if(Datas.size() == 0)
            return false;
        // check would it go over //
        if((size_t)endindex >= Datas.size())
            return false;

        for(int i = startindex; i < endindex + 1; i++) {
            // check this //
            if(!Datas[(size_t)i]->IsConversionAllowedNonPtr<DBT>()) {
                return false;
            }
        }
        // all passed, can be casted //
        return true;
    }

    DLLEXPORT int GetVariableType() const;
    DLLEXPORT int GetVariableType(const int& nindex) const;

    inline std::string GetName() const
    {
        return Name;
    }
    DLLEXPORT void GetName(std::string& name) const;

    DLLEXPORT void SetName(const std::string& name);
    DLLEXPORT bool CompareName(const std::string& name) const;
    // ------------------------------------ //
    DLLEXPORT std::string ToText(int WhichSeparator = 0, bool AddAllBrackets = false) const;

    // process functions //
    DLLEXPORT static bool ProcessDataDump(const std::string& data,
        std::vector<std::shared_ptr<NamedVariableList>>& vec, LErrorReporter* errorreport,
        std::map<std::string, std::shared_ptr<VariableBlock>>* predefined = nullptr);

    // operators //
    DLLEXPORT NamedVariableList& operator=(const NamedVariableList& other);

    //! Compare values extensively
    //!
    //! If this returns true then the values are the same and assignment would have no visible
    //! effect.
    DLLEXPORT bool operator==(const NamedVariableList& other) const;

    //! \see operator==
    DLLEXPORT bool operator!=(const NamedVariableList& other) const;

    // element access operator //
    DLLEXPORT VariableBlock& operator[](const int& nindex);

    //! \brief Switches values to a new instance
    //! \todo Rewrite this as a move operation
    DLLEXPORT static void SwitchValues(
        NamedVariableList& receiver, NamedVariableList& donator);

private:
    //! Data
    std::vector<VariableBlock*> Datas;

    //! Name
    std::string Name;
};


// holds a vector of NamedVariableLists and provides searching functions //
//! \todo Make all methods throw exceptions on invalid operations
class NamedVars : public ReferenceCounted, public ThreadSafe {
public:
    REFERENCE_COUNTED_PTR_TYPE(NamedVars);

    DLLEXPORT NamedVars();

    //! \brief Constructs a NamedVars by stealing variables from another
    //! \note The other object will be empty after this
    DLLEXPORT NamedVars(NamedVars* stealfrom);

    DLLEXPORT NamedVars(const NamedVars& other);
    //! \todo Allow predefined values
    DLLEXPORT NamedVars(const std::string& datadump, LErrorReporter* errorreport);
    DLLEXPORT NamedVars(const std::vector<std::shared_ptr<NamedVariableList>>& variables);
    DLLEXPORT NamedVars(std::shared_ptr<NamedVariableList> variable);


    //! \param takevariable new allocated ptr that will be owned by this object
    DLLEXPORT NamedVars(NamedVariableList* takevariable);

#ifdef SFML_PACKETS

    //! \brief Loads a NamedVars object from a packet
    DLLEXPORT NamedVars(sf::Packet& packet);

    //! \brief Writes this NamedVars to a packet
    DLLEXPORT void AddDataToPacket(sf::Packet& packet) const;

#endif // SFML_PACKETS

    DLLEXPORT ~NamedVars();
    // ------------------------------------ //

    //! \brief Adds a value
    //! \returns True if created new, false if updated an existing value
    DLLEXPORT bool Add(std::shared_ptr<NamedVariableList> value);
    DLLEXPORT bool SetValue(const std::string& name, const VariableBlock& value1);
    DLLEXPORT bool SetValue(const std::string& name, VariableBlock* value1);
    DLLEXPORT bool SetValue(
        const std::string& name, const std::vector<VariableBlock*>& values);

    DLLEXPORT bool SetValue(NamedVariableList& nameandvalues);

    DLLEXPORT size_t GetValueCount(const std::string& name) const;

    //! \returns True if index is valid
    DLLEXPORT inline bool IsIndexValid(size_t index) const
    {
        return index < Variables.size();
    }

    DLLEXPORT VariableBlock& GetValueNonConst(const std::string& name);

    DLLEXPORT const VariableBlock* GetValue(const std::string& name) const;

    DLLEXPORT bool GetValue(const std::string& name, VariableBlock& receiver) const;

    //! \brief Gets a VariableBlock from the specified index on the block matching name
    DLLEXPORT bool GetValue(
        const std::string& name, const int& index, VariableBlock& receiver) const;

    //! \brief Gets constant value from vector index
    DLLEXPORT bool GetValue(const int& index, VariableBlock& receiver) const;
    DLLEXPORT bool GetValues(
        const std::string& name, std::vector<const VariableBlock*>& receiver) const;



    DLLEXPORT std::shared_ptr<NamedVariableList> GetValueDirect(const std::string& name) const;

    //! \warning You need to make sure that this is valid while the pointer is used
    DLLEXPORT NamedVariableList* GetValueDirectRaw(const std::string& name) const;
    DLLEXPORT NamedVariableList* GetValueDirectRaw(size_t index) const;

    template<class T>
    bool GetValueAndConvertTo(const std::string& name, T& receiver) const
    {
        // use try block to catch all exceptions (not found and conversion fail //
        GUARD_LOCK();
        try {
            auto index = Find(guard, name);

            if(index >= Variables.size())
                return false;

            const VariableBlock* tmpblock = Variables[index]->GetValueDirect();
            if(tmpblock == NULL) {
                return false;
            }
            if(!tmpblock->ConvertAndAssingToVariable<T>(receiver)) {

                // Unallowed NamedVars block conversion
                return false;
            }
        } catch(...) {
            // variable not found / wrong type //
            return false;
        }
        // correct variable has been set //
        return true;
    }

    DLLEXPORT std::vector<VariableBlock*>* GetValues(const std::string& name);

    //! \brief Serializes this object into a string representation
    //! \param lineprefix Appended before each new line
    DLLEXPORT std::string Serialize(const std::string& lineprefix = "") const;

#ifdef LEVIATHAN_USING_ANGELSCRIPT
    // Script accessible functions //

    //! \brief Finds and returns the first value in a list matching name
    //! \warning For use from scripts
    ScriptSafeVariableBlock* GetScriptCompatibleValue(const std::string& name);

    //! For use from scripts
    //!
    //! Decrements value refcount
    bool AddScriptCompatibleValue(ScriptSafeVariableBlock* value);

#endif // LEVIATHAN_USING_ANGELSCRIPT

    // ------------------------------------ //
    DLLEXPORT int GetVariableType(const std::string& name) const;

    DLLEXPORT int GetVariableType(Lock& guard, size_t index) const;

    inline int GetVariableType(size_t index) const
    {
        GUARD_LOCK();
        return GetVariableType(guard, index);
    }

    DLLEXPORT int GetVariableTypeOfAll(const std::string& name) const;

    DLLEXPORT int GetVariableTypeOfAll(Lock& guard, size_t index) const;

    inline int GetVariableTypeOfAll(size_t index) const
    {
        GUARD_LOCK();
        return GetVariableTypeOfAll(guard, index);
    }

    DLLEXPORT std::string GetName(size_t index);
    DLLEXPORT bool GetName(size_t index, std::string& name) const;

    DLLEXPORT void SetName(Lock& guard, size_t index, const std::string& name);

    inline void SetName(size_t index, const std::string& name)
    {
        GUARD_LOCK();
        SetName(guard, index, name);
    }

    DLLEXPORT void SetName(const std::string& oldname, const std::string& name);

    DLLEXPORT bool CompareName(size_t index, const std::string& name) const;
    // ------------------------------------ //
    DLLEXPORT void AddVar(NamedVariableList* newvaluetoadd);
    DLLEXPORT void AddVar(std::shared_ptr<NamedVariableList> values);
    DLLEXPORT void AddVar(const std::string& name, VariableBlock* valuetosteal);
    DLLEXPORT void Remove(size_t index);
    DLLEXPORT void Remove(const std::string& name);
    //! \brief Removes a value with the given name if it exists
    DLLEXPORT void RemoveIfExists(const std::string& name, Lock& guard);
    // ------------------------------------ //
    DLLEXPORT bool LoadVarsFromFile(const std::string& file, LErrorReporter* errorreport);

    DLLEXPORT std::vector<std::shared_ptr<NamedVariableList>>* GetVec();
    DLLEXPORT void SetVec(std::vector<std::shared_ptr<NamedVariableList>>& vec);

    //! \brief Returns the size of the internal variable vector
    DLLEXPORT size_t GetVariableCount() const;

    inline operator bool() const
    {
        return !StateIsInvalid;
    }

    // ------------------------------------ //

    inline size_t Find(const std::string& name) const
    {
        GUARD_LOCK();
        return Find(guard, name);
    }

    DLLEXPORT size_t Find(Lock& guard, const std::string& name) const;
    // ------------------------------------ //
    template<class T>
    bool ShouldAddValueIfNotFoundOrWrongType(const std::string& name)
    {
        size_t index = Find(name);

        if(index >= Variables.size()) {
            // Add //
            return true;
        }
        // Check is type correct //
        if(!Variables[(size_t)index]->CanAllBeCastedToType<T>()) {
            // Incorrect types in the variables //
            return true;
        }

        // No need to do anything //
        return false;
    }

private:
    std::vector<std::shared_ptr<NamedVariableList>> Variables;

    //! If set the input data was invalid and this is in invalid state
    bool StateIsInvalid = false;
};

} // namespace Leviathan

#ifdef LEAK_INTO_GLOBAL
using Leviathan::NamedVariableList;
using Leviathan::NamedVars;
#endif
