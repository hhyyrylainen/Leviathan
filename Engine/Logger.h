#ifndef	LEVIATHAN_LOGGER
#define LEVIATHAN_LOGGER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
namespace Leviathan{
	class Logger{
	public:
		DLLEXPORT Logger();
		DLLEXPORT Logger(const wstring &start, bool autosave);
		DLLEXPORT ~Logger();

		DLLEXPORT void Write(const wstring &data);
		DLLEXPORT void DirectWriteBuffer(const wstring &data);
		DLLEXPORT void Info(const wstring &data);
		DLLEXPORT void Error(const wstring &data, int value);
		DLLEXPORT void Error(const wstring &data);
		DLLEXPORT void Write(const wstring &data, bool save);
		DLLEXPORT void Info(const wstring &data, bool save);
		DLLEXPORT void Error(const wstring &data, int value, bool save);
		DLLEXPORT void Warning(const wstring &data, bool save = false);

		DLLEXPORT static void SendDebugMessage(const wstring& str);
		DLLEXPORT static void QueueErrorMessage(const wstring& str);

		// uses string for script compatibility // 
		DLLEXPORT static void Print(string message, bool save = true);

		DLLEXPORT wstring GetText(){ return log; };
		DLLEXPORT void SetText(const wstring &data){ log = data; };
		DLLEXPORT void SetSavePath(const wstring& path);

		DLLEXPORT void Save();

		DLLEXPORT static Logger* Get();
		DLLEXPORT static Logger* GetIfExists();
	private:
		void inline CheckQueue();
		void WaitToFinish();

		static Logger* LatestLogger;
		static wstring QueuedLog;

		wstring log;
		wstring PendingLog;
		wstring Path;
		bool Saved;
		bool Autosave;

		bool FirstSaveDone;

		bool _inuse;

	};
}
#endif