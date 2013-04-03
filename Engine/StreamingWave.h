#ifndef LEVIATHAN_SOUND_STREAMINGWAVE
#define LEVIATHAN_SOUND_STREAMINGWAVE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Logger.h"
#include "FileSystem.h"

#include <XAudio2.h>
#include <XAudio2fx.h>

#include "WaveInfo.h"

// code loaned from //
//by Jay Tennant 3/8/12
//A Brief Look at XAudio2: Playing a Stream
//demonstrates streaming a wave from disk
//win32developer.com
//this code provided free, as in public domain; score!


//should remain a power of 2; should also stay 4096 or larger, just to guarantee a multiple of the disk sector size (most are at or below 4096)
#define STREAMINGWAVE_BUFFER_SIZE 65536
//should never be less than 3
#define STREAMINGWAVE_BUFFER_COUNT 3

class StreamingWave : public WaveInfo
{
private:
	HANDLE m_hFile; //the file being streamed
	DWORD m_currentReadPass; //the current pass for reading; this number multiplied by STREAMINGWAVE_BUFFER_SIZE, adding getDataOffset(), represents the file position
	DWORD m_currentReadBuffer; //the current buffer used for reading from file; the presentation buffer is the one right before this
	bool m_isPrepared; //whether the buffer is prepared for the swap
	BYTE *m_dataBuffer; //the wave buffers; the size is STREAMINGWAVE_BUFFER_COUNT * STREAMINGWAVE_BUFFER_SIZE + m_sectorAlignment
	XAUDIO2_BUFFER m_xaBuffer[STREAMINGWAVE_BUFFER_COUNT]; //the xaudio2 buffer information
	DWORD m_sectorAlignment; //the sector alignment for reading; this value is added to the entire buffer's size for sector-aligned reading and reference
	DWORD m_bufferBeginOffset; //the starting offset for each buffer (when the file reads are offset by an amount)
public:
	StreamingWave( LPCTSTR szFile = NULL ) : WaveInfo( NULL ), m_hFile(INVALID_HANDLE_VALUE), m_currentReadPass(0), m_currentReadBuffer(0), m_isPrepared(false), 
		m_dataBuffer(NULL), m_sectorAlignment(0), m_bufferBeginOffset(0) {
			memset( m_xaBuffer, 0, sizeof(m_xaBuffer) );

			//figure the sector alignment
			DWORD dw1, dw2, dw3;
			GetDiskFreeSpace( NULL, &dw1, &m_sectorAlignment, &dw2, &dw3 );

			//allocate the buffers
			m_dataBuffer = (BYTE*)_aligned_malloc( STREAMINGWAVE_BUFFER_COUNT * STREAMINGWAVE_BUFFER_SIZE + m_sectorAlignment, m_sectorAlignment );
			memset( m_dataBuffer, 0, STREAMINGWAVE_BUFFER_COUNT * STREAMINGWAVE_BUFFER_SIZE + m_sectorAlignment );

			load(szFile);
	}
	StreamingWave( const StreamingWave& c ) : WaveInfo(c), m_hFile(c.m_hFile), m_currentReadPass(c.m_currentReadPass), m_currentReadBuffer(c.m_currentReadBuffer),
		m_isPrepared(c.m_isPrepared), m_dataBuffer(NULL), m_sectorAlignment(c.m_sectorAlignment), m_bufferBeginOffset(c.m_bufferBeginOffset) {
			if( m_sectorAlignment == 0 )
			{
				//figure the sector alignment
				DWORD dw1, dw2, dw3;
				GetDiskFreeSpace( NULL, &dw1, &m_sectorAlignment, &dw2, &dw3 );
			}

			//allocate the buffers
			m_dataBuffer = (BYTE*)_aligned_malloc( STREAMINGWAVE_BUFFER_COUNT * STREAMINGWAVE_BUFFER_SIZE + m_sectorAlignment, m_sectorAlignment );
			memset( m_dataBuffer, 0, STREAMINGWAVE_BUFFER_COUNT * STREAMINGWAVE_BUFFER_SIZE + m_sectorAlignment );

			memcpy( m_dataBuffer, c.m_dataBuffer, STREAMINGWAVE_BUFFER_COUNT * STREAMINGWAVE_BUFFER_SIZE + m_sectorAlignment );
			memcpy( m_xaBuffer, c.m_xaBuffer, sizeof(m_xaBuffer) );
			for( int i = 0; i < STREAMINGWAVE_BUFFER_COUNT; i++ )
				m_xaBuffer[i].pAudioData = m_dataBuffer + m_bufferBeginOffset + i * STREAMINGWAVE_BUFFER_SIZE;
	}
	~StreamingWave() {
		close();

		if( m_dataBuffer != NULL )
			_aligned_free( m_dataBuffer );
		m_dataBuffer = NULL;
	}

