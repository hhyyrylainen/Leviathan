#ifndef LEVIATHAN_FILEREADERBUFFER
#define LEVIATHAN_FILEREADERBUFFER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{
	// WARNING: Don't directly copy these around, because this doesn't use deep copy so T* will be deleted by first object to go out of scope and the second
	// will seriously ruin your day //
	template<typename T>
	class FileReaderBuffer{
	public:
		DLLEXPORT FileReaderBuffer::FileReaderBuffer(T* Buffer, __int64 startposition, __int64 endposition){
			// set up values //
			MainBuffer = Buffer;

			BufferStartPosition = startposition;
			BufferEndPosition = endposition;

			// count lengths //
			BufferBitLenght = endposition-startposition+1;
			//BufferElementCount = BufferBitLenght/sizeof(T);
			BufferElementCount = BufferBitLenght;
		}

		DLLEXPORT FileReaderBuffer::~FileReaderBuffer(){
			// release memory //
			SAFE_DELETE(MainBuffer);
		}

		DLLEXPORT T* GetBuffer(){
			return MainBuffer;
		}

		DLLEXPORT __int64 GetStartPos(){
			return BufferStartPosition;
		}

		DLLEXPORT __int64 GetEndPos(){
			return BufferEndPosition;
		}

		DLLEXPORT __int64 GetElementCount(){
			return BufferElementCount;
		}
		DLLEXPORT __int64 GetBitCount(){
			return BufferBitLenght;
		}

		DLLEXPORT void LetGoOfBuffer(){
			// no longer point to main buffer //
			MainBuffer = NULL;
		}

	private:
		T* MainBuffer;

		__int64 BufferElementCount;
		__int64 BufferBitLenght;

		__int64 BufferStartPosition;
		__int64 BufferEndPosition;
	};
}
#endif