#ifndef LEVIATHAN_DATASTORE
#define LEVIATHAN_DATASTORE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Common/DataStoring/NamedVars.h"
#include "FileSystem.h"
#include "Events/AutoUpdateable.h"

namespace Leviathan{

#define DATAINDEX_TICKTIME				1
#define DATAINDEX_TICKCOUNT				2
#define DATAINDEX_FRAMETIME				3
#define DATAINDEX_FPS					4
#define DATAINDEX_WIDTH					5
#define DATAINDEX_HEIGHT				6

#define DATAINDEX_FRAMETIME_MAX			7
#define DATAINDEX_FRAMETIME_MIN			8
#define DATAINDEX_FRAMETIME_AVERAGE		9

#define DATAINDEX_FPS_MIN				10
#define DATAINDEX_FPS_MAX				11
#define DATAINDEX_FPS_AVERAGE			12

#define DATAINDEX_FONT_SIZEMULTIPLIER	13


#define DATAINDEX_UND			-1
//#define FUNCKREATE(x, y) private: #x #y; public: DLLEXPORT #x Get#y(){ return #y; }; public: DLLEXPORT void Set#y(#x newval){ #y = newval; ValueUpdate(); };


	struct DataListener{
		DLLEXPORT DataListener();
		DLLEXPORT DataListener(int index, bool onindex, const wstring& var = L"");

		int ListenIndex;
		bool ListenOnIndex;
		wstring VarName;

		//AutoUpdateableObject* Object;
	};

	// holds all data listeners related to a object //
	struct DataListenHolder{
		inline ~DataListenHolder(){
			// free memory //
			SAFE_DELETE_VECTOR(HandledListeners);
		}

		vector<DataListener*> HandledListeners;
	};


	class DataStore : public Object{
	public:
		DLLEXPORT DataStore();
		DLLEXPORT DataStore(bool main);
		DLLEXPORT ~DataStore();

		DLLEXPORT void SetPersistance(unsigned int index, bool toset);
		DLLEXPORT void SetPersistance(const wstring &name, bool toset);
		DLLEXPORT int GetPersistance(unsigned int index) const;
		DLLEXPORT int GetPersistance(const wstring &name) const;
		// ------------------------------------ //
		DLLEXPORT bool SetValue(const wstring &name, const VariableBlock &value1);
		DLLEXPORT bool SetValue(const wstring &name, VariableBlock* value1);
		DLLEXPORT bool SetValue(const wstring &name, const vector<VariableBlock*> &values);

		DLLEXPORT bool SetValue(NamedVariableList &nameandvalues);

		DLLEXPORT size_t GetValueCount(const wstring &name) const;

		DLLEXPORT const VariableBlock* GetValue(const wstring &name) const;
		DLLEXPORT bool GetValue(const wstring &name, VariableBlock &receiver) const;
		DLLEXPORT bool GetValue(const wstring &name, const int &nindex, VariableBlock &receiver) const;
		DLLEXPORT bool GetValues(const wstring &name, vector<const VariableBlock*> &receiver) const;


		template<class T>
		DLLEXPORT bool GetValueAndConvertTo(const wstring &name, T &receiver) const{
			// use try block to catch all exceptions (not found and conversion fail //
			try{
				if(!Values.GetValue(name)->ConvertAndAssingToVariable<T>(receiver)){
					// Couldn't convert or assign or find the value //
					return false;
				}
			}
			catch(const ExceptionInvalidArgument &e){
				// variable not found / wrong type //
				return false;
			}
			// correct variable has been set //
			return true;
		}

		//DLLEXPORT vector<VariableBlock*>* GetValues(const wstring &name) throw(...);

		DLLEXPORT void AddVar(NamedVariableList* newvaluetoadd);
		DLLEXPORT void AddVar(shared_ptr<NamedVariableList> values);
		DLLEXPORT void Remove(unsigned int index);
		DLLEXPORT void Remove(const wstring &name);

		DLLEXPORT int GetVariableType(const wstring &name) const;
		DLLEXPORT int GetVariableTypeOfAll(const wstring &name) const;
		// ------------------------------------ //

		DLLEXPORT static DataStore* Get();

		// variables //
	public:
		DLLEXPORT int GetTickTime() const;
		DLLEXPORT int GetTickCount() const;
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
		DLLEXPORT void RemoveListener(AutoUpdateableObject* object, int valueid, const wstring &name = L"", bool all = false);

	private:
		// ------------------------------------ //
		void Load();
		void Save();

		//void _RemoveListener(int index);
		void ValueUpdate(int index);
		void ValueUpdate(const wstring& name);
		// ------------------------------------ //
		NamedVars Values;
		// NamedVariableLists that should be saved to file on quit //
		vector<bool> Persistencestates;


		map<AutoUpdateableObject*, shared_ptr<DataListenHolder>> Listeners;


		//vector<DataListener*> Listeners;



		// vars //
		int TickTime;
		int TickCount;
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

}
#endif
