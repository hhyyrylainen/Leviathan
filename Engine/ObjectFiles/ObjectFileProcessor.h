// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "ObjectFile.h"
#include "ErrorReporter.h"
#include <string>
#include <memory>

namespace Leviathan{

//! \brief Static class for handling ObjectFiles
class ObjectFileProcessor{
public:

    ObjectFileProcessor() = delete;
    ~ObjectFileProcessor() = delete;
    
    DLLEXPORT static void Initialize();
    DLLEXPORT static void Release();

    //! \brief Reads an ObjectFile to an in-memory data structure
    DLLEXPORT static std::unique_ptr<ObjectFile> ProcessObjectFile(const std::string &file, 
        LErrorReporter* reporterror);

    static DLLEXPORT std::unique_ptr<ObjectFile> ProcessObjectFileFromString(
        const std::string &filecontents, const std::string &filenameforerrors,
        LErrorReporter* reporterror);


    //! \brief Writes an ObjectFile's data structure to a file
    //! \warning Using the process and this function will erase ALL comments,
    //! which is not optimal for config files. It is recommended to only append
    //! to an existing file to keep comments intact
    //! \return True when the file has been written, false if something failed
    DLLEXPORT static bool WriteObjectFile(ObjectFile &data, const std::string &file,
        LErrorReporter* reporterror);

    //! \brief Serializes an ObjectFile object into a string
    //!
    //! The string can be written to a file and later parsed to get the ObjectFile object back
    //! \param receiver The generated text will be appended to this string
    //! \returns True if succeeds, false if the object failed to be serialized for some reason
    DLLEXPORT static bool SerializeObjectFile(ObjectFile &data, std::string &receiver);

    //! \brief Registers a new value alias for the processor
    //! \warning Calling this while files are being parsed will cause undefined behavior
    DLLEXPORT static void RegisterValue(const std::string &name, VariableBlock* valuetokeep);

    // Utility functions //


    // function to shorten value loading in many places //
    template<class T>
		static bool LoadValueFromNamedVars(NamedVars* block, const std::string &varname,
            T &receiver, const T &defaultvalue, LErrorReporter* ReportError = nullptr,
            const std::string &errorprefix = "")
    {
        // try to get value and convert to receiver //
        if(!block->GetValueAndConvertTo<T>(varname, receiver)){
            // variable not found / wrong type //
            // report error if wanted //
            if(ReportError)
                ReportError->Error(errorprefix+" invalid variable "+varname+
                    ", not found/wrong type");
                
            // set value to provided default //
            receiver = defaultvalue;

            return false;
        }
        return true;
    }
        
    // function to call the one before //
    template<class T>
		static FORCE_INLINE bool LoadValueFromNamedVars(NamedVars &block,
            const std::string &varname, T &receiver, const T &defaultvalue,
            LErrorReporter* ReportError = nullptr, const std::string &errorprefix = "")
    {
        return LoadValueFromNamedVars<T>(&block, varname, receiver, defaultvalue,
            ReportError, errorprefix);
    }


    template<class RType, class SingleType, int VarCount>
		static void LoadMultiPartValueFromNamedVars(NamedVars* block,
            const std::string &varname, RType &receiver, const RType &defaultvalue,
            LErrorReporter* ReportError = nullptr, const std::string &errorprefix = "")
    {
        // get pointer to value list //
        std::shared_ptr<NamedVariableList> curvalues = block->GetValueDirect(varname);

        if(curvalues.get() == NULL){
                
            // not found //
            if(ReportError)
                ReportError->Error(errorprefix+" invalid variable "+varname+", not found");
                
            // set as default //
            receiver = defaultvalue;
            return;
        }
            
        // call assigning function //
        int varindex = 0;
        LoadMultiPartValueFromNamedVariableList<RType, SingleType, VarCount>(curvalues.get(),
            varindex, receiver, defaultvalue, ReportError, errorprefix);

    }

