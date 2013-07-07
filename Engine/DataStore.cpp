#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_DATASTORE
#include "DataStore.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "ObjectFileProcessor.h"

DLLEXPORT Leviathan::DataStore::DataStore(){
	Load();


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
}
DLLEXPORT Leviathan::DataStore::DataStore(bool man){
	if(!man)
		assert(0 && "this shouldn't be called with false");

	Staticaccess = this;

	Load();

	TickCount = 0;

	// register data indexes for use in Gui stuff //
	ObjectFileProcessor::RegisterValue(new NamedVariableBlock(new IntBlock(DATAINDEX_TICKTIME), L"DATAINDEX_TICKTIME"));
	ObjectFileProcessor::RegisterValue(new NamedVariableBlock(new IntBlock(DATAINDEX_TICKCOUNT), L"DATAINDEX_TICKCOUNT"));
	ObjectFileProcessor::RegisterValue(new NamedVariableBlock(new IntBlock(DATAINDEX_FRAMETIME), L"DATAINDEX_FRAMETIME"));
	ObjectFileProcessor::RegisterValue(new NamedVariableBlock(new IntBlock(DATAINDEX_FPS), L"DATAINDEX_FPS"));
	ObjectFileProcessor::RegisterValue(new NamedVariableBlock(new IntBlock(DATAINDEX_WIDTH), L"DATAINDEX_WIDTH"));
	ObjectFileProcessor::RegisterValue(new NamedVariableBlock(new IntBlock(DATAINDEX_HEIGHT), L"DATAINDEX_HEIGHT"));
	ObjectFileProcessor::RegisterValue(new NamedVariableBlock(new IntBlock(DATAINDEX_FRAMETIME_MAX), L"DATAINDEX_FRAMETIME_MAX"));
	ObjectFileProcessor::RegisterValue(new NamedVariableBlock(new IntBlock(DATAINDEX_FRAMETIME_MIN), L"DATAINDEX_FRAMETIME_MIN"));
	ObjectFileProcessor::RegisterValue(new NamedVariableBlock(new IntBlock(DATAINDEX_FRAMETIME_AVERAGE), L"DATAINDEX_FRAMETIME_AVERAGE"));
	ObjectFileProcessor::RegisterValue(new NamedVariableBlock(new IntBlock(DATAINDEX_FPS_MIN), L"DATAINDEX_FPS_MIN"));
	ObjectFileProcessor::RegisterValue(new NamedVariableBlock(new IntBlock(DATAINDEX_FPS_MAX), L"DATAINDEX_FPS_MAX"));
	ObjectFileProcessor::RegisterValue(new NamedVariableBlock(new IntBlock(DATAINDEX_FPS_AVERAGE), L"DATAINDEX_FPS_AVERAGE"));

}
DLLEXPORT Leviathan::DataStore::~DataStore(){
	Save();
}
void Leviathan::DataStore::Load(){
	// load //
	vector<shared_ptr<NamedVariableList>> tempvec;
	FileSystem::LoadDataDump(L".\\Persist.txt", tempvec);

	Values.SetVec(tempvec);
	Persistencestates.resize(tempvec.size());
	for(unsigned int i = 0; i < Persistencestates.size(); i++){
		Persistencestates[i] = true;
	}

}
void Leviathan::DataStore::Save(){
	wstring tosave = L"";
	vector<shared_ptr<NamedVariableList>>* tempvec = Values.GetVec();

	for(unsigned int i = 0; i < Persistencestates.size(); i++){
		if(Persistencestates[i]){
			tosave += tempvec->at(i)->ToText(0);
			if(i+1 < Persistencestates.size()){
				tosave += L"\n";
			}
		}

	}
	FileSystem::WriteToFile(tosave, L".\\Persist.txt");

}

DataStore* Leviathan::DataStore::Staticaccess = NULL;
DataStore* Leviathan::DataStore::Get(){ return Staticaccess; };
// ------------------------------------ //
DLLEXPORT bool Leviathan::DataStore::GetValue(const wstring &name, VariableBlock &receiver) const{
	return Values.GetValue(name, receiver);
}

