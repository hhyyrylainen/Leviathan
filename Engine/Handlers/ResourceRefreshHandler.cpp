// ------------------------------------ //
#include "ResourceRefreshHandler.h"

#include "Common/StringOperations.h"
#include "../TimeIncludes.h"
#include "IDFactory.h"
#ifdef __linux__
#include <sys/types.h>
#include <sys/inotify.h>
#endif
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
#ifdef __linux__

#define IN_EVENT_SIZE (sizeof(inotify_event))
#define IN_READ_BUFFER_SIZE (124*(IN_EVENT_SIZE + 16))


#endif

DLLEXPORT Leviathan::ResourceRefreshHandler::ResourceRefreshHandler(){

}

DLLEXPORT Leviathan::ResourceRefreshHandler::~ResourceRefreshHandler(){
	LEVIATHAN_ASSERT(Staticaccess != this,
        "ResourceRefreshHandler should have been released before destructor");
}

DLLEXPORT ResourceRefreshHandler* Leviathan::ResourceRefreshHandler::Get(){
	return Staticaccess;
}

ResourceRefreshHandler* Leviathan::ResourceRefreshHandler::Staticaccess = NULL;
// ------------------------------------ //
DLLEXPORT bool Leviathan::ResourceRefreshHandler::Init(){
	// Set the next update time //
	NextUpdateTime = Time::GetThreadSafeSteadyTimePoint()+MillisecondDuration(1000);

	Staticaccess = this;
	return true;
}

