// ------------------------------------ //
#include "DataStore.h"

#include "Application/AppDefine.h"
#include "Common/StringOperations.h"
#include "FileSystem.h"
#include "ObjectFiles/ObjectFileProcessor.h"

using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::DataStore::DataStore()
{
    Load();

    // TODO: move these to the class definition
    // set default values //
    TickTime = 0;
    TickCount = 0;
    FrameTime = 0;
    FPS = 0;

    FrameTimeMin = 0;
    FrameTimeMax = 0;
    FrameTimeAverage = 0;


    FPSMin = 0;
    FPSMax = 0;
    FPSAverage = 0;

    // set font multiplier default //
    FontSizeMultiplier = 1;
}
DLLEXPORT Leviathan::DataStore::DataStore(bool man)
{
    LEVIATHAN_ASSERT(man, "this shouldn't be called with false");

    Staticaccess = this;

    Load();

    TickCount = 0;

    // set default values //
    TickTime = 0;
    TickCount = 0;
    FrameTime = 0;
    FPS = 0;

    FrameTimeMin = 0;
    FrameTimeMax = 0;
    FrameTimeAverage = 0;


    FPSMin = 0;
    FPSMax = 0;
    FPSAverage = 0;

    // set font multiplier default //
    FontSizeMultiplier = 1;

    // register data indexes for use in Gui stuff //
    // moved directly to object file processor
}
DLLEXPORT Leviathan::DataStore::~DataStore()
{
    Save();
}

DataStore* Leviathan::DataStore::Staticaccess = NULL;

DataStore* Leviathan::DataStore::Get()
{
    return Staticaccess;
}
// ------------------------------------ //
void Leviathan::DataStore::Load()
{
    // load //
    std::vector<std::shared_ptr<NamedVariableList>> tempvec;
    FileSystem::LoadDataDump(GetPersistStorageFile(), tempvec, Logger::Get());

    Values.SetVec(tempvec);

    // All loaded from file will also be saved again //
    Persistencestates.resize(tempvec.size(), true);
}