DLLEXPORT bool Leviathan::DataStore::GetValue(const wstring &name, const int &nindex, VariableBlock &receiver) const{
	return Values.GetValue(name, nindex, receiver);
}

DLLEXPORT size_t Leviathan::DataStore::GetValueCount(const wstring &name) const{
	return Values.GetValueCount(name);
}

DLLEXPORT bool Leviathan::DataStore::GetValues(const wstring &name, vector<const VariableBlock*> &receiver) const{
	return Values.GetValues(name, receiver);
}
// ------------------------------------ //
void Leviathan::DataStore::SetPersistance(unsigned int index, bool toset){
	ARR_INDEX_CHECKINV(index, Persistencestates.size())
		return;

	Persistencestates[index] = toset;
}
void DataStore::SetPersistance(const wstring &name, bool toset){
	int index = Values.Find(name);
	ARR_INDEX_CHECKINV(index, (int)Persistencestates.size())
		return;

	Persistencestates[index] = toset;
}
int DataStore::GetPersistance(unsigned int index) const{
	ARR_INDEX_CHECKINV(index, Persistencestates.size())
		return -1;

	return Persistencestates[index];
}
int DataStore::GetPersistance(const wstring &name) const{
	int index = Values.Find(name);
	ARR_INDEX_CHECKINV(index, (int)Persistencestates.size())
		return -1;

	return Persistencestates[index];
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::DataStore::SetValue(const wstring &name, const VariableBlock &value1){
	// use variable holder to do this //
	Values.SetValue(name, value1);

	// send update to value listeners //
	ValueUpdate(name);

	return true;
}

DLLEXPORT bool Leviathan::DataStore::SetValue(const wstring &name, VariableBlock* value1){
	// use variable holder to do this //
	Values.SetValue(name, value1);

	// send update to value listeners //
	ValueUpdate(name);

	return true;
}

DLLEXPORT bool Leviathan::DataStore::SetValue(const wstring &name, const vector<VariableBlock*> &values){
	// use variable holder to do this //
	this->Values.SetValue(name, values);

	// send update to value listeners //
	ValueUpdate(name);

	return true;
}

DLLEXPORT bool Leviathan::DataStore::SetValue(NamedVariableList &nameandvalues){

	// send update to value listeners //
	ValueUpdate(nameandvalues.GetName());

	return true;
}

// ----------------------------------------------- //
DLLEXPORT void Leviathan::DataStore::AddVar(NamedVariableList* newvaluetoadd){
	if(Values.Find(newvaluetoadd->GetName()) < 0){
		// can add new //
		Values.AddVar(newvaluetoadd);

		// don't forget to add persistence //
		Persistencestates.push_back(false);


	} else {
		// can't add, must delete //
		SAFE_DELETE(newvaluetoadd);
	}
}

DLLEXPORT void Leviathan::DataStore::AddVar(shared_ptr<NamedVariableList> values){
	if(this->Values.Find(values->GetName()) < 0){
		// can add new //
		this->Values.AddVar(values);

		// don't forget to add persistence //
		Persistencestates.push_back(false);
	}
}

DLLEXPORT void Leviathan::DataStore::Remove(unsigned int index){
	// index checking //
	ARR_INDEX_CHECKINV(index, Values.GetVec()->size()){
		// invalid index //
		DEBUG_BREAK;
		return;
	}

	// remove from store //
	Values.Remove(index);
	// don't forget to remove from persistence states //
	Persistencestates.erase(Persistencestates.begin()+index);
}

DLLEXPORT void Leviathan::DataStore::Remove(const wstring &name){
	// call overload //
	Remove(Values.Find(name));
}
// ----------------------------------------------- //
DLLEXPORT int Leviathan::DataStore::GetVariableType(const wstring &name) const{
	return Values.GetVariableType(name);
}

DLLEXPORT int Leviathan::DataStore::GetVariableTypeOfAll(const wstring &name) const{
	return Values.GetVariableTypeOfAll(name);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::DataStore::RegisterListener(AutoUpdateableObject* object, DataListener* listen){
	// set into the map //

	DataListenHolder* tmpptre = Listeners[object].get();

	if(tmpptre == NULL){
		// new required //
		tmpptre = new DataListenHolder();
		// add back to map //
		Listeners[object] = shared_ptr<DataListenHolder>(tmpptre);

	}

	// can add new one //
	tmpptre->HandledListeners.push_back(listen);
}

DLLEXPORT void Leviathan::DataStore::RemoveListener(AutoUpdateableObject* object, int valueid, const wstring &name /* = L""*/, bool all /*= false*/){
	if(all){
		// just erase the bulk //
		Listeners.erase(object);
		return;
	}

	// get pointer to block //
	DataListenHolder* tmpptre = Listeners[object].get();

	if(tmpptre == NULL){
		return;
	}

	// erase the wanted ones //
	for(size_t i = 0; i < tmpptre->HandledListeners.size(); i++){
		if(tmpptre->HandledListeners[i]->ListenIndex == valueid){
			// check name if wanted //
			if(name.size() == 0 || name == tmpptre->HandledListeners[i]->VarName){
				// erase //

				SAFE_DELETE(tmpptre->HandledListeners[i]);
				tmpptre->HandledListeners.erase(tmpptre->HandledListeners.begin()+i);

				break;
			}
		}
	}
}
// ------------------------------------ //
DLLEXPORT int Leviathan::DataStore::GetTickTime() const{
	return TickTime;
}

DLLEXPORT int Leviathan::DataStore::GetTickCount() const{
	return TickCount;
}

DLLEXPORT int Leviathan::DataStore::GetFrameTime() const{
	return FrameTime;
}

void Leviathan::DataStore::ValueUpdate(int index){
	shared_ptr<NamedVariableList> updatedval(nullptr);


	for(auto iter = Listeners.begin(); iter != Listeners.end(); ++iter) {
		// iterate held indexes //
		for(size_t i = 0; i < iter->second->HandledListeners.size(); i++){
			// check for match //
			if(iter->second->HandledListeners[i]->ListenIndex == index){
				// check is value fine or not //
				if(updatedval.get() == NULL){
					// create //
					updatedval = shared_ptr<NamedVariableList>(new NamedVariableList(Convert::IntToWstring(index), new 
						IntBlock(GetValueFromValIndex(index))));
				}
				// send update //
				iter->first->OnUpdate(updatedval);
			}
		}
	}
}

void Leviathan::DataStore::ValueUpdate(const wstring& name){
	shared_ptr<NamedVariableList> updatedval(nullptr);

	for(auto iter = Listeners.begin(); iter != Listeners.end(); ++iter) {
		// iterate held indexes //
		for(size_t i = 0; i < iter->second->HandledListeners.size(); i++){
			// check for match //
			if(iter->second->HandledListeners[i]->VarName == name){
				// check is value fine or not //
				if(updatedval.get() == NULL){
					// create //
					updatedval = shared_ptr<NamedVariableList>(new NamedVariableList(name, new VariableBlock(
						Values.GetValue(name)->GetBlockConst()->AllocateNewFromThis())));
				}
				// send update //
				iter->first->OnUpdate(updatedval);
			}
		}
	}
}



DLLEXPORT void Leviathan::DataStore::SetHeight(int newval){
	Height = newval; 
	ValueUpdate(DATAINDEX_HEIGHT);
}

DLLEXPORT void Leviathan::DataStore::SetWidth(int newval){
	Width = newval; 
	ValueUpdate(DATAINDEX_WIDTH);
}

DLLEXPORT void Leviathan::DataStore::SetGUiActive(int newval){
	Gui = newval; 
	//ValueUpdate(4);
}

DLLEXPORT void Leviathan::DataStore::SetFPS(int newval){
	FPS = newval; 
	ValueUpdate(4);
}

DLLEXPORT void Leviathan::DataStore::SetFrameTime(int newval){
	FrameTime = newval; 
	ValueUpdate(3);
}

DLLEXPORT void Leviathan::DataStore::SetTickCount(int newval){
	TickCount = newval; 
	ValueUpdate(2);
}

DLLEXPORT void Leviathan::DataStore::SetTickTime(int newval){
	TickTime = newval; 
	ValueUpdate(1);
}

DLLEXPORT int Leviathan::DataStore::GetGUiActive() const{
	return Gui;
}

DLLEXPORT int Leviathan::DataStore::GetWidth() const{
	return Width;
}

DLLEXPORT int Leviathan::DataStore::GetHeight() const{
	return Height;
}

DLLEXPORT int Leviathan::DataStore::GetFPS() const{
	return FPS;
}

DLLEXPORT int Leviathan::DataStore::GetValueFromValIndex(int valindex) const{
	switch(valindex){
		case DATAINDEX_TICKTIME:
			{
				return TickTime;
			}
		break;
		case DATAINDEX_TICKCOUNT:
			{
				return TickCount;
			}
		break;
		case DATAINDEX_FRAMETIME:
			{
				return FrameTime;
			}
		break;
		case DATAINDEX_FPS:
			{
				return FPS;
			}
		break;
			//case 5: // doesn't exist
			//	{
			//		Listeners[i]->Object->OnUpdate(GuiActive, true, L"");
			//	}
			//break;
		case DATAINDEX_HEIGHT:
			{
				return Height;
			}
		break;
		case DATAINDEX_FRAMETIME_MAX:
			{
				return FrameTimeMax;
			}
		break;
		case DATAINDEX_FRAMETIME_MIN:
			{
				return FrameTimeMin;
			}
		break;
		case DATAINDEX_FRAMETIME_AVERAGE:
			{
				return FrameTimeAverage;
			}
		break;
		case DATAINDEX_FPS_MIN:
			{
				return FPSMin;
			}
		break;
		case DATAINDEX_FPS_MAX:
			{
				return FPSMax;
			}
		break;
		case DATAINDEX_FPS_AVERAGE:
			{
				return FPSAverage;
			}
		break;
	}
	return -1;
}


DLLEXPORT int Leviathan::DataStore::GetFrameTimeMin() const{
	return FrameTimeMin;
}

DLLEXPORT int Leviathan::DataStore::GetFrameTimeMax() const{
	return FrameTimeMax;
}

DLLEXPORT int Leviathan::DataStore::GetFrameTimeAverage() const{
	return FrameTimeAverage;
}

DLLEXPORT int Leviathan::DataStore::GetFPSMin() const{
	return FPSMax;
}

DLLEXPORT int Leviathan::DataStore::GetFPSMax() const{
	return FPSMax;
}

DLLEXPORT int Leviathan::DataStore::GetFPSAverage() const{
	return FPSAverage;
}

DLLEXPORT void Leviathan::DataStore::SetFrameTimeMin(int newval){
	FrameTimeMin = newval;
	ValueUpdate(DATAINDEX_FRAMETIME_MIN);
}

DLLEXPORT void Leviathan::DataStore::SetFrameTimeMax(int newval){
	FrameTimeMax = newval;
	ValueUpdate(DATAINDEX_FRAMETIME_MAX);
}

DLLEXPORT void Leviathan::DataStore::SetFrameTimeAverage(int newval){
	FrameTimeAverage = newval;
	ValueUpdate(DATAINDEX_FRAMETIME_AVERAGE);
}

DLLEXPORT void Leviathan::DataStore::SetFPSMin(int newval){
	FPSMin = newval;
	ValueUpdate(DATAINDEX_FPS_MIN);
}

DLLEXPORT void Leviathan::DataStore::SetFPSMax(int newval){
	FPSMax = newval;
	ValueUpdate(DATAINDEX_FPS_MAX);
}

DLLEXPORT void Leviathan::DataStore::SetFPSAverage(int newval){
	FPSAverage = newval;
	ValueUpdate(DATAINDEX_FPS_AVERAGE);
}

// ----------------------------------------------- //
DLLEXPORT Leviathan::DataListener::DataListener(){
	ListenIndex = -1;
	ListenOnIndex = false;
	VarName = L"";
}

DLLEXPORT Leviathan::DataListener::DataListener(int index, bool onindex, const wstring &var /*= L""*/){
	ListenIndex = index;
	ListenOnIndex = onindex;
	VarName = var;
}
