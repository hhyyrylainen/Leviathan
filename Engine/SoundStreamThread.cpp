#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SOUND_STREAMER
#include "SoundStreamThread.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "SoundDevice.h"

DWORD WINAPI StreamProc(LPVOID pContext){
	// thread that streams audio to XAudio2 buffers //

	//required by XAudio2
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	// get pointer to interface //
	IXAudio2* Sengine = SoundDevice::GetAudio();


	StreamContext* sc = (StreamContext*)pContext;

	// instantiate voice callback class //
	StreamingVoiceCallback callback;

	// load file for streaming, non-buffered disk read (no system caching) //
	StreamingWave file;
	if(!file.load(sc->szFile.c_str())){

		SetEvent(sc->hVoiceLoadEvent);
		CoUninitialize();
		return (DWORD)-3;
	}

	// create voice //
	IXAudio2SourceVoice* source = NULL;
	if(FAILED(Sengine->CreateSourceVoice(&source, file.wf(), 0, 2.0f, &callback))){

		SetEvent( sc->hVoiceLoadEvent );
		CoUninitialize();
		return (DWORD)-5;
	}


	//fill and queue the maximum number of buffers (except the one needed for reading new wave data)
	bool somethingsWrong = false;
	XAUDIO2_VOICE_STATE voiceState = {0};
	source->GetState( &voiceState );
	while( voiceState.BuffersQueued < STREAMINGWAVE_BUFFER_COUNT - 1 && !somethingsWrong )
	{
		//read and fill the next buffer to present
		switch( file.prepare() )
		{
		case StreamingWave::PR_EOF:
			//if end-of-file (or end-of-data), loop the file read
			file.resetFile(); //intentionally fall-through to loop sound
		case StreamingWave::PR_SUCCESS:
			//present the next available buffer
			file.swap();
			//submit another buffer
			source->SubmitSourceBuffer( file.buffer() );
			source->GetState( &voiceState );
			break;
		case StreamingWave::PR_FAILURE:
			somethingsWrong = true;
			break;
		}
	}

	//return the created voice through the context pointer
	sc->pVoice = source;

	int CurrentVolume = sc->Volume;
	// set volume //
	sc->pVoice->SetVolume(((float)CurrentVolume)/100);



	//signal that the voice has prepared for streaming, and ready to start
	SetEvent( sc->hVoiceLoadEvent );

	//group the events for the Wait function
	HANDLE hEvents[3] = { callback.m_hBufferEndEvent, SoundDevice::GetAbort(), sc->UpDateEvent };

	bool quitting = false;
	while( !quitting )
	{
		//wait until either the source voice is ready for another buffer, or the abort signal is set
		DWORD eventFired = WaitForMultipleObjects( 3, hEvents, FALSE, INFINITE );
		if(sc->End){ 
			// sound stop called
			quitting = true;
			break;
		}
		//// check volume //
		if(CurrentVolume != sc->Volume){
			CurrentVolume = sc->Volume;
			// set volume //
			sc->pVoice->SetVolume(((float)CurrentVolume)/100);
		}

		// change sound //
		switch( eventFired )
		{
		case 0: //buffer ended event for source voice
			//reset the event manually; why Manually? well why not?!
			ResetEvent( hEvents[0] );

			//make sure there's a full number of buffers
			source->GetState( &voiceState );
			while( voiceState.BuffersQueued < STREAMINGWAVE_BUFFER_COUNT - 1 && !somethingsWrong )
			{
				//read and fill the next buffer to present
				switch( file.prepare() )
				{
				case StreamingWave::PR_EOF:
					{
						sc->Repeatcount--;
						if(sc->Repeatcount < 1){
							quitting = true;
							break;
						}
						//if end-of-file (or end-of-data), loop the file read
						file.resetFile(); //intentionally fall-through to loop sound
					}
				case StreamingWave::PR_SUCCESS:
					//present the next available buffer
					file.swap();
					//submit another buffer
					source->SubmitSourceBuffer( file.buffer() );
					source->GetState( &voiceState );
					break;
				case StreamingWave::PR_FAILURE:
					somethingsWrong = true;
					break;
				}
			}

			break;
		case 1: //abort event
			quitting = true;
			break;
		case 2:
			{
				// update event //
				if(sc->End){ 
					// sound stop called
					quitting = true;
					break;
				}
				//// check volume //
				if(CurrentVolume != sc->Volume){
					CurrentVolume = sc->Volume;
					// set volume //
					sc->pVoice->SetVolume(((float)CurrentVolume)/100);
				}
			}
		break;
		default: //something's wrong...
			quitting = true;
		}
	}

	//stop and destroy the voice
	source->Stop();
	source->FlushSourceBuffers();
	source->DestroyVoice();

	//close the streaming wave file;
	//this is done automatically in the class destructor,
	//so this is redundant
	file.close();

	sc->ended = true;
	//cleanup
	CoUninitialize();
	return 0;
}
