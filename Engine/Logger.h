#ifndef	LEVIATHAN_LOGGER
#define LEVIATHAN_LOGGER
// ------------------------------------ //
// To reduce bloat in precompiled header
// ------------------------------------ //
// ---- includes ---- //
#include "boost/thread/lockable_adapter.hpp"
#include "boost/thread/mutex.hpp"
#include "boost/thread/strict_lock.hpp"

namespace Leviathan{

    //! \todo Change to use utf8 strings
	class Logger : public boost::basic_lockable_adapter<boost::mutex>{
	public:
		DLLEXPORT Logger(const wstring &file);
		DLLEXPORT Logger(const wstring &file, const wstring &start, const bool &autosave);
		DLLEXPORT ~Logger();

		DLLEXPORT void Write(const wstring &data, const bool &save = false);
        DLLEXPORT void Write(const string &data, const bool &save = false);

		//! \brief Adds raw data to the queue unmodified (don't forget line ends!)
		DLLEXPORT void DirectWriteBuffer(const wstring &data);

		DLLEXPORT void Info(const wstring &data, const bool &save = false);
        DLLEXPORT void Info(const string &data, const bool &save = false);
		DLLEXPORT void Error(const wstring &data, const int &pvalue = 0, const bool &save = false);
        DLLEXPORT void Error(const string &data, const int &pvalue = 0, const bool &save = false);
		DLLEXPORT void Warning(const wstring &data, bool save = false);
        DLLEXPORT void Warning(const string &data, bool save = false);

		DLLEXPORT static void SendDebugMessage(const wstring& str, boost::strict_lock<Logger> &guard);

		// uses string for script compatibility //
		DLLEXPORT static void Print(string message, bool save = true);

		DLLEXPORT void inline Save(){
			// thread safety //
			boost::strict_lock<Logger> guard(*this);

			Save(guard);
		}
		DLLEXPORT void Save(boost::strict_lock<Logger> &guard);

		DLLEXPORT static Logger* Get();
		DLLEXPORT static Logger* GetIfExists();
	private:
        
		void inline _LogUpdateEndPart(const bool &save, boost::strict_lock<Logger> &guard);

		// ------------------------------------ //
		wstring PendingLog;
		wstring Path;

		// ------------------------------------ //
		static Logger* LatestLogger;
	};
}
#endif
