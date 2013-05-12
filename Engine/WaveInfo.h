#ifndef LEVIATHAN_SOUND_WAVEINFO
#define LEVIATHAN_SOUND_WAVEINFO
// ------------------------------------ //
#include "Logger.h"
#include "FileSystem.h"

#include <XAudio2.h>
#include <XAudio2fx.h>
#include <MMSystem.h>
// ------------------------------------ //
// ---- PARTIAL "CLASS" FILE ---- //


// code loaned from //
//by Jay Tennant 3/8/12
//A Brief Look at XAudio2: Playing a Stream
//demonstrates streaming a wave from disk
//win32developer.com
//this code provided free, as in public domain; score!

class WaveInfo
{
private:
	WAVEFORMATEXTENSIBLE m_wf;
	DWORD m_dataOffset;
	DWORD m_dataLength;

protected:
	//looks for the FOURCC chunk, returning -1 on failure
	DWORD findChunk(HANDLE hFile, FOURCC cc, BYTE* memBuffer, DWORD sectorAlignment) {
		DWORD dwChunkId = 0;
		DWORD dwChunkSize = 0;
		DWORD i = 0; //guaranteed to be always aligned with the sectors, except when done searching
		OVERLAPPED overlapped = {0};
		DWORD sectorOffset = 0;
		DWORD bytesRead = 0;

		bool searching = true;
		while(searching){

			sectorOffset = 0;
			overlapped.Offset = i;
			if(FALSE == ReadFile( hFile, memBuffer, sectorAlignment, &bytesRead, &overlapped)){
				//Logger::Get()->Error((L"Undocumented error: WaveInfo.h line: "+__LINE__), true);
				return (DWORD)-1;
			}

			bool needAnotherRead = false;
			while(searching && !needAnotherRead){

				if(8 + sectorOffset > sectorAlignment){ //reached the end of our memory buffer
					needAnotherRead = true;
				}
				else if(8 + sectorOffset > bytesRead){ //reached EOF, and not found a match
					//Logger::Get()->Error((L"Undocumented error: WaveInfo.h line: "+__LINE__), true);
					return (DWORD)-1;
				} else { //looking through the read memory

					dwChunkId = *reinterpret_cast<DWORD*>( memBuffer + sectorOffset );
					dwChunkSize = *reinterpret_cast<DWORD*>( memBuffer + sectorOffset + 4 );

					if(dwChunkId == cc){ //found a match
						searching = false;
						i += sectorOffset;
					} else { //no match found, add to offset

						dwChunkSize += 8; //add offsets of the chunk id, and chunk size data entries
						dwChunkSize += 1;
						dwChunkSize &= 0xfffffffe; //guarantees WORD padding alignment

						if(i == 0 && sectorOffset == 0) //just in case we're at the 'RIFF' chunk; the dwChunkSize here means the entire file size
							sectorOffset += 12;
						else
							sectorOffset += dwChunkSize;
					}
				}
			}

			//if still searching, search the next sector
			if(searching){
				i += sectorAlignment;
			}
		}

		return i;
	}

