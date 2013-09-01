#ifndef LEVIATHAN_FILEREADER
#define LEVIATHAN_FILEREADER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //

#define FILEREADER_DEFAULT_READSIZE		450

#ifndef WIN32
#error FileReader.h requires windows file i/o functions
#endif

namespace Leviathan{

	class FileReader : public Object{
	public:
		DLLEXPORT FileReader::FileReader(const wstring& source, bool cachetomemory = true, unsigned long readbuffersize = FILEREADER_DEFAULT_READSIZE);
		DLLEXPORT FileReader::~FileReader();

		DLLEXPORT void CloseNow();

		// public handling functions //
		DLLEXPORT wstring ReadNextLine();

		DLLEXPORT long GetFilePointerPosition();
		DLLEXPORT void SetFileReadPosition(const long &newpos);

		DLLEXPORT bool IsEOFReached();

	private:
		// private functions //
		bool OpenFileHandleForReading();
		bool CloseOpenHandle();
		bool ReadNewBufferToMemory();
		void ClearInMemoryBuffers();
		int GetBufferNearPos();
		wstring GetWstringOfRegion(DWORD StartPos, DWORD EndPos);

		// data //
		bool IsFileOpen;
		// this is used by get functions //
		DWORD FileSearchPosition;
		// this on the other hand is used by internal functions //
		DWORD FileReadPosition;
		DWORD LastReadBytes;
		bool EndOfFileReached;

		wstring SourceFile;

		// actual in reading //
		HANDLE FileHandle;
		wchar_t* ReadingBuffer;
		ULONG ReadBufferSize;

		// read cache //
		bool AllowCaching;
		vector<shared_ptr<wchar_t>> ReadsInMemory;
		vector<unique_ptr<DWORD>> ReadsStartPositions;
		vector<unique_ptr<DWORD>> ReadsSize;
	};

}
#endif