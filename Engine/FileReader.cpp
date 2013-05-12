#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_FILEREADER
#include "FileReader.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::FileReader::FileReader(const wstring& source, bool cachetomemory /*= true*/, unsigned long readbuffersize /*= FILEREADER_DEFAULT_READSIZE*/)
	: SourceFile(source), ReadsInMemory(0), ReadsStartPositions(0), ReadsSize(0)
{
	// set up for reading //
	AllowCaching = cachetomemory;
	ReadBufferSize = readbuffersize;
	LastReadBytes = 0;
	EndOfFileReached = false;
	FileReadPosition = 0;
	ReadingBuffer = NULL;
	FileSearchPosition = 0;

	// open handle //
	if(!OpenFileHandleForReading()){
		EndOfFileReached = true;
		FileReadPosition = 0;
		return;
	}
	IsFileOpen = true;
}

DLLEXPORT Leviathan::FileReader::~FileReader(){
	// close everything //
	CloseNow();
}

DLLEXPORT void Leviathan::FileReader::CloseNow(){
	// close windows handle //
	CloseOpenHandle();
	// release buffers //
	ClearInMemoryBuffers();
	// everything else should be fine to be deleted in destructor //
}
// ------------------------------------ //
DLLEXPORT long Leviathan::FileReader::GetFilePointerPosition(){
	return FileSearchPosition;
}

DLLEXPORT void Leviathan::FileReader::SetFileReadPosition(const long &newpos){
	EndOfFileReached = false;
	FileSearchPosition = newpos;
}
DLLEXPORT bool Leviathan::FileReader::IsEOFReached(){
	return EndOfFileReached;
}
// ------------------------------------ //

// ------------------------------------ //
DLLEXPORT wstring Leviathan::FileReader::ReadNextLine(){
	// get buffer that has data around this search position //
	int buffer = -1;

	while(buffer == -1){
		buffer = GetBufferNearPos();
		// check for no loaded buffers //
		if(buffer == -1){
			// no data around here //
			FileReadPosition = FileSearchPosition*sizeof(wchar_t);
			// read some data //
			if(!ReadNewBufferToMemory()){
				DEBUG_BREAK;
			}
		}
	}
	
	// get buffer //
	wchar_t* CurBuffer = ReadsInMemory[buffer].get();
	DWORD CurBufStartPos = *ReadsStartPositions[buffer].get();
	// set this position as first character to get //
	DWORD CopyStartPos = FileSearchPosition;

	DWORD PosInBuffer = FileSearchPosition-CurBufStartPos;
	DWORD BufferSize = *ReadsSize[buffer].get()+1;

	DWORD CopyEndPositon = 0;
	bool EndFound = false;

	// search for line end //
	for(PosInBuffer; PosInBuffer < BufferSize; PosInBuffer++){
		if(CurBuffer[PosInBuffer] == L'\n'){
			CopyEndPositon = CurBufStartPos+PosInBuffer-1;

			FileSearchPosition = CurBufStartPos+PosInBuffer+1;
			EndFound = true;
			// end found, maybe skip next one too //
			if(PosInBuffer+1 < BufferSize){
				if(CurBuffer[PosInBuffer+1] == L'\r'){
					// skip all the way to next character of this //
					FileSearchPosition += 1;
				}
			} else {
				// would need to check next buffer //
				DEBUG_BREAK;
			}
			break;
		}
	}

	if(!EndFound){
		// advance to next buffer area //
		FileSearchPosition = CurBufStartPos+PosInBuffer-1;
	}

	while(!EndFound){
		// needs to keep searching //
		// increment into next buffer //
		//FileSearchPosition += 1;
		buffer = -1;
		// get next buffer //
		while(buffer == -1){
			buffer = GetBufferNearPos();
			// check for no loaded buffers //
			if(buffer == -1){
				// no data around here //
				FileReadPosition = FileSearchPosition*sizeof(wchar_t);
				// read some data //
				if(!ReadNewBufferToMemory()){
					DEBUG_BREAK;
				}
			}
		}
		// get new buffer //
		CurBuffer = ReadsInMemory[buffer].get();
		// and other data //
		PosInBuffer = 0; // 0 in all new buffers //
		CurBufStartPos = *ReadsStartPositions[buffer].get(); // in the beginning of new search pos //
		BufferSize = *ReadsSize[buffer].get()+1;

		// try to find end //
		for(PosInBuffer = 0; PosInBuffer < BufferSize; PosInBuffer++){
			if(CurBuffer[PosInBuffer] == L'\n'){
				CopyEndPositon = CurBufStartPos+PosInBuffer-1;

				FileSearchPosition = CurBufStartPos+PosInBuffer+1;
				EndFound = true;
				// end found, maybe skip next one too //
				if(PosInBuffer+1 < BufferSize){
					if(CurBuffer[PosInBuffer+1] == L'\r'){
						// skip all the way to next character of this //
						FileSearchPosition += 1;
					}
				} else {
					// would need to check next buffer //
					DEBUG_BREAK;
				}
				break;
			}
		}
		// should go automatically to new buffer //
		if(!EndFound){
			// advance to next buffer area //
			FileSearchPosition = CurBufStartPos+PosInBuffer;
		}
	}

	// return wstring from CopyStartPos to CopyEndPositon //
	return GetWstringOfRegion(CopyStartPos, CopyEndPositon);
}
// ------------------------------------ //

