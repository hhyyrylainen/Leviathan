#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_DATASTORE
#include "DataStore.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "ObjectFileProcessor.h"

DataStore::DataStore(){
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
DataStore::DataStore(bool man){
	if(!man)
		assert(false);

	Staticaccess = this;

	Load();

	TickCount = 0;

	// register data indexes for use in Gui stuff //
	ObjectFileProcessor::RegisterValue(wstring(L"DATAINDEX_TICKTIME"), DATAINDEX_TICKTIME);
	ObjectFileProcessor::RegisterValue(wstring(L"DATAINDEX_TICKCOUNT"), DATAINDEX_TICKCOUNT);
	ObjectFileProcessor::RegisterValue(wstring(L"DATAINDEX_FRAMETIME"), DATAINDEX_FRAMETIME);
	ObjectFileProcessor::RegisterValue(wstring(L"DATAINDEX_FPS"), DATAINDEX_FPS);
	ObjectFileProcessor::RegisterValue(wstring(L"DATAINDEX_WIDTH"), DATAINDEX_WIDTH);
	ObjectFileProcessor::RegisterValue(wstring(L"DATAINDEX_HEIGHT"), DATAINDEX_HEIGHT);
	ObjectFileProcessor::RegisterValue(wstring(L"DATAINDEX_FRAMETIME_MAX"), DATAINDEX_FRAMETIME_MAX);
	ObjectFileProcessor::RegisterValue(wstring(L"DATAINDEX_FRAMETIME_MIN"), DATAINDEX_FRAMETIME_MIN);
	ObjectFileProcessor::RegisterValue(wstring(L"DATAINDEX_FRAMETIME_AVERAGE"), DATAINDEX_FRAMETIME_AVERAGE);
	ObjectFileProcessor::RegisterValue(wstring(L"DATAINDEX_FPS_MIN"), DATAINDEX_FPS_MIN);
	ObjectFileProcessor::RegisterValue(wstring(L"DATAINDEX_FPS_MAX"), DATAINDEX_FPS_MAX);
	ObjectFileProcessor::RegisterValue(wstring(L"DATAINDEX_FPS_AVERAGE"), DATAINDEX_FPS_AVERAGE);

}
DataStore::~DataStore(){
	Save();
}

DataStore* Leviathan::DataStore::Staticaccess = NULL;
DataStore* DataStore::Get(){ return Staticaccess; };
// ------------------------------------ //
int DataStore::GetValue(const wstring &name) const{
	int var = 0;
	values.GetValue(name, var);
	return var;
}
//int DataStore::GetValue(int index){
//	values.GetValue(
//}
bool DataStore::SetValue(const wstring &name, int value){
	values.SetValue(name, value);

	ValueUpdate(name);

	return true;
}
//bool DataStore::SetValue(int index, int value){
//	values.SetValue(
//}
// ------------------------------------ //
int DataStore::GetIndex(const wstring &name) const{
	return values.Find(name);
}
int DataStore::SeekIndex(const int &value, const wstring &partofname){
	vector<shared_ptr<NamedVar>>* vec = values.GetVec();

	int val = 0;
	wstring var;

	for(unsigned int i = 0; i < vec->size(); i++){
		vec->at(i)->GetValue(val, var);

		if(val == value){
			if(Misc::CountOccuranceWstring(vec->at(i)->GetName(), partofname) > 0){
				return i;
			}

		}

	}
	return -1;
}
// ------------------------------------ //
void Leviathan::DataStore::SetPersistance(unsigned int index, bool toset){
	ARR_INDEX_CHECKINV(index, Persistancestates.size())
		return;

	Persistancestates[index] = toset;
}
void DataStore::SetPersistance(const wstring &name, bool toset){
	int index = GetIndex(name);
	ARR_INDEX_CHECKINV(index, (int)Persistancestates.size())
		return;

	Persistancestates[index] = toset;
}
int DataStore::GetPersistance(unsigned int index) const{
	ARR_INDEX_CHECKINV(index, Persistancestates.size())
		return -1;

	return Persistancestates[index];
}
int DataStore::GetPersistance(const wstring &name) const{
	int index = GetIndex(name);
	ARR_INDEX_CHECKINV(index, (int)Persistancestates.size())
		return -1;

	return Persistancestates[index];
}
// ------------------------------------ //
bool DataStore::AddValue(const wstring &name, const int &data){
	values.AddVar(name, data, L"", true);
	Persistancestates.push_back(false);
	return true;
}


bool Leviathan::DataStore::AddValueIfDoesntExist(const wstring &name, const int &data){
	if(values.Find(name) == -1){
		// didn't find, should not exist then //
		AddValue(name, data);
		return true;
	}
	return false;
}


bool DataStore::RemoveVariable(const wstring &name){
	int index = GetIndex(name);
	ARR_INDEX_CHECKINV(index, (int)Persistancestates.size())
		return false;
	values.Remove(index);
	Persistancestates.erase(Persistancestates.begin()+index);
	return true;
}
bool DataStore::RemoveVariable(int index){
	values.Remove(index);
	return true;
}
// ------------------------------------ //
void DataStore::Load(){
	// load //
	vector<shared_ptr<NamedVar>> tempvec;
	FileSystem::LoadDataDumb(L".\\Persist.txt", tempvec);

	values.SetVec(tempvec);
	Persistancestates.resize(tempvec.size());
	for(unsigned int i = 0; i < Persistancestates.size(); i++){
		Persistancestates[i] = true;
	}

}
void DataStore::Save(){
	wstring tosave = L"";
	vector<shared_ptr<NamedVar>>* tempvec = values.GetVec();

	for(unsigned int i = 0; i < Persistancestates.size(); i++){
		if(Persistancestates[i]){
			tosave += tempvec->at(i)->ToText(0);
			if(i+1 < Persistancestates.size()){
				tosave += L"\n";
			}
		}

	}
	FileSystem::WriteToFile(tosave, L".\\Persist.txt");

}
// ------------------------------------ //
void DataStore::RegisterListener(DataListener* listen){
	Listeners.push_back(listen);

//#ifdef _DEBUG
//	Logger::Get()->Info(L"DataStore: DataListener registered! for value "+Convert::IntToWstring(listen->ListenIndex)+L" "+listen->VarName);
//#endif
}

void Leviathan::DataStore::RemoveListener(AutoUpdateableObject* object, int valueid, const wstring &name /* = L""*/, bool all /*= false*/){
	for(unsigned int i = 0; i < Listeners.size(); i++){
		if(Listeners[i]->Object == object){
			// check type, or skip if all //
			if(name != L""){
				if(((all) || (Listeners[i]->ListenIndex == valueid)) && (valueid != -1)){
					_RemoveListener(i);
					return;
				}
			} else {
				if(((all) || (Listeners[i]->VarName == name)) && (name != L"")){
					_RemoveListener(i);
					return;
				}
			}

			continue;
		}
	}
}

int Leviathan::DataStore::GetTickTime() const{
	return TickTime;
}

int Leviathan::DataStore::GetTickCount() const{
	return TickCount;
}

int Leviathan::DataStore::GetFrameTime() const{
	return FrameTime;
}

void Leviathan::DataStore::ValueUpdate(int index){
	shared_ptr<CombinedClass<NamedVar, Int1>> updatedval;

	for(unsigned int i = 0; i < Listeners.size(); i++){
		if(Listeners[i]->ListenOnIndex){
			if(Listeners[i]->ListenIndex == index){
				if(updatedval.get() == NULL){
					// create //
					updatedval = shared_ptr<CombinedClass<NamedVar, Int1>>(new CombinedClass<NamedVar, Int1>());
					updatedval->SetName(Convert::IntToWstring(index));
					updatedval->SetValue(GetValueFromValIndex(index));
					updatedval->SetIntValue(index);
				}
				
				Listeners[i]->Object->OnUpdate(updatedval);
			}
		}
	}
}

void Leviathan::DataStore::ValueUpdate(const wstring& name){
	shared_ptr<CombinedClass<NamedVar, Int1>> updatedval;

	for(unsigned int i = 0; i < Listeners.size(); i++){
		if(!Listeners[i]->ListenOnIndex){
			if(Listeners[i]->VarName == name){
				if(updatedval.get() == NULL){
					// create //
					updatedval = shared_ptr<CombinedClass<NamedVar, Int1>>(new CombinedClass<NamedVar, Int1>());
					updatedval->SetName(name);

					int intval = -1;
					wstring wval = L"";

					this->values.GetValue(name,intval, wval);

					if(wval == L""){
						updatedval->SetValue(intval);
					} else {
						updatedval->SetValue(wval);
					}
					updatedval->SetIntValue(-1);
				}

				Listeners[i]->Object->OnUpdate(updatedval);
			}
		}
	}
}

void Leviathan::DataStore::SetHeight(int newval){
	Height = newval; 
	ValueUpdate(DATAINDEX_HEIGHT);
}

void Leviathan::DataStore::SetWidth(int newval){
	Width = newval; 
	ValueUpdate(DATAINDEX_WIDTH);
}

void Leviathan::DataStore::SetGUiActive(int newval){
	Gui = newval; 
	//ValueUpdate(4);
}

void Leviathan::DataStore::SetFPS(int newval){
	FPS = newval; 
	ValueUpdate(4);
}

void Leviathan::DataStore::SetFrameTime(int newval){
	FrameTime = newval; 
	ValueUpdate(3);
}

void Leviathan::DataStore::SetTickCount(int newval){
	TickCount = newval; 
	ValueUpdate(2);
}

void Leviathan::DataStore::SetTickTime(int newval){
	TickTime = newval; 
	ValueUpdate(1);
}

int Leviathan::DataStore::GetGUiActive() const{
	return Gui;
}

int Leviathan::DataStore::GetWidth() const{
	return Width;
}

int Leviathan::DataStore::GetHeight() const{
	return Height;
}

int Leviathan::DataStore::GetFPS() const{
	return FPS;
}

int Leviathan::DataStore::GetValueFromValIndex(int valindex) const{
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


int Leviathan::DataStore::GetFrameTimeMin() const{
	return FrameTimeMin;
}

int Leviathan::DataStore::GetFrameTimeMax() const{
	return FrameTimeMax;
}

int Leviathan::DataStore::GetFrameTimeAverage() const{
	return FrameTimeAverage;
}

int Leviathan::DataStore::GetFPSMin() const{
	return FPSMax;
}

int Leviathan::DataStore::GetFPSMax() const{
	return FPSMax;
}

int Leviathan::DataStore::GetFPSAverage() const{
	return FPSAverage;
}

void Leviathan::DataStore::SetFrameTimeMin(int newval){
	FrameTimeMin = newval;
	ValueUpdate(DATAINDEX_FRAMETIME_MIN);
}

void Leviathan::DataStore::SetFrameTimeMax(int newval){
	FrameTimeMax = newval;
	ValueUpdate(DATAINDEX_FRAMETIME_MAX);
}

void Leviathan::DataStore::SetFrameTimeAverage(int newval){
	FrameTimeAverage = newval;
	ValueUpdate(DATAINDEX_FRAMETIME_AVERAGE);
}

void Leviathan::DataStore::SetFPSMin(int newval){
	FPSMin = newval;
	ValueUpdate(DATAINDEX_FPS_MIN);
}

void Leviathan::DataStore::SetFPSMax(int newval){
	FPSMax = newval;
	ValueUpdate(DATAINDEX_FPS_MAX);
}

void Leviathan::DataStore::SetFPSAverage(int newval){
	FPSAverage = newval;
	ValueUpdate(DATAINDEX_FPS_AVERAGE);
}



void DataStore::_RemoveListener(int index){
	ARR_INDEX_CHECK(index, (int)Listeners.size()){
		// is valid, ARR_INDEX_CHECKINV would be used if error would be generated here //
		delete Listeners[index];
		Listeners.erase(Listeners.begin()+index);
		return;
	}
	// error //
	Logger::Get()->Error(L"DataStore: Trying to remove listener with invalid index", index);
}

// ----------------------------------------------- //
Leviathan::DataListener::DataListener(){
	ListenIndex = -1;
	ListenOnIndex = false;
	VarName = L"";

	Object = NULL;

	Termination = true;
}

Leviathan::DataListener::DataListener(int index, bool onindex, AutoUpdateableObject* obj, const wstring &var /*= L""*/){
	ListenIndex = index;
	ListenOnIndex = onindex;
	VarName = var;

	Object = obj;

	Termination = false;
}
