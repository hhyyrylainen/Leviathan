#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RESOURCEREFRESHHANDLER
#include "ResourceRefreshHandler.h"
#endif
#include "Common/StringOperations.h"
#include "Common/Misc.h"
#ifdef __GNUC__
#include <sys/types.h>
#include <sys/inotify.h>
#endif
using namespace Leviathan;
// ------------------------------------ //
#ifdef __GNUC__

#define IN_EVENT_SIZE (sizeof(inotify_event))
#define IN_READ_BUFFER_SIZE (124*(IN_EVENT_SIZE + 16))


#endif

DLLEXPORT Leviathan::ResourceRefreshHandler::ResourceRefreshHandler(){

}

DLLEXPORT Leviathan::ResourceRefreshHandler::~ResourceRefreshHandler(){
	assert(!Inited && "ResourceRefreshHandler should have been released before destructor");
}

DLLEXPORT ResourceRefreshHandler* Leviathan::ResourceRefreshHandler::Get(){
	return Staticaccess;
}

ResourceRefreshHandler* Leviathan::ResourceRefreshHandler::Staticaccess = NULL;
// ------------------------------------ //
DLLEXPORT bool Leviathan::ResourceRefreshHandler::Init(){
	// Set the next update time //
	NextUpdateTime = Misc::GetThreadSafeSteadyTimePoint()+MillisecondDuration(1000);


	Inited = true;

	Staticaccess = this;
	return true;
}

