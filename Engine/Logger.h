#ifndef	LEVIATHAN_LOGGER
#define LEVIATHAN_LOGGER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "boost/thread/thread.hpp"
#include "boost/thread/lockable_adapter.hpp"
#include "boost/thread/recursive_mutex.hpp"
#include "boost/thread/strict_lock.hpp"

namespace Leviathan{


	class Logger : public boost::basic_lockable_adapter<boost::recursive_mutex>{
	public:
		DLLEXPORT Logger(const wstring &file);
		DLLEXPORT Logger(const wstring &file, const wstring &start, const bool &autosave);
		DLLEXPORT ~Logger();

		DLLEXPORT void Write(const wstring &data, const bool &save = false);

		DLLEXPORT void DirectWriteBuffer(const wstring &data);
		DLLEXPORT static void QueueErrorMessage(const wstring& str);

		DLLEXPORT void Info(const wstring &data, const bool &save = false);
		DLLEXPORT void Error(const wstring &data, const int &pvalue = 0, const bool &save = false);
		DLLEXPORT void Warning(const wstring &data, bool save = false);

		DLLEXPORT static void SendDebugMessage(const wstring& str);

		// uses string for script compatibility //
		DLLEXPORT static void Print(string message, bool save = true);


		DLLEXPORT void SetSavePath(const wstring& path);
		DLLEXPORT void inline Save(){
			// thread safety //
			boost::strict_lock<Logger> guard(*this);

			Save(guard);
		}
		DLLEXPORT void Save(boost::strict_lock<Logger> &guard);

		DLLEXPORT static Logger* Get();
		DLLEXPORT static Logger* GetIfExists();
	private:
		void inline CheckQueue(){
			// thread safety //
			boost::strict_lock<Logger> guard(*this);

			CheckQueue(guard);
		}
		void inline CheckQueue(boost::strict_lock<Logger> &guard){
			if(LatestLogger != NULL){
				LatestLogger->PendingLog += QueuedLog;
				// clear //
				QueuedLog.resize(0);
			}
		}

		void inline _LogUpdateEndPart(const bool &save, boost::strict_lock<Logger> &guard);

		// ------------------------------------ //
		wstring PendingLog;
		wstring Path;

		bool Saved;
		bool Autosave;
		bool FirstSaveDone;

		// ------------------------------------ //
		static Logger* LatestLogger;
		static wstring QueuedLog;
	};
}
#endif
