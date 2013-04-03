#ifndef LEVIATHAN_SOUND_STREAMER
#define LEVIATHAN_SOUND_STREAMER
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


#include "StreamingWave.h"
// forward declaration //

// code loaned from //
//by Jay Tennant 3/8/12
//A Brief Look at XAudio2: Playing a Stream
//demonstrates streaming a wave from disk
//win32developer.com
//this code provided free, as in public domain; score!

//the context to send to the StreamProc
struct StreamContext
{
public:
	StreamContext(IXAudio2SourceVoice* voice, wstring file, HANDLE loadevent, HANDLE notifyevent){
		pVoice = voice;
		szFile = file;
		hVoiceLoadEvent = loadevent;
		End = false;
		ID = -1;
		Volume = 100;
		Repeatcount = 1;
		ended = false;
		ThreadID = 0;
		hstreamingVoiceThread = NULL;

		UpDateEvent = notifyevent;
	};
	StreamContext(){
		pVoice = NULL;
		szFile = L"";
		hVoiceLoadEvent = NULL;
		End = false;
		ID = -1;
		Volume = 100;
		Repeatcount = 1;
		ended = true;
		ThreadID = 0;
		hstreamingVoiceThread = NULL;
	};
	IXAudio2SourceVoice* pVoice; //the source voice that is created on the thread
	wstring szFile; //name of the file to stream
	HANDLE hVoiceLoadEvent; //lets us know the thread is set up for streaming, or encountered an error
	HANDLE UpDateEvent; // thread needs to update
	int ID;
	int Volume;
	int Repeatcount;
	bool End;

	DWORD ThreadID;
	HANDLE hstreamingVoiceThread;

	bool ended;
};

struct StreamingVoiceCallback : public IXAudio2VoiceCallback
{
public:
	HANDLE m_hBufferEndEvent;

	StreamingVoiceCallback() : m_hBufferEndEvent( CreateEvent( NULL, TRUE, FALSE, NULL ) ) {}
	virtual ~StreamingVoiceCallback() { CloseHandle( m_hBufferEndEvent ); }

#pragma warning( disable: 4100 )
	//overrides
	STDMETHOD_( void, OnVoiceProcessingPassStart )( UINT32 bytesRequired )
	{
	}
	STDMETHOD_( void, OnVoiceProcessingPassEnd )()
	{
	}
	STDMETHOD_( void, OnStreamEnd )()
	{
	}
	STDMETHOD_( void, OnBufferStart )( void* pContext )
	{
	}
	STDMETHOD_( void, OnBufferEnd )( void* pContext )
	{
		SetEvent( m_hBufferEndEvent );
	}
	STDMETHOD_( void, OnLoopEnd )( void* pContext )
	{
	}
	STDMETHOD_( void, OnVoiceError )( void* pContext, HRESULT error )
	{
	}
};
DLLEXPORT DWORD WINAPI StreamProc(LPVOID pContext);











#endif