bool Leviathan::FileReader::OpenFileHandleForReading(){
	FileHandle = CreateFile(SourceFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(FileHandle == INVALID_HANDLE_VALUE){

		return false;
	}
	return true;
}

bool Leviathan::FileReader::CloseOpenHandle(){
	// close handle //
	CloseHandle(FileHandle);

	IsFileOpen = false;
	return true;
}

bool Leviathan::FileReader::ReadNewBufferToMemory(){
	if(EndOfFileReached || (FileHandle == INVALID_HANDLE_VALUE))
		return false;
	
	// set position to handle //
	if(SetFilePointer(FileHandle, FileReadPosition, NULL, FILE_BEGIN) != FileReadPosition){
		// failed //
		int errorcode = GetLastError();
		DEBUG_BREAK;
		return false;
	}

	// read the buffer from file //

	// create new buffer //
	if(ReadingBuffer == NULL){
		ReadingBuffer = new wchar_t[ReadBufferSize];
	}

	BOOL readresult = ReadFile(FileHandle, ReadingBuffer, sizeof(wchar_t)*ReadBufferSize,  &LastReadBytes, NULL);
	if(!readresult){
		// error //
		int errorcode = GetLastError();
		return false;
	}

	if(LastReadBytes == 0){
		// file ended //
		EndOfFileReached = true;
	}

	// store start position for this read //
	ReadsStartPositions.push_back(unique_ptr<DWORD>(new DWORD(FileReadPosition/sizeof(wchar_t))));
	ReadsSize.push_back(unique_ptr<DWORD>(new DWORD((LastReadBytes/sizeof(wchar_t))-1))); // -1 because this should be last valid index //

	// get position from handle //
	FileReadPosition = SetFilePointer(FileHandle, NULL, NULL, FILE_CURRENT);

	// put buffer to vector //
	ReadsInMemory.push_back(shared_ptr<wchar_t>(ReadingBuffer));
	ReadingBuffer = NULL;
	return true;
}

void Leviathan::FileReader::ClearInMemoryBuffers(){
	// smart pointers, just clear vectors //
	ReadsInMemory.clear();
	ReadsStartPositions.clear();
	ReadsSize.clear();

	// need to release reading buffer //
	if(ReadingBuffer != NULL)
		SAFE_DELETE(ReadingBuffer);
}

int Leviathan::FileReader::GetBufferNearPos(){
	// find a buffer in whose area search position is in //
	for(unsigned int i = 0; i < ReadsInMemory.size(); i++){
		if(*ReadsSize[i].get()+*ReadsStartPositions[i].get() >= FileSearchPosition){
			if((*ReadsStartPositions[i].get() <= FileSearchPosition)){
				// found correct buffer //
				return (int)i;
			}
		}
	}
	// nothing found //
	return -1;
}

std::wstring Leviathan::FileReader::GetWstringOfRegion(DWORD StartPos, DWORD EndPos){
	// go through buffers and add the area that is correct inside them to result //
	wstring wanterarea = L"";
	for(unsigned int i = 0; i < ReadsInMemory.size(); i++){
		if(*ReadsStartPositions[i].get()+*ReadsSize[i].get() >= StartPos){
			// this has right characters //
			DWORD WantStart = 0;
			if(StartPos >= *ReadsStartPositions[i].get()){
				WantStart = StartPos-*ReadsStartPositions[i].get();
			}

			wchar_t* Buffer = ReadsInMemory[i].get();

			if(*ReadsSize[i].get()+*ReadsStartPositions[i].get() >= EndPos){
				// this buffer contains last bits of wanted stuff //
				DWORD WantEnd = EndPos-*ReadsStartPositions[i].get();

				wstring adding(&Buffer[WantStart], &Buffer[WantEnd]);

				wanterarea += adding;

				break;
			} else {
				// wanted till the end //
				wanterarea += wstring(&Buffer[WantStart], &Buffer[*ReadsSize[i].get()]);
			}
		}
	}


	return wanterarea;
}