DLLEXPORT void Leviathan::ResourceRefreshHandler::Release(){
	GUARD_LOCK_THIS_OBJECT();

	Staticaccess = NULL;

	// Release all listeners //
	auto end = ActiveFileListeners.end();
	for(auto iter = ActiveFileListeners.begin(); iter != end; ++iter){

		(*iter)->StopThread();
	}
	ActiveFileListeners.clear();

	Inited = false;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ResourceRefreshHandler::ListenForFileChanges(const std::vector<const wstring*> &filestowatch, 
	boost::function<void (const wstring &, ResourceFolderListener&)> notifyfunction, int &createdid)
{

	unique_ptr<ResourceFolderListener> tmpcreated(new ResourceFolderListener(filestowatch, notifyfunction));

	createdid = tmpcreated->GetID();

	if(!tmpcreated->StartListening())
		return false;

	GUARD_LOCK_THIS_OBJECT();

	// Add it //
	ActiveFileListeners.push_back(move(tmpcreated));

	return true;
}

DLLEXPORT void Leviathan::ResourceRefreshHandler::StopListeningForFileChanges(int idoflistener){

	GUARD_LOCK_THIS_OBJECT();

	// Find the specific listener //
	auto end = ActiveFileListeners.end();
	for(auto iter = ActiveFileListeners.begin(); iter != end; ++iter){

		if((*iter)->GetID() == idoflistener){

			(*iter)->StopThread();
			ActiveFileListeners.erase(iter);
			return;
		}
	}
}

DLLEXPORT void Leviathan::ResourceRefreshHandler::CheckFileStatus(){
	GUARD_LOCK_THIS_OBJECT();

	if(Misc::GetThreadSafeSteadyTimePoint() > NextUpdateTime){
		// Update all the file listeners //
		for(size_t i = 0; i < ActiveFileListeners.size(); i++){

			ActiveFileListeners[i]->CheckUpdatesEnded();
		}

		// Set new update time //
		NextUpdateTime = Misc::GetThreadSafeSteadyTimePoint()+MillisecondDuration(1000);
	}
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ResourceRefreshHandler::MarkListenersAsNotUpdated(const std::vector<int> &ids){

	GUARD_LOCK_THIS_OBJECT();

	// Find any listeners matching any of the ids //
	auto end = ActiveFileListeners.end();
	for(auto iter = ActiveFileListeners.begin(); iter != end; ++iter){

		const int& curid = (*iter)->GetID();

		// Check against all the ids //
		auto end2 = ids.end();
		for(auto iter2 = ids.begin(); iter2 != end2; ++iter2){ 
			if(curid == *iter2){

				(*iter)->MarkAllAsNotUpdated();
				break;
			}
		}
	}
}
// ------------------ ResourceFolderListener ------------------ //
Leviathan::ResourceFolderListener::ResourceFolderListener(const std::vector<const wstring*> &filestowatch, 
	boost::function<void (const wstring &, ResourceFolderListener&)> notifyfunction) : CallbackFunction(notifyfunction), 
	ListenedFiles(filestowatch.size()), ShouldQuit(false), ID(IDFactory::GetID()), 
	UpdatedFiles(filestowatch.size(), false)
#ifdef _WIN32
	, OurReadBuffer(NULL), OverlappedInfo(NULL)

#else
	, InotifyID(-1), InotifyWatches(-1), ReadBuffer(NULL)

#endif //_WIN32


{
#ifdef _WIN32
	// Avoid having to re-allocate the vector later //
	SignalingHandles.reserve(1+1);
	
#else
	
	InotifyID = inotify_init();
	
	if(InotifyID < -1){
		
		Logger::Get()->Error(L"ResourceRefreshHandler: ResourceFolderListener: failed to create inotify instance");
		return;
	}
	
	
#endif //_WIN32

	// Copy the target files //
	for(size_t i = 0; i < ListenedFiles.size(); i++){

		// Get the folder on the first loop //
		if(i == 0){

			
			TargetFolder = StringOperations::GetPathWstring(*filestowatch[i]);
		}

		ListenedFiles[i] = move(unique_ptr<wstring>(new wstring(StringOperations::RemovePathWstring(*filestowatch[i]))));
	}

}

Leviathan::ResourceFolderListener::~ResourceFolderListener(){
	assert(ShouldQuit && "ResourceFolderListener should have been stopped before destructor");
}
// ------------------------------------ //
int Leviathan::ResourceFolderListener::GetID() const{
	return ID;
}
// ------------------------------------ //
bool Leviathan::ResourceFolderListener::StartListening(){

#ifdef _WIN32
	// First create the stop signaler //
	HANDLE ourstopper = CreateEvent(NULL, FALSE, FALSE, NULL);

	if(!ourstopper){
		
		Logger::Get()->Error(L"ResourceFolderListener: StartListening: failed to create stop notify handle, CreateEvent failed", GetLastError());
		return false;
	}


	SignalingHandles.push_back(ourstopper);

	// Now the folder listener //
	TargetFolderHandle = CreateFileW(TargetFolder.c_str(), FILE_READ_DATA | FILE_TRAVERSE | FILE_READ_EA,
		FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);

	if(TargetFolderHandle == INVALID_HANDLE_VALUE || !TargetFolderHandle){

		Logger::Get()->Error(L"ResourceFolderListener: StartListening: failed to open folder for reading, error: "
			+Convert::ToHexadecimalWstring(GetLastError()));
		return false;
	}


	// Create the OVERLAPPED struct next //
	OverlappedInfo = new OVERLAPPED;

	ZeroMemory(OverlappedInfo, sizeof(OVERLAPPED));
	
	// Create an event for notification //
	HANDLE readcompleteevent = CreateEvent(NULL, FALSE, FALSE, NULL);

	if(!readcompleteevent){

		Logger::Get()->Error(L"ResourceFolderListener: StartListening: failed to create read notify handle, CreateEvent failed, error: "
			+Convert::ToHexadecimalWstring(GetLastError()));
		return false;
	} 

	// Add it to the overlapped //
	OverlappedInfo->hEvent = readcompleteevent;


	
	OurReadBuffer = new FILE_NOTIFY_INFORMATION[100];


	// Create the update notification read thing //
	BOOL createresult = ReadDirectoryChangesW(TargetFolderHandle, OurReadBuffer, sizeof(FILE_NOTIFY_INFORMATION)*100, 
		// Only the top level directory is watched
		FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE, NULL, OverlappedInfo, NULL);

	if(!createresult){

		CloseHandle(TargetFolderHandle);
		Logger::Get()->Error(L"ResourceFolderListener: StartListening: failed to start reading directory changes, error: "
			+Convert::ToHexadecimalWstring(GetLastError()));
		return false;
	}

	SignalingHandles.push_back(readcompleteevent);
	
#else
	
	// Create watches for all of them //
	
	// Apparently we need to use utf8 here //
	string toutf8 = Convert::Utf16ToUtf8(TargetFolder);
	
	
	InotifyWatches = inotify_add_watch(InotifyID, toutf8.c_str(), IN_MODIFY);
	
	
	if(InotifyWatches < 0){
		
		
		Logger::Get()->Error(L"ResourceRefreshHandler: ResourceFolderListener: failed to add watch for folder: "+TargetFolder);
		return false;
	}
	
		
	// Allocate the read buffer //
	ReadBuffer = new char[IN_READ_BUFFER_SIZE];
	
	
	
#endif //_WIN32
	

	ShouldQuit = false;

	// Finally start the thread //
	ListenerThread = boost::thread(boost::bind(&ResourceFolderListener::_RunListeningThread, this));

	return true;
}

void Leviathan::ResourceFolderListener::StopThread(){
#ifdef _WIN32
	// Don't do anything if already done //
	if(ShouldQuit && SignalingHandles.empty())
		return;

	ShouldQuit = true;

	// Signal the close handle //
	SetEvent(SignalingHandles[0]);

	// Join the thread to wait for it to quit //
	ListenerThread.join();

	// Close the handles //
	CloseHandle(SignalingHandles[0]);
	CloseHandle(SignalingHandles[1]);
	CloseHandle(TargetFolderHandle);

	SignalingHandles.clear();

	if(OurReadBuffer){
		delete[] OurReadBuffer;
		OurReadBuffer = NULL;
	}

	SAFE_DELETE(OverlappedInfo);
#else
	
	if(ShouldQuit && InotifyID == -1)
		return;
	
	ShouldQuit = true;
	
	inotify_rm_watch(InotifyID, InotifyWatches);
	close(InotifyID);
	
	SAFE_DELETE(ReadBuffer);
	
	InotifyID = -1;
	InotifyWatches = -1;
	
#endif //_WIN32
}
// ------------------------------------ //
void Leviathan::ResourceFolderListener::_RunListeningThread(){
	// Run until quit is requested //
	while(!ShouldQuit){

#ifdef _WIN32
		
		// Wait for the handles //
		DWORD waitstatus = WaitForMultipleObjects(SignalingHandles.size(), &SignalingHandles[0], FALSE, INFINITE);

		// Check what happened //
		switch(waitstatus){ 
		case WAIT_OBJECT_0:
			
			// Quit has been called //
			return;

		case WAIT_OBJECT_0 + 1:
			{
				// A modification has been detected //

				// Check what is detected //
				FILE_NOTIFY_INFORMATION* dataptr = OurReadBuffer;

				DWORD numread;

				GetOverlappedResult(TargetFolderHandle, OverlappedInfo, &numread, FALSE);

				if(numread < 1){
					// Nothing read/failed //
					Logger::Get()->Error(L"ResourceFolderListener: _RunListeningThread: result has 0 bytes: ");
					continue;
				}

				bool working = true;

				while(working){
					// Check what the notification is //
					if(dataptr->Action == FILE_ACTION_REMOVED || dataptr->Action == FILE_ACTION_RENAMED_OLD_NAME){

						goto movetonextdatalabel;
					}

					{
						// Get the filename //
						wstring entrydata;

						size_t filenameinwchars = dataptr->FileNameLength/sizeof(wchar_t);

						entrydata.resize(filenameinwchars);

						// Copy the data //
						memcpy_s(&entrydata[0], entrydata.size()*sizeof(wchar_t), dataptr->FileName, dataptr->FileNameLength);

						// Skip if nothing //
						if(entrydata.empty()){

							goto movetonextdatalabel;
						}


						// Check which file matches and set it as updated //
						for(size_t i = 0; i < ListenedFiles.size(); i++){

							if(*ListenedFiles[i] == entrydata){

								// Updated //
								UpdatedFiles[i] = true;
								break;
							}
						}
					}
movetonextdatalabel:

					if(!dataptr->NextEntryOffset){

						// No more entries
						working = false;
						break;
					} else {
						// Move to next entry //

						char* tmpptr = reinterpret_cast<char*>(dataptr);
						tmpptr += dataptr->NextEntryOffset;

						dataptr = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(tmpptr);
					}
				}

				// Start listening again //
				BOOL createresult = ReadDirectoryChangesW(TargetFolderHandle, OurReadBuffer, sizeof(FILE_NOTIFY_INFORMATION)*100, 
					// Only the top level directory is watched
					FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE, NULL, OverlappedInfo, NULL);

				if(!createresult){

					Logger::Get()->Error(L"ResourceFolderListener: _RunListeningThread: re-queuing file change read failed: "
						+Convert::ToHexadecimalWstring(GetLastError()));
					return;
				}

			}
			break; 

		case WAIT_TIMEOUT:
			// NO such thing, continue!
			continue;

		default:
			Logger::Get()->Error(L"ResourceFolderListener: _RunListeningThread: invalid wait result: "+Convert::ToWstring(waitstatus));
			break;
		}
		
		
			
#else
		
		// Read the changes until the end of time //
		int readcount = read(InotifyID, ReadBuffer, IN_READ_BUFFER_SIZE);
		
		if(readcount < 0){
			
			// Failed reading, quit //
			Logger::Get()->Warning(L"ResourceFolderListener: read failed, quitting thread");
			return;
		}
		
		// Handle all the data //
		for(int i = 0; i < readcount; ){
			
			inotify_event* event = reinterpret_cast<inotify_event*>(&ReadBuffer[i]);
			
			if(event->len){
				
				if(event->mask & IN_MODIFY){
					if(!(event->mask & IN_ISDIR)){
						
						// Some file was modified, check was it one of ours //
						const string modifiedfile(event->name, event->len);
						
						const wstring entrydata = Convert::Utf8ToUtf16(modifiedfile);
						
						if(!entrydata.empty()){
							
							// Check which file matches and set it as updated //
							for(size_t i = 0; i < ListenedFiles.size(); i++){

								if(*ListenedFiles[i] == entrydata){

									// Updated //
									UpdatedFiles[i] = true;
									break;
								}
							}
						}
					}
				}
			}
			
			i += IN_EVENT_SIZE+event->len;
		}
		
		
		
#endif //_Win32
	}
}
// ------------------------------------ //
void Leviathan::ResourceFolderListener::CheckUpdatesEnded(){
	// Check are some updated files readable //
	for(size_t i = 0; i < UpdatedFiles.size(); i++){

		if(UpdatedFiles[i]){

			// Check is it readable //
			const wstring checkread = TargetFolder+*ListenedFiles[i];

			string realtarget = Convert::Utf16ToUtf8(checkread);

			ifstream reader(realtarget);

			if(reader.is_open()){

				reader.close();

				// Notify that the file is now available //
				CallbackFunction(*ListenedFiles[i], *this);
				UpdatedFiles[i] = false;
			}
		}
	}
}
// ------------------------------------ //
void Leviathan::ResourceFolderListener::MarkAllAsNotUpdated(){
	auto end = UpdatedFiles.end();
	for(auto iter = UpdatedFiles.begin(); iter != end; ++iter){

		(*iter) = false;
	}
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ResourceFolderListener::IsAFileStillUpdated() const{
	// Try to find a set bool //
	auto end = UpdatedFiles.end();
	for(auto iter = UpdatedFiles.begin(); iter != end; ++iter){

		if((*iter)){

			return true;
		}
	}

	// No file is marked as updated //
	return false;
}
