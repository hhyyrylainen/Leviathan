#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_FILEREADERSIMPLE
#include "FileReaderSimple.h"
#endif
using namespace Leviathan;
// ------------------------------------ //


DLLEXPORT Leviathan::FileReaderSimple::FileReaderSimple(const wstring& source, UINT readbuffersize /*= FILEREADERSIMPLE_DEFAULT_READSIZE*/) 
	throw (...) : SourceFile(source), FileIndexBlocks(0), ReadBufferSize(readbuffersize), LastReadAmount(0), 
	FileReadPosition(0), ReadingBuffer(NULL), FileSearchPosition(0), Reader(), FileLength(0)
{
	// set up for reading //
	EndOfFileReached = false;
	IsFileOpen = false;

	// open reader //
	if(!OpenFileForReading()){
		EndOfFileReached = true;
		throw ExceptionInvalidArguement(L"could not read file", 0, __WFUNCTION__,L"source(\""+SourceFile+L"\")");
		return;
	}
	IsFileOpen = true;
}

DLLEXPORT Leviathan::FileReaderSimple::~FileReaderSimple(){
	// close everything //
	CloseNow();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::FileReaderSimple::CloseNow(){
	// close windows handle //
	CloseOpenFile();
	// release buffers //
	ClearInMemoryBuffers();
	// remove indexed sections //
	FileIndexBlocks.clear();
	// everything else should be fine to be deleted in destructor //
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::FileReaderSimple::ReadNextLine(wstring& receiver)  throw (...) {
	//if(!IsFileOpen){
	//	// well shit //
	//	receiver = L"";
	//	throw ExceptionInvalidArguement(L"could not read file", 0, __WFUNCTION__,L"internal SourceFile(\""+SourceFile+L"\")");
	//}

	//if(FileSearchPosition >= FileLength/FILEREADERSIMPLE_POSITIONRELATIVITY){
	//	// ended //
	//	receiver = L"";
	//	return false;
	//}

	//// set this position as first character to get //
	//__int64 CopyStartPos = FileSearchPosition*FILEREADERSIMPLE_POSITIONRELATIVITY;
	//__int64 CopyEndPositon = 0;

	//bool EndFound = false;

	//while(!EndFound){
	//	// needs to keep searching //
	//	// get buffer that has data around this search position/get next buffer //
	//	int bufferindex = GetBufferIndexInSearchPosition();

	//	if(bufferindex == -2){
	//		// file ended //
	//		// return from start to last position in file //

	//		__int64 MaxEnd = 0;

	//		for(unsigned int i = 0; i < FileIndexBlocks.size(); i++){
	//			if(FileIndexBlocks[i]->GetEndPos() > MaxEnd){
	//				MaxEnd = FileIndexBlocks[i]->GetEndPos();
	//			}
	//		}

	//		CopyEndPositon = MaxEnd;
	//		// set search position over length //
	//		FileSearchPosition = FileLength/FILEREADERSIMPLE_POSITIONRELATIVITY;
	//		EndFound = true;
	//		break;
	//	}

	//	// get pointer to buffer //
	//	shared_ptr<FileReaderBuffer<wchar_t>> CurBuffer = FileIndexBlocks[bufferindex];
	//	// and other data //
	//	// calculate Bit position inside buffer // // this needs to be 0 in iterations //
	//	__int64 PosInBuffer = FileSearchPosition-(CurBuffer->GetStartPos()/FILEREADERSIMPLE_POSITIONRELATIVITY);

	//	// search for line end //
	//	for(PosInBuffer; PosInBuffer < CurBuffer->GetElementCount(); PosInBuffer++){
	//		if(CurBuffer->GetBuffer()[PosInBuffer] == L'\n'){
	//			CopyEndPositon = CurBuffer->GetStartPos()+(PosInBuffer*FILEREADERSIMPLE_POSITIONRELATIVITY-1);

	//			FileSearchPosition = (CopyEndPositon/FILEREADERSIMPLE_POSITIONRELATIVITY)+2;
	//			EndFound = true;
	//			// end found, maybe skip next one too //
	//			if(PosInBuffer+1 < CurBuffer->GetElementCount()){
	//				if(CurBuffer->GetBuffer()[PosInBuffer+1] == L'\r'){
	//					// skip all the way to next character of this //
	//					FileSearchPosition += 1;
	//				}
	//			} else {
	//				// would need to check next buffer //
	//				// get that buffer //
	//				bufferindex = GetBufferIndexInSearchPosition();
	//				// point CurBuffer //
	//				CurBuffer = FileIndexBlocks[bufferindex];
	//				// search begin of the new buffer //
	//				if(CurBuffer->GetBuffer()[0] == L'\r'){
	//					// skip all the way to next character of this //
	//					FileSearchPosition += 1;
	//				}
	//			}
	//			// positions set properly, time to return result //
	//			break;
	//		}
	//	}

	//	// should go automatically to new buffer //
	//	if(!EndFound){
	//		// advance to next buffer area //
	//		FileSearchPosition = (CurBuffer->GetEndPos()/FILEREADERSIMPLE_POSITIONRELATIVITY)+1;
	//	}
	//}

	//// return wstring from CopyStartPos to CopyEndPositon //
	//receiver = GetWstringOfRegion(CopyStartPos, CopyEndPositon);

	return true;
}
DLLEXPORT void Leviathan::FileReaderSimple::ReadWholeFile(wstring &receiver) throw (...){
	// return all data in all buffers //
	receiver.clear();

	for(unsigned int i = 0; i < FileIndexBlocks.size(); i++){
		if(!LoadFileSection((int)i)){

			DEBUG_BREAK;
			continue;
		}

		wchar_t* buffer = FileIndexBlocks[i]->Buffer;

		// create wstring from buffer and add //
		receiver += wstring(&buffer[0], &buffer[FileIndexBlocks[i]->BufferSize-1]);
	}



	// done //
}

DLLEXPORT void Leviathan::FileReaderSimple::ReadUntilEnd(wstring &receiver) throw (...){

}

// ------------------------------------ //
DLLEXPORT __int64 Leviathan::FileReaderSimple::GetFilePointerPosition(){
	return FileSearchPosition;
}

DLLEXPORT void Leviathan::FileReaderSimple::SetFileSearchPosition(const __int64 &newpos){
	FileSearchPosition = newpos;
}

DLLEXPORT bool Leviathan::FileReaderSimple::IsEOFReached(){
	return EndOfFileReached;
}
// ------------------------------------ //
bool Leviathan::FileReaderSimple::OpenFileForReading(){
	// open file//
	Reader.open(SourceFile.c_str());
	// return is it open //
	if(Reader.is_open()){
		// count length of file //
		// go to end
		Reader.seekg(0, ios::end);
		FileLength = Reader.tellg();
		// go back to beginning //
		Reader.seekg(0, ios::beg);
		// needs to create sections in file to allow reading //
		return CreateFileSections();
	}
	return false;
}

bool Leviathan::FileReaderSimple::CloseOpenFile(){
	// close reader //
	Reader.close();

	IsFileOpen = false;
	return true;
}

bool Leviathan::FileReaderSimple::CreateFileSections(){
	// estimate required space and reserve //
	int estimate = (int)((FileLength / (float)ReadBufferSize)+0.5f);

	FileIndexBlocks.reserve(estimate+1);

	for(__int64 i = 0; i < FileLength; i+=ReadBufferSize){
		// allocate new Index block and fill it //
		FileIndexBlocks.push_back(shared_ptr<FileBlock>(new FileBlock(i, ReadBufferSize-1, ReadBufferSize)));
	}
	return true;
}
// ------------------------------------ //
bool Leviathan::FileReaderSimple::LoadFileSection(int index) throw(...){
	// find right index //
	ARR_INDEX_CHECKINV(index, (int)FileIndexBlocks.size()){
		// invalid index //
		throw ExceptionInvalidArguement(L"Invalid index", index, __WFUNCTION__, L"index");
	}

	FileBlock* tempptr = FileIndexBlocks[index].get();

	if(tempptr->IsRead)
		return true;

	// reset reader state //
	Reader.clear();

	// set position to reader //
	Reader.seekg(tempptr->BlockStartReadPosition);

	if(!Reader.good()){

		DEBUG_BREAK;
		return false;
	}

	// read the buffer from file //

	// create new buffer if needed //
	if(ReadingBuffer == NULL){
		ReadingBuffer = new wchar_t[tempptr->BufferSize];
	}

	// ensure a null terminator //
	ReadingBuffer[tempptr->BufferSize-1] = L'\0';
	// -1 so that doesn't overwrite the null terminator //
	Reader.read(ReadingBuffer, tempptr->BufferSize-1);

	LastReadAmount = wcslen(ReadingBuffer);
	//LastReadAmount = Reader.gcount();

	if(LastReadAmount != tempptr->BlockLength){

		//DEBUG_BREAK;
	}

	// store this read //
	tempptr->Buffer = ReadingBuffer;
	// buffer object now handles the buffer //
	ReadingBuffer = NULL;

	// get position from reader //
	FileReadPosition = Reader.tellg();
	
	if(FileReadPosition != tempptr->BlockEndReadPosition){

		DEBUG_BREAK;
	}
	// mark as read buffer //
	tempptr->IsRead = true;

	return true;
}

bool Leviathan::FileReaderSimple::LoadAllFileSections(){
	for(unsigned int i = 0; i < FileIndexBlocks.size(); i++){
		if(!LoadFileSection((int)i)){

			return false;
		}
	}
	return true;
}

void Leviathan::FileReaderSimple::ClearInMemoryBuffers(){
	// smart pointers, just clear vectors //
	for(unsigned int i = 0; i < FileIndexBlocks.size(); i++){
		// delete it's buffer //
		SAFE_DELETE(FileIndexBlocks[i]->Buffer);
		FileIndexBlocks[i]->IsRead = false;
	}

	// need to release reading buffer //
	SAFE_DELETE(ReadingBuffer);

}

int Leviathan::FileReaderSimple::GetBufferContainingPosition(const __int64 &pos){
	// find a buffer in whose area search position is in //
	for(unsigned int i = 0; i < FileIndexBlocks.size(); i++){
		if(FileIndexBlocks[i]->BlockStartReadPosition <= pos){
			if(FileIndexBlocks[i]->BlockEndReadPosition >= pos){
				// found correct buffer //
				return (int)i;
			}
		}
	}
	// nothing found //
	return -1;
}

int Leviathan::FileReaderSimple::GetBufferIndexInSearchPosition(){
	int bufferindex = -1;

	while(bufferindex == -1){
		bufferindex = GetBufferContainingPosition(FileSearchPosition);
		// load if not loaded //
		if(!LoadFileSection(bufferindex)){

			DEBUG_BREAK;
		}
		break;
	}
	return bufferindex;
}
// ------------------------------------ //
std::wstring Leviathan::FileReaderSimple::GetWstringOfRegion(__int64 StartPos, __int64 EndPos){
	// go through buffers and add the area that is correct inside them to result //
	wstring wantedrarea = L"";
	//for(unsigned int i = 0; i < FileIndexBlocks.size(); i++){
	//	if(FileIndexBlocks[i]->GetEndPos() >= StartPos){
	//		// this has right characters //
	//		__int64 WantStart = 0;
	//		if(StartPos >= FileIndexBlocks[i]->GetStartPos()){
	//			WantStart = StartPos-FileIndexBlocks[i]->GetStartPos();
	//		}

	//		wchar_t* Buffer = FileIndexBlocks[i]->GetBuffer();

	//		if(FileIndexBlocks[i]->GetEndPos() >= EndPos){
	//			// this buffer contains last bits of wanted stuff //
	//			__int64 WantExclude = (FileIndexBlocks[i]->GetEndPos()-EndPos)/FILEREADERSIMPLE_POSITIONRELATIVITY;

	//			wantedrarea += wstring(&Buffer[WantStart/FILEREADERSIMPLE_POSITIONRELATIVITY], &Buffer[FileIndexBlocks[i]->GetElementCount()-WantExclude]);
	//			// result is now complete //
	//			break;
	//		} else {
	//			// wanted till the end //
	//			// last element is always null terminator, so don't worry about going over the bounds //
	//			wantedrarea += wstring(&Buffer[WantStart/FILEREADERSIMPLE_POSITIONRELATIVITY], &Buffer[FileIndexBlocks[i]->GetElementCount()]);
	//			continue;
	//		}
	//	}
	//}

	return wantedrarea;
}