	//reads a certain amount of data in, returning the number of bytes copied
	DWORD readData(HANDLE hFile, DWORD bytesToRead, DWORD fileOffset, void* pDest, BYTE* memBuffer, DWORD sectorAlignment){
		if(bytesToRead == 0){
			//Logger::Get()->Error((L"Undocumented error: WaveInfo.h line: "+__LINE__), true);
			return 0;
		}

		DWORD totalAmountCopied = 0;
		DWORD copyBeginOffset = fileOffset % sectorAlignment;
		OVERLAPPED overlapped = {0};
		bool fetchingData = true;
		DWORD pass = 0;
		DWORD dwNumberBytesRead = 0;

		while(fetchingData){

			//calculate the sector to read
			overlapped.Offset = fileOffset - (fileOffset % sectorAlignment) + pass * sectorAlignment;

			//read the amount in; if the read failed, return 0
			if(FALSE == ReadFile(hFile, memBuffer, sectorAlignment, &dwNumberBytesRead, &overlapped)){
				//Logger::Get()->Error((L"Undocumented error: WaveInfo.h line: "+__LINE__), true);
				return 0;
			}

			//if the full buffer was not filled (ie. EOF)
			if(dwNumberBytesRead < sectorAlignment){

				//calculate how much can be copied
				DWORD amountToCopy = 0;
				if(dwNumberBytesRead > copyBeginOffset)
					amountToCopy = dwNumberBytesRead - copyBeginOffset;
				if(totalAmountCopied + amountToCopy > bytesToRead)
					amountToCopy = bytesToRead - totalAmountCopied;

				//copy that amount over
				memcpy(((BYTE*)pDest) + totalAmountCopied, memBuffer + copyBeginOffset, amountToCopy);

				//add to the total amount copied
				totalAmountCopied += amountToCopy;

				//end the fetching data loop
				fetchingData = false;
			} else {
				//calculate how much can be copied
				DWORD amountToCopy = sectorAlignment - copyBeginOffset;
				if( totalAmountCopied + amountToCopy > bytesToRead )
					amountToCopy = bytesToRead - totalAmountCopied;

				//copy that amount over
				memcpy( ((BYTE*)pDest) + totalAmountCopied, memBuffer + copyBeginOffset, amountToCopy );

				//add to the total amount copied
				totalAmountCopied += amountToCopy;

				//set the copyBeginOffset to 0
				copyBeginOffset = 0;
			}

			//if the total amount equals the bytesToRead, end the fetching data loop
			if(totalAmountCopied == bytesToRead)
				fetchingData = false;

			//increment the pass
			pass++;
		}

		//return the total amount copied
		return totalAmountCopied;
	}

public:
	WaveInfo(LPCTSTR szFile = NULL) : m_dataOffset(0), m_dataLength(0) {
		memset( &m_wf, 0, sizeof(m_wf) );
		load( szFile );
	}
	WaveInfo(const WaveInfo& c) : m_wf(c.m_wf), m_dataOffset(c.m_dataOffset), m_dataLength(c.m_dataLength) {}

	//loads the wave format, offset to the wave data, and length of the wave data;
	//returns true on success, false on failure
	bool load(LPCTSTR szFile){
		memset(&m_wf, 0, sizeof(m_wf));
		m_dataOffset = 0;
		m_dataLength = 0;

		if(szFile == NULL)
			return false;

		//load the file without system cacheing
		HANDLE hFile = CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);

		if(hFile == INVALID_HANDLE_VALUE)
			return false;

		//figure the sector size for reading
		DWORD dwSectorSize = 0;
		{
			DWORD dw1, dw2, dw3;
			GetDiskFreeSpace(NULL, &dw1, &dwSectorSize, &dw2, &dw3);
		}

		//allocate the aligned memory buffer, used in finding and reading the chunks in the file
		BYTE *memBuffer = (BYTE*)_aligned_malloc(dwSectorSize, dwSectorSize);
		if(memBuffer == NULL){

			//Logger::Get()->Error((L"Undocumented error: WaveInfo.h line: "+__LINE__), true);
			CloseHandle( hFile );
			return false;
		}

		//look for 'RIFF' chunk
		DWORD dwChunkOffset = findChunk(hFile, MAKEFOURCC( 'R', 'I', 'F', 'F' ), memBuffer, dwSectorSize);
		if(dwChunkOffset == -1){

			//Logger::Get()->Error((L"Undocumented error: WaveInfo.h line: "+__LINE__), true);
			_aligned_free( memBuffer );
			CloseHandle( hFile );
			return false;
		}

