#ifndef LEVIATHAN_DATASTORE
#define LEVIATHAN_DATASTORE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "NamedVars.h"
#include "FileSystem.h"
#include "AutoUpdateable.h"

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


#define DATAINDEX_UND			-1
//#define FUNCKREATE(x, y) private: #x #y; public: DLLEXPORT #x Get#y(){ return #y; }; public: DLLEXPORT void Set#y(#x newval){ #y = newval; ValueUpdate(); };
	struct DataListener{
			DataListener();
			DataListener(int index, bool onindex, AutoUpdateableObject* obj, const wstring& var = L"");
			int ListenIndex;
			bool ListenOnIndex;
			wstring VarName;

			AutoUpdateableObject* Object;

			bool Termination;
		};

	class DataStore : public Object{
	public:
		DLLEXPORT DataStore::DataStore();
		DLLEXPORT DataStore::DataStore(bool main);
		DLLEXPORT DataStore::~DataStore();

		DLLEXPORT static DataStore* Get();

		DLLEXPORT int GetValue(const wstring& name) const;
		DLLEXPORT int GetValue(const int &index) const;
		DLLEXPORT bool SetValue(const wstring &name, int value);
		DLLEXPORT bool SetValue(const int &index, int value);


		DLLEXPORT int GetIndex(const wstring &name) const;
		DLLEXPORT int SeekIndex(const int &value, const wstring &partofname);


		DLLEXPORT void SetPersistance(unsigned int index, bool toset);
		DLLEXPORT void SetPersistance(const wstring &name, bool toset);
		DLLEXPORT int GetPersistance(unsigned int index) const;
		DLLEXPORT int GetPersistance(const wstring &name) const;


		DLLEXPORT bool AddValue(const wstring &name, const int &data);
		DLLEXPORT bool AddValueIfDoesntExist(const wstring &name, const int &data);
		DLLEXPORT bool RemoveVariable(const wstring &name);
		DLLEXPORT bool RemoveVariable(int index);

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




		// height/width //
		DLLEXPORT void SetWidth(int newval);
		DLLEXPORT void SetHeight(int newval);

		DLLEXPORT int GetValueFromValIndex(int valindex) const;
	public:
		DLLEXPORT void RegisterListener(DataListener* listen);
		DLLEXPORT void RemoveListener(AutoUpdateableObject* object, int valueid, const wstring &name = L"", bool all = false);

	private:


		// ----------------- //
		void Load();
		void Save();

		//void ValueUpdate(int valindex);


		//void NameVarUpdate(wstring &name);

		void _RemoveListener(int index);
		void ValueUpdate(int index);
		void ValueUpdate(const wstring& name);
		// ----------------- //
		NamedVars values;
		vector<bool> Persistancestates;
		vector<DataListener*> Listeners;

		static DataStore* Staticaccess;

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

	};

}
#endif