void Leviathan::DataStore::Save()
{
    std::string tosave = "";
    std::vector<std::shared_ptr<NamedVariableList>>* tempvec = Values.GetVec();

    for(unsigned int i = 0; i < Persistencestates.size(); i++) {
        if(Persistencestates[i]) {

            try {

                tosave += tempvec->at(i)->ToText(0);

            } catch(const InvalidType& e) {

                Logger::Get()->Error("DataStore: failed to serialize value \"" +
                                     tempvec->at(i)->GetName() + "\": ");
                e.PrintToLog();
                continue;
            }

            if(i + 1 < Persistencestates.size()) {
                tosave += "\n";
            }
        }
    }

    FileSystem::WriteToFile(tosave, GetPersistStorageFile());
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::DataStore::GetValue(
    const std::string& name, VariableBlock& receiver) const
{
    return Values.GetValue(name, receiver);
}

DLLEXPORT size_t Leviathan::DataStore::GetValueCount(const std::string& name) const
{
    return Values.GetValueCount(name);
}

DLLEXPORT bool Leviathan::DataStore::GetValues(
    const std::string& name, std::vector<const VariableBlock*>& receiver) const
{
    return Values.GetValues(name, receiver);
}
// ------------------------------------ //
void Leviathan::DataStore::SetPersistance(unsigned int index, bool toset)
{

    Persistencestates[index] = toset;
}
void DataStore::SetPersistance(const std::string& name, bool toset)
{
    auto index = Values.Find(name);

    if(!Values.IsIndexValid(index))
        throw Exception("SetPersitance called for non-existent value");

    Persistencestates[index] = toset;
}
int DataStore::GetPersistance(unsigned int index) const
{

    return Persistencestates[index];
}
int DataStore::GetPersistance(const std::string& name) const
{
    auto index = Values.Find(name);

    if(!Values.IsIndexValid(index))
        throw Exception("GetPersistance called for non-existent value");

    return Persistencestates[index];
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::DataStore::SetValue(
    const std::string& name, const VariableBlock& value1)
{
    // use variable holder to do this //
    Values.SetValue(name, value1);

    // send update to value listeners //
    ValueUpdate(name);

    return true;
}

DLLEXPORT bool Leviathan::DataStore::SetValue(const std::string& name, VariableBlock* value1)
{
    // use variable holder to do this //
    Values.SetValue(name, value1);

    // send update to value listeners //
    ValueUpdate(name);

    return true;
}

DLLEXPORT bool Leviathan::DataStore::SetValue(
    const std::string& name, const std::vector<VariableBlock*>& values)
{
    // use variable holder to do this //
    this->Values.SetValue(name, values);

    // send update to value listeners //
    ValueUpdate(name);

    return true;
}

DLLEXPORT bool Leviathan::DataStore::SetValue(NamedVariableList& nameandvalues)
{
    // send update to value listeners //
    ValueUpdate(nameandvalues.GetName());

    return true;
}

// ----------------------------------------------- //
DLLEXPORT void Leviathan::DataStore::AddVar(std::shared_ptr<NamedVariableList> values)
{
    if(Values.Find(values->GetName()) > Values.GetVariableCount()) {
        // can add new //
        this->Values.AddVar(values);

        // don't forget to add persistence //
        Persistencestates.push_back(false);
    }
}

DLLEXPORT void Leviathan::DataStore::Remove(size_t index)
{
    if(index >= Values.GetVariableCount())
        return;

    // remove from store //
    Values.Remove(index);
    // don't forget to remove from persistence states //
    Persistencestates.erase(Persistencestates.begin() + index);
}

DLLEXPORT void Leviathan::DataStore::Remove(const std::string& name)
{
    // call overload //
    Remove(Values.Find(name));
}
// ----------------------------------------------- //
DLLEXPORT int Leviathan::DataStore::GetVariableType(const std::string& name) const
{
    return Values.GetVariableType(name);
}

DLLEXPORT int Leviathan::DataStore::GetVariableTypeOfAll(const std::string& name) const
{
    return Values.GetVariableTypeOfAll(name);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::DataStore::RegisterListener(
    AutoUpdateableObject* object, DataListener* listen)
{
    // set into the map //

    DataListenHolder* tmpptre = Listeners[object].get();

    if(tmpptre == NULL) {
        // new required //
        // add back to map //
        Listeners[object] = std::make_shared<DataListenHolder>();

        // recurse to use the new object //
        return RegisterListener(object, listen);
    }

    // can add new one //
    tmpptre->HandledListeners.push_back(listen);
}

DLLEXPORT void Leviathan::DataStore::RemoveListener(
    AutoUpdateableObject* object, int valueid, const std::string& name, bool all)
{
    if(all) {
        // just erase the bulk //
        Listeners.erase(object);
        return;
    }

    // get pointer to block //
    DataListenHolder* tmpptre = Listeners[object].get();

    if(tmpptre == NULL) {
        return;
    }

    // erase the wanted ones //
    for(size_t i = 0; i < tmpptre->HandledListeners.size(); i++) {
        if(tmpptre->HandledListeners[i]->ListenIndex == valueid) {
            // check name if wanted //
            if(name.size() == 0 || name == tmpptre->HandledListeners[i]->VarName) {
                // erase //

                SAFE_DELETE(tmpptre->HandledListeners[i]);
                tmpptre->HandledListeners.erase(tmpptre->HandledListeners.begin() + i);

                break;
            }
        }
    }
}
// ------------------------------------ //
DLLEXPORT int Leviathan::DataStore::GetTickTime() const
{
    return TickTime;
}

DLLEXPORT int Leviathan::DataStore::GetTickCount() const
{
    return TickCount;
}

DLLEXPORT int DataStore::GetTicksBehind() const
{
    return TicksBehind;
}

DLLEXPORT int Leviathan::DataStore::GetFrameTime() const
{
    return FrameTime;
}

void Leviathan::DataStore::ValueUpdate(int index)
{
    std::shared_ptr<NamedVariableList> updatedval(nullptr);

    for(auto iter = Listeners.begin(); iter != Listeners.end(); ++iter) {
        // iterate held indexes //

        for(size_t i = 0; i < iter->second->HandledListeners.size(); i++) {
            // check for match //

            if(iter->second->HandledListeners[i]->ListenIndex == index) {
                // check is value fine or not //
                if(updatedval.get() == NULL) {
                    // create //
                    updatedval = std::make_shared<NamedVariableList>(
                        Convert::ToString(index), new IntBlock(GetValueFromValIndex(index)));
                }

                // send update //
                iter->first->OnUpdate(updatedval);
            }
        }
    }
}

void Leviathan::DataStore::ValueUpdate(const std::string& name)
{
    std::shared_ptr<NamedVariableList> updatedval(nullptr);

    for(auto iter = Listeners.begin(); iter != Listeners.end(); ++iter) {
        // iterate held indexes //
        for(size_t i = 0; i < iter->second->HandledListeners.size(); i++) {
            // check for match //
            if(iter->second->HandledListeners[i]->VarName == name) {
                // check is value fine or not //
                if(updatedval.get() == NULL) {
                    // create //
                    updatedval = std::shared_ptr<NamedVariableList>(new NamedVariableList(name,
                        new VariableBlock(
                            Values.GetValue(name)->GetBlockConst()->AllocateNewFromThis())));
                }

                // send update //
                iter->first->OnUpdate(updatedval);
            }
        }
    }
}



DLLEXPORT void Leviathan::DataStore::SetHeight(int newval)
{
    Height = newval;
    ValueUpdate(DATAINDEX_HEIGHT);
}

DLLEXPORT void Leviathan::DataStore::SetWidth(int newval)
{
    Width = newval;
    ValueUpdate(DATAINDEX_WIDTH);
}

DLLEXPORT void Leviathan::DataStore::SetGUiActive(int newval)
{
    Gui = newval;
    // ValueUpdate(4);
}

DLLEXPORT void Leviathan::DataStore::SetFPS(int newval)
{
    FPS = newval;
    ValueUpdate(4);
}

DLLEXPORT void Leviathan::DataStore::SetFrameTime(int newval)
{
    FrameTime = newval;
    ValueUpdate(3);
}

DLLEXPORT void Leviathan::DataStore::SetTickCount(int newval)
{
    TickCount = newval;
    ValueUpdate(2);
}

DLLEXPORT void Leviathan::DataStore::SetTickTime(int newval)
{
    TickTime = newval;
    ValueUpdate(1);
}

DLLEXPORT void DataStore::SetTicksBehind(int value)
{
    TicksBehind = value;
}

DLLEXPORT int Leviathan::DataStore::GetGUiActive() const
{
    return Gui;
}

DLLEXPORT int Leviathan::DataStore::GetWidth() const
{
    return Width;
}

DLLEXPORT int Leviathan::DataStore::GetHeight() const
{
    return Height;
}

DLLEXPORT int Leviathan::DataStore::GetFPS() const
{
    return FPS;
}

DLLEXPORT int Leviathan::DataStore::GetValueFromValIndex(int valindex) const
{
    switch(valindex) {
    case DATAINDEX_TICKTIME: {
        return TickTime;
    } break;
    case DATAINDEX_TICKCOUNT: {
        return TickCount;
    } break;
    case DATAINDEX_FRAMETIME: {
        return FrameTime;
    } break;
    case DATAINDEX_FPS: {
        return FPS;
    } break;
    case DATAINDEX_HEIGHT: {
        return Height;
    } break;
    case DATAINDEX_FRAMETIME_MAX: {
        return FrameTimeMax;
    } break;
    case DATAINDEX_FRAMETIME_MIN: {
        return FrameTimeMin;
    } break;
    case DATAINDEX_FRAMETIME_AVERAGE: {
        return FrameTimeAverage;
    } break;
    case DATAINDEX_FPS_MIN: {
        return FPSMin;
    } break;
    case DATAINDEX_FPS_MAX: {
        return FPSMax;
    } break;
    case DATAINDEX_FPS_AVERAGE: {
        return FPSAverage;
    } break;
    }
    return -1;
}


DLLEXPORT int Leviathan::DataStore::GetFrameTimeMin() const
{
    return FrameTimeMin;
}

DLLEXPORT int Leviathan::DataStore::GetFrameTimeMax() const
{
    return FrameTimeMax;
}

DLLEXPORT int Leviathan::DataStore::GetFrameTimeAverage() const
{
    return FrameTimeAverage;
}

DLLEXPORT int Leviathan::DataStore::GetFPSMin() const
{
    return FPSMax;
}

DLLEXPORT int Leviathan::DataStore::GetFPSMax() const
{
    return FPSMax;
}

DLLEXPORT int Leviathan::DataStore::GetFPSAverage() const
{
    return FPSAverage;
}

DLLEXPORT void Leviathan::DataStore::SetFrameTimeMin(int newval)
{
    FrameTimeMin = newval;
    ValueUpdate(DATAINDEX_FRAMETIME_MIN);
}

DLLEXPORT void Leviathan::DataStore::SetFrameTimeMax(int newval)
{
    FrameTimeMax = newval;
    ValueUpdate(DATAINDEX_FRAMETIME_MAX);
}

DLLEXPORT void Leviathan::DataStore::SetFrameTimeAverage(int newval)
{
    FrameTimeAverage = newval;
    ValueUpdate(DATAINDEX_FRAMETIME_AVERAGE);
}

DLLEXPORT void Leviathan::DataStore::SetFPSMin(int newval)
{
    FPSMin = newval;
    ValueUpdate(DATAINDEX_FPS_MIN);
}

DLLEXPORT void Leviathan::DataStore::SetFPSMax(int newval)
{
    FPSMax = newval;
    ValueUpdate(DATAINDEX_FPS_MAX);
}

DLLEXPORT void Leviathan::DataStore::SetFPSAverage(int newval)
{
    FPSAverage = newval;
    ValueUpdate(DATAINDEX_FPS_AVERAGE);
}

DLLEXPORT int Leviathan::DataStore::GetFontSizeMultiplier() const
{
    return FontSizeMultiplier;
}

DLLEXPORT void Leviathan::DataStore::SetFontSizeMultiplier(int newval)
{
    FontSizeMultiplier = newval;
}
// ------------------------------------ //
DLLEXPORT std::string DataStore::GetPersistStorageFile()
{

    std::string logName = StringOperations::RemoveEnding<std::string>(
        StringOperations::RemoveExtension(Logger::Get()->GetLogFile(), false), "Log");

    return logName + "Persist.txt";
}
// ----------------------------------------------- //
// DataListener
DLLEXPORT Leviathan::DataListener::DataListener()
{
    ListenIndex = -1;
    ListenOnIndex = false;
    VarName = "";
}

DLLEXPORT Leviathan::DataListener::DataListener(
    int index, bool onindex, const std::string& var)
{

    ListenIndex = index;
    ListenOnIndex = onindex;
    VarName = var;
}
// ------------------------------------ //