		DWORD riffFormat = 0;
		//inFile.seekg( dwChunkOffset + 8, std::ios::beg );
		//inFile.read( reinterpret_cast<char*>(&riffFormat), sizeof(riffFormat) );
		if(sizeof(DWORD) != readData( hFile, sizeof(riffFormat), dwChunkOffset + 8, &riffFormat, memBuffer, dwSectorSize)){

			//Logger::Get()->Error((L"Undocumented error: WaveInfo.h line: "+__LINE__), true);
			_aligned_free( memBuffer );
			CloseHandle( hFile );
			return false;
		}
		if(riffFormat != MAKEFOURCC('W', 'A', 'V', 'E')){

			//Logger::Get()->Error((L"Undocumented error: WaveInfo.h line: "+__LINE__), true);
			_aligned_free( memBuffer );
			CloseHandle( hFile );
			return false;
		}

		//look for 'fmt ' chunk
		dwChunkOffset = findChunk(hFile, MAKEFOURCC( 'f', 'm', 't', ' ' ), memBuffer, dwSectorSize);
		if( dwChunkOffset == -1 ){

			//Logger::Get()->Error((L"Undocumented error: WaveInfo.h line: "+__LINE__), true);
			_aligned_free( memBuffer );
			CloseHandle( hFile );
			return false;
		}

		//read in first the WAVEFORMATEX structure
		if(sizeof(m_wf.Format) != readData( hFile, sizeof(m_wf.Format), dwChunkOffset + 8, &m_wf.Format, memBuffer, dwSectorSize)){

			//Logger::Get()->Error((L"Undocumented error: WaveInfo.h line: "+__LINE__), true);
			_aligned_free( memBuffer );
			CloseHandle( hFile );
			return false;
		}

		if(m_wf.Format.cbSize == (sizeof(m_wf) - sizeof(m_wf.Format))){

			//read in whole WAVEFORMATEXTENSIBLE structure
			if(sizeof(m_wf) != readData(hFile, sizeof(m_wf), dwChunkOffset + 8, &m_wf, memBuffer, dwSectorSize)){

				//Logger::Get()->Error((L"Undocumented error: WaveInfo.h line: "+__LINE__), true);
				_aligned_free( memBuffer );
				CloseHandle( hFile );
				return false;
			}
		}

		//look for 'data' chunk
		dwChunkOffset = findChunk(hFile, MAKEFOURCC( 'd', 'a', 't', 'a' ), memBuffer, dwSectorSize);
		if(dwChunkOffset == -1){

			//Logger::Get()->Error((L"Undocumented error: WaveInfo.h line: "+__LINE__), true);
			_aligned_free( memBuffer );
			CloseHandle( hFile );
			return false;
		}

		//set the offset to the wave data, read in length, then return
		m_dataOffset = dwChunkOffset + 8;
		//inFile.seekg( dwChunkOffset + 4, std::ios::beg );
		//inFile.read( reinterpret_cast<char*>(&m_dataLength), 4 );
		if(sizeof(m_dataLength) != readData(hFile, sizeof(m_dataLength), dwChunkOffset + 4, &m_dataLength, memBuffer, dwSectorSize)){

			//Logger::Get()->Error((L"Undocumented error: WaveInfo.h line: "+__LINE__), true);
			_aligned_free( memBuffer );
			CloseHandle( hFile );
			return false;
		}

		_aligned_free( memBuffer );

		CloseHandle( hFile );

		return true;
	}

	//returns true if the format is WAVEFORMATEXTENSIBLE; false if WAVEFORMATEX
	bool isExtensible() const { return (m_wf.Format.cbSize > 0); }
	//retrieves the WAVEFORMATEX structure
	const WAVEFORMATEX* wf() const { return &m_wf.Format; }
	//retrieves the WAVEFORMATEXTENSIBLE structure; meaningless if the wave is not WAVEFORMATEXTENSIBLE
	const WAVEFORMATEXTENSIBLE* wfex() const { return &m_wf; }
	//gets the offset from the beginning of the file to the actual wave data
	DWORD getDataOffset() const { return m_dataOffset; }
	//gets the length of the wave data
	DWORD getDataLength() const { return m_dataLength; }
};


#endif