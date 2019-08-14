// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/DataStoring/NamedVars.h"
#include "Events/AutoUpdateable.h"

namespace Leviathan {

#define DATAINDEX_TICKTIME 1
#define DATAINDEX_TICKCOUNT 2
#define DATAINDEX_FRAMETIME 3
#define DATAINDEX_FPS 4
#define DATAINDEX_WIDTH 5
#define DATAINDEX_HEIGHT 6

#define DATAINDEX_FRAMETIME_MAX 7
#define DATAINDEX_FRAMETIME_MIN 8
#define DATAINDEX_FRAMETIME_AVERAGE 9

#define DATAINDEX_FPS_MIN 10
#define DATAINDEX_FPS_MAX 11
#define DATAINDEX_FPS_AVERAGE 12

#define DATAINDEX_FONT_SIZEMULTIPLIER 13


#define DATAINDEX_UND -1


struct DataListener {
    DLLEXPORT DataListener();
    DLLEXPORT DataListener(int index, bool onindex, const std::string& var = "");

    int ListenIndex;
    bool ListenOnIndex;
    std::string VarName;

    // AutoUpdateableObject* Object;
};

// holds all data listeners related to a object //
struct DataListenHolder {
    inline ~DataListenHolder()
    {
        // free memory //
        SAFE_DELETE_VECTOR(HandledListeners);
    }

    std::vector<DataListener*> HandledListeners;
};


class DataStore {
public:
    DLLEXPORT DataStore();
    DLLEXPORT DataStore(bool main);
    DLLEXPORT ~DataStore();

    DLLEXPORT void SetPersistance(unsigned int index, bool toset);
    DLLEXPORT void SetPersistance(const std::string& name, bool toset);
    DLLEXPORT int GetPersistance(unsigned int index) const;
    DLLEXPORT int GetPersistance(const std::string& name) const;
    // ------------------------------------ //
    DLLEXPORT bool SetValue(const std::string& name, const VariableBlock& value1);
    DLLEXPORT bool SetValue(const std::string& name, VariableBlock* value1);
    DLLEXPORT bool SetValue(
        const std::string& name, const std::vector<VariableBlock*>& values);

    DLLEXPORT bool SetValue(NamedVariableList& nameandvalues);

    DLLEXPORT size_t GetValueCount(const std::string& name) const;

    DLLEXPORT const VariableBlock* GetValue(const std::string& name) const;
    DLLEXPORT bool GetValue(const std::string& name, VariableBlock& receiver) const;
    DLLEXPORT bool GetValues(
        const std::string& name, std::vector<const VariableBlock*>& receiver) const;


    template<class T>
    bool GetValueAndConvertTo(const std::string& name, T& receiver) const
    {
        // use try block to catch all exceptions (not found and conversion fail //
        try {
            if(!Values.GetValue(name)->ConvertAndAssingToVariable<T>(receiver)) {
                // Couldn't convert or assign or find the value //
                return false;
            }
        } catch(...) {
            // variable not found / wrong type //
            return false;
        }
        // correct variable has been set //
        return true;
    }

    // DLLEXPORT vector<VariableBlock*>* GetValues(const std::string &name) throw(...);

    DLLEXPORT void AddVar(std::shared_ptr<NamedVariableList> values);
    DLLEXPORT void Remove(size_t index);
    DLLEXPORT void Remove(const std::string& name);

    DLLEXPORT int GetVariableType(const std::string& name) const;
    DLLEXPORT int GetVariableTypeOfAll(const std::string& name) const;
    // ------------------------------------ //

    DLLEXPORT static std::string GetPersistStorageFile();

    DLLEXPORT static DataStore* Get();

    // variables //
public:
    DLLEXPORT int GetTickTime() const;
    DLLEXPORT int GetTickCount() const;
    DLLEXPORT int GetTicksBehind() const;
    DLLEXPORT int GetFrameTime() const;
    DLLEXPORT int GetFPS() const;
    DLLEXPORT int GetHeight() const;
    DLLEXPORT int GetWidth() const;
    DLLEXPORT int GetGUiActive() const;
    DLLEXPORT int GetFrameTimeMin() const;
    DLLEXPORT int GetFrameTimeMax() const;
    DLLEXPORT int GetFrameTimeAverage() const;
    DLLEXPORT int GetFPSMin() const;
    DLLEXPORT int GetFPSMax() const;
    DLLEXPORT int GetFPSAverage() const;
    DLLEXPORT int GetFontSizeMultiplier() const;

    DLLEXPORT void SetTickTime(int newval);
    DLLEXPORT void SetTickCount(int newval);
    DLLEXPORT void SetTicksBehind(int value);
    DLLEXPORT void SetFrameTime(int newval);
    DLLEXPORT void SetFPS(int newval);
    DLLEXPORT void SetGUiActive(int newval);
    DLLEXPORT void SetFrameTimeMin(int newval);
    DLLEXPORT void SetFrameTimeMax(int newval);
    DLLEXPORT void SetFrameTimeAverage(int newval);
    DLLEXPORT void SetFPSMin(int newval);
    DLLEXPORT void SetFPSMax(int newval);
    DLLEXPORT void SetFPSAverage(int newval);
    DLLEXPORT void SetFontSizeMultiplier(int newval);

    DLLEXPORT void SetWidth(int newval);
    DLLEXPORT void SetHeight(int newval);

    DLLEXPORT int GetValueFromValIndex(int valindex) const;

public:
    DLLEXPORT void RegisterListener(AutoUpdateableObject* object, DataListener* listen);
    DLLEXPORT void RemoveListener(AutoUpdateableObject* object, int valueid,
        const std::string& name = "", bool all = false);

private:
    // ------------------------------------ //
    void Load();
    void Save();

    // void _RemoveListener(int index);
    void ValueUpdate(int index);
    void ValueUpdate(const std::string& name);
    // ------------------------------------ //
    NamedVars Values;
    // NamedVariableLists that should be saved to file on quit //
    std::vector<bool> Persistencestates;


    std::map<AutoUpdateableObject*, std::shared_ptr<DataListenHolder>> Listeners;


    // vector<DataListener*> Listeners;



    // vars //
    int TickTime;
    int TickCount;
    int TicksBehind = 0;
    int FrameTime;
    int FrameTimeMin;
    int FrameTimeMax;
    int FrameTimeAverage;

    int FPS;
    int FPSMin;
    int FPSMax;
    int FPSAverage;
    int Gui;

    int Width;
    int Height;

    int FontSizeMultiplier;

    // static //
    static DataStore* Staticaccess;
};

} // namespace Leviathan
