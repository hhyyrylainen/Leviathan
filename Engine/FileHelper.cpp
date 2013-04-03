#include "Include.h"
#include "FileHelper.h"

HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD &ChunkSize, DWORD &ChunkDataPosition){
    HRESULT hr = S_OK;
    if(INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
        return HRESULT_FROM_WIN32(GetLastError());

    DWORD ChunkType;
    DWORD ChunkDataSize;
    DWORD RIFFDataSize = 0;
    DWORD FileType;
    DWORD bytesRead = 0;
    DWORD Offset = 0;

    while(hr == S_OK){
        DWORD Read;

        if(0 == ReadFile(hFile, &ChunkType, sizeof(DWORD), &Read, NULL))
            hr = HRESULT_FROM_WIN32(GetLastError());

        if(0 == ReadFile(hFile, &ChunkDataSize, sizeof(DWORD), &Read, NULL))
            hr = HRESULT_FROM_WIN32(GetLastError());

        switch (ChunkType){
        case fourccRIFF:
			{
				RIFFDataSize = ChunkDataSize;
				ChunkDataSize = 4;
				if(0 == ReadFile(hFile, &FileType, sizeof(DWORD), &Read, NULL))
					hr = HRESULT_FROM_WIN32(GetLastError());
			}
        break;
        default:
            if(INVALID_SET_FILE_POINTER == SetFilePointer(hFile, ChunkDataSize, NULL, FILE_CURRENT))
            return HRESULT_FROM_WIN32(GetLastError());            
        }

        Offset += sizeof(DWORD) * 2;
        
        if(ChunkType == fourcc){
			// correct chunk found //
            ChunkSize = ChunkDataSize;
            ChunkDataPosition = Offset;
            return S_OK;
        }

        Offset += ChunkDataSize;
        
        if(bytesRead >= RIFFDataSize)
			return S_FALSE;

    }

    return S_OK;
}


HRESULT ReadChunkData(HANDLE hFile, void * buffer, DWORD buffersize, DWORD bufferoffset){
    HRESULT hr = S_OK;

    if(INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
        return HRESULT_FROM_WIN32(GetLastError());

    DWORD dwRead;

    if(0 == ReadFile( hFile, buffer, buffersize, &dwRead, NULL))
        hr = HRESULT_FROM_WIN32(GetLastError());

    return hr;
}