DLLEXPORT void Leviathan::ResourceRefreshHandler::Release(){
	GUARD_LOCK();

	Staticaccess = NULL;

	// Release all listeners //
	auto end = ActiveFileListeners.end();
	for(auto iter = ActiveFileListeners.begin(); iter != end; ++iter){

		(*iter)->StopThread();
	}

    
	ActiveFileListeners.clear();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ResourceRefreshHandler::ListenForFileChanges(
    const std::vector<const std::string*> &filestowatch, 
	std::function<void (const std::string &, ResourceFolderListener&)> notifyfunction,
    int &createdid)
{

	unique_ptr<ResourceFolderListener> tmpcreated(new ResourceFolderListener(filestowatch,
            notifyfunction));

	createdid = tmpcreated->GetID();

	if(!tmpcreated->StartListening())
		return false;

	GUARD_LOCK();

	// Add it //
	ActiveFileListeners.push_back(move(tmpcreated));

	return true;
}

DLLEXPORT void Leviathan::ResourceRefreshHandler::StopListeningForFileChanges(
    int idoflistener)
{

	GUARD_LOCK();

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
	GUARD_LOCK();

	if(Time::GetThreadSafeSteadyTimePoint() > NextUpdateTime){
		// Update all the file listeners //
		for(size_t i = 0; i < ActiveFileListeners.size(); i++){

			ActiveFileListeners[i]->CheckUpdatesEnded();
		}

		// Set new update time //
		NextUpdateTime = Time::GetThreadSafeSteadyTimePoint()+MillisecondDuration(1000);
	}
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ResourceRefreshHandler::MarkListenersAsNotUpdated(
    const std::vector<int> &ids)
{

	GUARD_LOCK();

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
Leviathan::ResourceFolderListener::ResourceFolderListener(
    const std::vector<const std::string*> &filestowatch, 
	std::function<void (const std::string &, ResourceFolderListener&)> notifyfunction) :
    ListenedFiles(filestowatch.size()), 
	UpdatedFiles(filestowatch.size(), false),
    ID(IDFactory::GetID()), CallbackFunction(notifyfunction)
{
#ifdef _WIN32
	// Avoid having to re-allocate the vector later //
	SignalingHandles.reserve(1+1);
	
#else
	
	InotifyID = inotify_init();
	
	if(InotifyID < -1){
		
		Logger::Get()->Error("ResourceRefreshHandler: ResourceFolderListener: "
            "failed to create inotify instance");
		return;
	}
	
#endif //_WIN32

	// Copy the target files //
	for(size_t i = 0; i < ListenedFiles.size(); i++){

		// Get the folder on the first loop //
		if(i == 0){

			
			TargetFolder = StringOperations::GetPath<std::string>(*filestowatch[i]);
		}

		ListenedFiles[i] = make_unique<std::string>(
            StringOperations::RemovePath<std::string>(*filestowatch[i]));
	}

}

Leviathan::ResourceFolderListener::~ResourceFolderListener(){
	LEVIATHAN_ASSERT(ShouldQuit,
        "ResourceFolderListener should have been stopped before destructor");
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
		
		Logger::Get()->Error("ResourceFolderListener: StartListening: failed to create stop "
            "notify handle, CreateEvent failed");
		return false;
	}


	SignalingHandles.push_back(ourstopper);

	// Now the folder listener //
	TargetFolderHandle = CreateFileA(TargetFolder.c_str(), FILE_READ_DATA | FILE_TRAVERSE |
        FILE_READ_EA, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);

	if(TargetFolderHandle == INVALID_HANDLE_VALUE || !TargetFolderHandle){

		Logger::Get()->Error("ResourceFolderListener: StartListening: failed to open folder for "
            "reading, error: "+Convert::ToHexadecimalString(GetLastError()));
		return false;
	}


	// Create the OVERLAPPED struct next //
	OverlappedInfo = new OVERLAPPED;

	ZeroMemory(OverlappedInfo, sizeof(OVERLAPPED));
	
	// Create an event for notification //
	HANDLE readcompleteevent = CreateEvent(NULL, FALSE, FALSE, NULL);

	if(!readcompleteevent){

		Logger::Get()->Error("ResourceFolderListener: StartListening: "
            "failed to create read notify handle, CreateEvent failed, error: " +
            Convert::ToHexadecimalString(GetLastError()));
		return false;
	} 

	// Add it to the overlapped //
	OverlappedInfo->hEvent = readcompleteevent;


	
	OurReadBuffer = new FILE_NOTIFY_INFORMATION[100];


	// Create the update notification read thing //
	BOOL createresult = ReadDirectoryChangesW(TargetFolderHandle, OurReadBuffer,
        sizeof(FILE_NOTIFY_INFORMATION)*100, 
		// Only the top level directory is watched
		FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE, NULL, OverlappedInfo, NULL);

	if(!createresult){

		CloseHandle(TargetFolderHandle);
		Logger::Get()->Error("ResourceFolderListener: StartListening: failed to start reading "
            "directory changes, error: "+Convert::ToHexadecimalString(GetLastError()));
		return false;
	}

	SignalingHandles.push_back(readcompleteevent);
	
#else
	
	// Create watches for all of them //
	
	InotifyWatches = inotify_add_watch(InotifyID, TargetFolder.c_str(), IN_MODIFY);
	
	
	if(InotifyWatches < 0){
		
		
		Logger::Get()->Error("ResourceRefreshHandler: ResourceFolderListener: failed to add "
            "watch for folder: "+TargetFolder);
		return false;
	}
	
		
	// Allocate the read buffer //
	ReadBuffer = new char[IN_READ_BUFFER_SIZE];
	
#endif //_WIN32
	

	ShouldQuit = false;

	// Finally start the thread //
	ListenerThread = std::thread(std::bind(&ResourceFolderListener::_RunListeningThread,
            this));

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
	
	ListenerThread.join();

	if(ReadBuffer){
		delete[] ReadBuffer;
		ReadBuffer = NULL;
	}
	
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
		DWORD waitstatus = WaitForMultipleObjects(static_cast<DWORD>(SignalingHandles.size()), &SignalingHandles[0],
            FALSE, INFINITE);

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
					Logger::Get()->Error("ResourceFolderListener: _RunListeningThread: result "
                        "has 0 bytes: ");
					continue;
				}

				bool working = true;

				while(working){
					// Check what the notification is //
					if(dataptr->Action == FILE_ACTION_REMOVED ||
                        dataptr->Action == FILE_ACTION_RENAMED_OLD_NAME){

						goto movetonextdatalabel;
					}

					{
						// Get the filename //
						std::wstring entrydata;

						size_t filenameinwchars = dataptr->FileNameLength/sizeof(wchar_t);

						entrydata.resize(filenameinwchars);

						// Copy the data //
						memcpy_s(&entrydata[0], entrydata.size()*sizeof(wchar_t),
                            dataptr->FileName, dataptr->FileNameLength);

                        std::string utf8file = Convert::Utf16ToUtf8(entrydata);
                        
						// Skip if nothing //
						if(utf8file.empty()){

							goto movetonextdatalabel;
						}


						// Check which file matches and set it as updated //
						for(size_t i = 0; i < ListenedFiles.size(); i++){

							if(*ListenedFiles[i] == utf8file){

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
				BOOL createresult = ReadDirectoryChangesW(TargetFolderHandle, OurReadBuffer,
                    sizeof(FILE_NOTIFY_INFORMATION)*100, 
					// Only the top level directory is watched
					FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE, NULL, OverlappedInfo, NULL);

				if(!createresult){

					Logger::Get()->Error("ResourceFolderListener: _RunListeningThread: "
                        "re-queuing file change read failed: "+
                        Convert::ToHexadecimalString(GetLastError()));
					return;
				}

			}
			break; 

		case WAIT_TIMEOUT:
			// NO such thing, continue!
			continue;

		default:
			Logger::Get()->Error("ResourceFolderListener: _RunListeningThread: invalid wait "
                "result: "+Convert::ToString(waitstatus));
			break;
		}
			
#else
		
		// Read the changes until the end of time //
		int readcount = read(InotifyID, ReadBuffer, IN_READ_BUFFER_SIZE);
		
		if(readcount < 0){
			
			// Failed reading, quit //
			Logger::Get()->Warning("ResourceFolderListener: read failed, quitting thread");
			return;
		}
		
		// Handle all the data //
		for(int i = 0; i < readcount; ){

			// Break if invalid buffer //
			if(!ReadBuffer)
				return;
			
			inotify_event* event = reinterpret_cast<inotify_event*>(&ReadBuffer[i]);
			
			if(event->len){
				
				if(event->mask & IN_MODIFY){
					if(!(event->mask & IN_ISDIR)){
						
						// Some file was modified, check was it one of ours //
						const string modifiedfile(event->name, event->len);
						
						if(!modifiedfile.empty()){
							
							// Check which file matches and set it as updated //
							for(size_t i = 0; i < ListenedFiles.size(); i++){

								if(*ListenedFiles[i] == modifiedfile){

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
			const std::string checkread = TargetFolder+*ListenedFiles[i];

			ifstream reader(checkread);

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