    template<class RType, class SingleType, int VarCount>
		static bool LoadMultiPartValueFromNamedVariableList(NamedVariableList* block,
            int &valuestartindex, RType &receiver, const RType &defaultvalue,
            LErrorReporter* ReportError = nullptr, const std::string &errorprefix = "")
    {
        // make sure that size is right and types are correct //
        if(block->GetVariableCount()-valuestartindex < VarCount ||
            !block->CanAllBeCastedToType<SingleType>(
                valuestartindex, valuestartindex+VarCount-1))
        {
            // not enough values / wrong types //
            if(ReportError){
                ReportError->Error(errorprefix+" invalid variable "+block->GetName()+
                    ", not enough values ("+Convert::ToString<int>(VarCount)+
                    " needed) or wrong types");
            }
            // set as default //
            receiver = defaultvalue;
            return false;
        }

        // iterate over how many are wanted and assign //
        for(int i = 0; i < VarCount; i++){

            // convert and set //
            receiver[i] = (SingleType)block->GetValue(valuestartindex+i);
        }
            
        // values copied //
        // increment the index before returning //
        valuestartindex += VarCount;

        return true;
    }

    // function to call the one before //
    template<class RType, class SingleType, int VarCount>
		static FORCE_INLINE void LoadMultiPartValueFromNamedVars(NamedVars &block,
            const std::string &varname, RType &receiver, const RType &defaultvalue,
            LErrorReporter* ReportError = nullptr, const std::string &errorprefix = "")
    {
        return LoadMultiPartValueFromNamedVars<RType, SingleType, VarCount>(&block,
            varname, receiver, defaultvalue, ReportError, errorprefix);
    }


    //! \brief Loads a vector of type from a NamedVars
    template<class T>
		static bool LoadVectorOfTypeUPtrFromNamedVars(NamedVars &block,
            const std::string &varname,
            std::vector<std::unique_ptr<T>> &receiver, size_t mustbedivisableby = 1,
            LErrorReporter* ReportError = nullptr, const std::string &errorprefix = "")
    {
        NamedVariableList* inanimlist = block.GetValueDirectRaw(varname);

        if(inanimlist && inanimlist->GetVariableCount() && inanimlist->GetVariableCount() %
            mustbedivisableby == 0 &&
            inanimlist->CanAllBeCastedToType<T>())
        {

            receiver.reserve(inanimlist->GetVariableCount());

            for(size_t i = 0; i < inanimlist->GetVariableCount(); i++){

                const T tmpresult =
                    inanimlist->GetValueDirect(i)->ConvertAndReturnVariable<T>();

                receiver.push_back(std::move(std::unique_ptr<T>(new T(tmpresult))));
            }

            return true;
        }

        // Something isn't right //
        if(ReportError)
            ReportError->Error(errorprefix+" invalid variable list "+varname+
                ", not found/wrong type/wrong amount");
        return false;
    }

private:

    //! \brief Handling function for NamedVariables
    static std::shared_ptr<NamedVariableList> TryToLoadNamedVariables(const std::string &file,
        StringIterator &itr, const std::string &preceeding, LErrorReporter* reporterror);

    //! \brief Handling function for template definitions and instantiations
    static bool TryToHandleTemplate(const std::string &file, StringIterator &itr,
        ObjectFile &obj, const std::string &preceeding, LErrorReporter* reporterror);

    //! \brief Handling function for loading whole objects
    static std::shared_ptr<ObjectFileObject> TryToLoadObject(const std::string &file,
        StringIterator &itr, ObjectFile &obj, const std::string &preceeding,
        LErrorReporter* reporterror);


    // Object loading split into parts //
    //! \brief Loading variable lists
    static bool TryToLoadVariableList(const std::string &file, StringIterator &itr,
        ObjectFileObject &obj, size_t startline, LErrorReporter* reporterror);


    //! \brief Loading text blocks
    static bool TryToLoadTextBlock(const std::string &file, StringIterator &itr,
        ObjectFileObject &obj, size_t startline, LErrorReporter* reporterror);


    //! \brief Loading script blocks
    static bool TryToLoadScriptBlock(const std::string &file, StringIterator &itr,
        ObjectFileObject &obj, size_t startline, LErrorReporter* reporterror);

    static std::map<std::string, std::shared_ptr<VariableBlock>> RegisteredValues;
};

}

