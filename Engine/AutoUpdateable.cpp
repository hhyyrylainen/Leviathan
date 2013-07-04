#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_AUTOUPDATEABLE
#include "AutoUpdateable.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "DataStore.h"
Leviathan::AutoUpdateableObject::AutoUpdateableObject(){
	ValuesUpdated = false;
}
Leviathan::AutoUpdateableObject::~AutoUpdateableObject(){
	if(MonitoredIndexes.size() != 0){
		StopMonitoring(-1, L"", true);
	}
}
// ------------------------------------ //
void Leviathan::AutoUpdateableObject::StartMonitoring(int valueid, bool nonid, wstring varname /*= L""*/){
	ValuesUpdated = false;

	if(!nonid){
		DataStore::Get()->RegisterListener(new DataListener(valueid, !nonid, this));
		// store it as being updated //
		MonitoredIndexes.push_back(valueid);
	} else {
		DataStore::Get()->RegisterListener(new DataListener(-1, !nonid, this, varname));
		// store it as being updated //
		MonitoredValueNames.push_back(shared_ptr<wstring>(new wstring(varname)));
	}

}
void Leviathan::AutoUpdateableObject::StopMonitoring(int index, wstring varname /*= L""*/, bool all /*= false*/){
	// check is index based or name based being erased //
	if(varname == L""){

		// remove value //
		if(all){
			// clear all //
			MonitoredIndexes.clear();
		} else {
			// remove specific one //
			for(size_t i = 0; i < MonitoredIndexes.size(); i++){
				if(MonitoredIndexes[i] == index){

					MonitoredIndexes.erase(MonitoredIndexes.begin()+i);
					break;
				}
			}
		}
		DataStore::Get()->RemoveListener(this, index, L"", all);

	} else {
		// remove value //
		if(all){
			// clear all //
			MonitoredValueNames.clear();
		} else {
			// remove specific one //
			for(unsigned int i = 0; i < MonitoredValueNames.size(); i++){
				if(*MonitoredValueNames[i] == varname){

					MonitoredValueNames.erase(MonitoredValueNames.begin()+i);
					break;
				}
			}
		}
		DataStore::Get()->RemoveListener(this, -1, varname, all);
	}
}

DLLEXPORT bool Leviathan::AutoUpdateableObject::OnUpdate(const shared_ptr<NamedVariableList> &updated){
	ValuesUpdated = true;
	
	// push to update vector //
	UpdatedValues.push_back(updated);

	return true;
}


// ------------------------------------ //
void Leviathan::AutoUpdateableObject::_PopUdated(){
	// just clear the vector, should automatically delete //
	UpdatedValues.clear();

	ValuesUpdated = false;
}
// ------------------------------------ //

// ------------------------------------ //


