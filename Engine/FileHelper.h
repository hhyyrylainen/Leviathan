#ifndef LEVIATHAN_SOUND_FILEHELPER
#define LEVIATHAN_SOUND_FILEHELPER
// ------------------------------------ //
#include "Define.h"

#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'
// ------------------------------------ //

HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD &ChunkSize, DWORD &ChunkDataPosition);


HRESULT ReadChunkData(HANDLE hFile, void * buffer, DWORD buffersize, DWORD bufferoffset);


#endif