	//loads the file for streaming wave data
	bool load(LPCTSTR szFile){
		close();

		//test if the data can be loaded
		if(!WaveInfo::load(szFile)){

			//Logger::Get()->Error((L"Undocumented error: StreamingWave.h line: "+__LINE__), true);
			return false;
		}

		//figure the offset for the wave data in allocated memory
		m_bufferBeginOffset = getDataOffset() % m_sectorAlignment;

		//open the file
		m_hFile = CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);
		if(m_hFile == INVALID_HANDLE_VALUE){

			//Logger::Get()->Error((L"Undocumented error: StreamingWave.h line: "+__LINE__), true);
			return false;
		}

		//set the xaudio2 buffer struct to refer to appropriate buffer starting points (but leave size of the data as 0)
		for( int i = 0; i < STREAMINGWAVE_BUFFER_COUNT; i++ )
			m_xaBuffer[i].pAudioData = m_dataBuffer + m_bufferBeginOffset + i * STREAMINGWAVE_BUFFER_SIZE;

		return true;
	}

	//closes the file stream, resetting this object's state
	void close(){
		if( m_hFile != INVALID_HANDLE_VALUE )
			CloseHandle( m_hFile );
		m_hFile = INVALID_HANDLE_VALUE;

		m_bufferBeginOffset = 0;
		memset( m_xaBuffer, 0, sizeof(m_xaBuffer) );
		memset( m_dataBuffer, 0, STREAMINGWAVE_BUFFER_COUNT * STREAMINGWAVE_BUFFER_SIZE + m_sectorAlignment );
		m_isPrepared = false;
		m_currentReadBuffer = 0;
		m_currentReadPass = 0;

		WaveInfo::load(NULL);
	}

	//swaps the presentation buffer to the next one
	void swap() {m_currentReadBuffer = (m_currentReadBuffer + 1) % STREAMINGWAVE_BUFFER_COUNT; m_isPrepared = false;}

	//gets the current buffer
	const XAUDIO2_BUFFER* buffer() const {return &m_xaBuffer[ (m_currentReadBuffer + STREAMINGWAVE_BUFFER_COUNT - 1) % STREAMINGWAVE_BUFFER_COUNT ];}

	//resets the file pointer to the beginning of the wave data;
	//this will not wipe out buffers that have been prepared, so it is safe to call
	//after a call to prepare() has returned PR_EOF, and before a call to swap() has
	//been made to present the prepared buffer
	void resetFile(){m_currentReadPass = 0;}

	enum PREPARE_RESULT {
		PR_SUCCESS = 0,
		PR_FAILURE = 1,
		PR_EOF = 2,
	};

	//prepares the next buffer for presentation;
	//returns PR_SUCCESS on success,
	//PR_FAILURE on failure,
	//and PR_EOF when the end of the data has been reached
	DWORD prepare(){
		//validation check
		if(m_hFile == INVALID_HANDLE_VALUE){

			//Logger::Get()->Error((L"Undocumented error: StreamingWave.h line: "+__LINE__), true);
			m_xaBuffer[ m_currentReadBuffer ].AudioBytes = 0;
			m_xaBuffer[ m_currentReadBuffer ].Flags = XAUDIO2_END_OF_STREAM;
			return PR_FAILURE;
		}

		//are we already prepared?
		if(m_isPrepared)
			return PR_SUCCESS;

		//figure the offset of the file pointer
		OVERLAPPED overlapped = {0};
		overlapped.Offset = getDataOffset() - m_bufferBeginOffset + STREAMINGWAVE_BUFFER_SIZE * m_currentReadPass;

		//preliminary end-of-data check
		if(overlapped.Offset + m_bufferBeginOffset > getDataLength() + getDataOffset()){

			//Logger::Get()->Error((L"Undocumented error: StreamingWave.h line: "+__LINE__), true);
			m_xaBuffer[ m_currentReadBuffer ].AudioBytes = 0;
			m_xaBuffer[ m_currentReadBuffer ].Flags = XAUDIO2_END_OF_STREAM;
			m_isPrepared = true;
			return PR_EOF;
		}

		//read in data from file
		DWORD dwNumBytesRead = 0;
		if(FALSE == ReadFile( m_hFile, m_dataBuffer + STREAMINGWAVE_BUFFER_SIZE * m_currentReadBuffer, STREAMINGWAVE_BUFFER_SIZE + m_sectorAlignment, &dwNumBytesRead, &overlapped)){

			//Logger::Get()->Error((L"Undocumented error: StreamingWave.h line: "+__LINE__), true);
			m_xaBuffer[ m_currentReadBuffer ].AudioBytes = 0;
			m_xaBuffer[ m_currentReadBuffer ].Flags = XAUDIO2_END_OF_STREAM;
			return PR_FAILURE;
		}

		//force dwNumBytesRead to be less than the actual amount read if reading past the end of the data chunk
		if(dwNumBytesRead + STREAMINGWAVE_BUFFER_SIZE * m_currentReadPass > getDataLength()){

			if(STREAMINGWAVE_BUFFER_SIZE * m_currentReadPass <= getDataLength())
				dwNumBytesRead = min( dwNumBytesRead, getDataLength() - STREAMINGWAVE_BUFFER_SIZE * m_currentReadPass); //bytes read are from overlapping file chunks
			else
				dwNumBytesRead = 0; //none of the bytes are from the correct data chunk; this should never happen due to the preliminary end-of-data check, unless the file was wrong
		}

		//end-of-file/data check
		if(dwNumBytesRead < STREAMINGWAVE_BUFFER_SIZE + m_sectorAlignment){

			//check for case where less than the sectorAlignment amount of data is still available in the file;
			//of course, only do something if there isn't that amount of data left
			if( dwNumBytesRead < m_bufferBeginOffset ){//no valid data at all; this shouldn't happen since the preliminary end-of-data check happened already, unless the file was wrong
				m_xaBuffer[ m_currentReadBuffer ].AudioBytes = 0;
				m_xaBuffer[ m_currentReadBuffer ].Flags = XAUDIO2_END_OF_STREAM;
				m_isPrepared = true;

				//increment the current read pass
				m_currentReadPass++;
				return PR_EOF;
			} else if( dwNumBytesRead - m_bufferBeginOffset <= STREAMINGWAVE_BUFFER_SIZE ){//some valid data; this should always happen for the end-of-file and end-of-data conditions
				m_xaBuffer[ m_currentReadBuffer ].AudioBytes = dwNumBytesRead - m_bufferBeginOffset; //do not include the data offset as valid data
				m_xaBuffer[ m_currentReadBuffer ].Flags = XAUDIO2_END_OF_STREAM;
				m_isPrepared = true;

				//increment the current read pass
				m_currentReadPass++;
				return PR_EOF;
			}
		}

		//set the amount of data available;
		//this should always be STREAMINGWAVE_BUFFER_SIZE, unless one of the previous conditions (end-of-file, end-of-data) were met
		m_xaBuffer[ m_currentReadBuffer ].AudioBytes = STREAMINGWAVE_BUFFER_SIZE;
		m_xaBuffer[ m_currentReadBuffer ].Flags = 0;
		m_isPrepared = true;

		//increment the current read pass
		m_currentReadPass++;

		//return success
		return PR_SUCCESS;
	}
};

#endif