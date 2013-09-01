#ifndef LEVIATHAN_FILEREADERSIMPLE
#define LEVIATHAN_FILEREADERSIMPLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "FileReaderBuffer.h"


#define FILEREADERSIMPLE_DEFAULT_READSIZE		450


namespace Leviathan{

	struct FileBlock{
		FileBlock(__int64 startpos, __int64 endpos, UINT buffsize , bool readbuff = false, wchar_t* buff = NULL){
			// set //
			BlockStartReadPosition = startpos;
			BlockEndReadPosition = endpos;

			BufferSize = buffsize;
			// count //
			BlockLength = endpos-startpos+1;

			IsRead = readbuff;
			Buffer = buff;
		};
		~FileBlock(){
			// release memory //
			SAFE_DELETE(Buffer);
		}

		// what stream says for which tellg spot this starts //
		__int64 BlockStartReadPosition;
		// what stream says at which tellg this block ends //
		__int64 BlockEndReadPosition;
		// BlockEndReadPosition-BlockStartReadPosition+1 theoretical number of characters //
		__int64 BlockLength;
		// actual number of elements not index if used as index -1 should be used //
		UINT BufferSize;

		bool IsRead;
		wchar_t* Buffer;
	};

	class FileReaderSimple : public Object{
	public:
		DLLEXPORT FileReaderSimple::FileReaderSimple(const wstring &source, UINT readbuffersize = FILEREADERSIMPLE_DEFAULT_READSIZE) throw (...);
		DLLEXPORT FileReaderSimple::~FileReaderSimple();

		DLLEXPORT void CloseNow();

		// public handling functions //
		DLLEXPORT bool ReadNextLine(wstring &receiver) throw (...);
		DLLEXPORT void ReadWholeFile(wstring &receiver) throw (...);
		DLLEXPORT void ReadUntilEnd(wstring &receiver) throw (...);

		DLLEXPORT __int64 GetFilePointerPosition();
		DLLEXPORT void SetFileSearchPosition(const __int64 &newpos);

		DLLEXPORT bool IsEOFReached();

	private:
		// private functions //
		bool OpenFileForReading();
		bool CloseOpenFile();
		//bool ReadNewBufferToMemory();
		bool CreateFileSections();
		bool LoadFileSection(int index) throw(...);
		bool LoadAllFileSections();

		void ClearInMemoryBuffers();
		int GetBufferContainingPosition(const __int64 &pos);
		int GetBufferIndexInSearchPosition();

		wstring GetWstringOfRegion(__int64 StartPos, __int64 EndPos);


		// data //
		bool IsFileOpen;
		// file position markers //
		__int64 FileSearchPosition;

		__int64 FileReadPosition;
		__int64 LastReadAmount;
		// the length of file, in tellg amount //
		__int64 FileLength;

		bool EndOfFileReached;
		// name/path to the actual file //
		wstring SourceFile;

		// actual in reading //
		wifstream Reader;
		wchar_t* ReadingBuffer;
		UINT ReadBufferSize;

		// read cache //
		vector<shared_ptr<FileBlock>> FileIndexBlocks;
	};

}
#endif