#pragma once
#ifndef LEVIATHAN_RESOURCEREFRESHHANDLER
#define LEVIATHAN_RESOURCEREFRESHHANDLER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
#include "boost\function.hpp"
#include "boost\thread.hpp"
#include "Common\ThreadSafe.h"
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{


	//! \brief A file listener instance which listens for file changes in a folder
	//! \todo Do linux file listening
	class ResourceFolderListener{
	public:
		//! \brief Creates a new listener
		//! \see ResourceRefreshHandler::ListenForFileChanges
		ResourceFolderListener(const std::vector<const wstring*> &filestowatch, 
			boost::function<void (const wstring &, ResourceFolderListener&)> notifyfunction);
		~ResourceFolderListener();

		//! \brief Gets the ID of this object
		DLLEXPORT int GetID() const;

		//! \brief Starts a listening thread
		//! \see StopThread
		bool StartListening();


		//! \brief Sets the internal thread to die and signals it
		void StopThread();

		//! \brief Checks if files marked as updated (only the first is actually checked) are available for reading
		void CheckUpdatesEnded();

		//! \brief Marks all files as not updated
		//!
		//! Useful for getting just one notification from all the files
		DLLEXPORT void MarkAllAsNotUpdated();

		//! \brief Checks whether a file is still marked as updated
		DLLEXPORT bool IsAFileStillUpdated() const;

	protected:

		void _RunListeningThread();
		// ------------------------------------ //

		//! The listening thread
		boost::thread ListenerThread;

		//! The folder in which to listen for stuff
		wstring TargetFolder;

		//! The files which are listened for
		std::vector<unique_ptr<wstring>> ListenedFiles;

		//! Marks the files that have been updated
		std::vector<bool> UpdatedFiles;

		//! Property set when quitting
		bool ShouldQuit;

		//! ID used to find this specific object
		int ID;

		//! The function called when a change is detected
		boost::function<void (const wstring &, ResourceFolderListener&)> CallbackFunction;

#ifdef _WIN32
		// Windows specific listener resources

		//! Vector of handles to wait for
		//!
		//! First handle is always the quit handle
		std::vector<HANDLE> SignalingHandles;


		FILE_NOTIFY_INFORMATION* OurReadBuffer;

		HANDLE TargetFolderHandle;

		OVERLAPPED* OverlappedInfo;

#else
		


#endif // _WIN32

	};




	//! \brief Allows various resource loaders to get notified when the file on disk changes
	//!
	//! Mainly used for quickly reloading GUI files after minor changes
	//! \note This class has lots of platform specific features which might not be available on non-windows platforms
	//! \todo Combine listeners with the same file into a single thing
	class ResourceRefreshHandler : public EngineComponent, public ThreadSafe{
	public:
		DLLEXPORT ResourceRefreshHandler();
		DLLEXPORT ~ResourceRefreshHandler();

		DLLEXPORT virtual bool Init();
		DLLEXPORT virtual void Release();

		
		//! \brief Starts listening for changes made to filestowatch
		//! \return True when properly started, false otherwise
		//! \param notifyfunction The function to call when one of the files change
		//! \param filestowatch A vector of files to listen changes on, the pointers will not be deleted. 
		//! The first file should contain a path to the folder which contains the files
		//! \param createdid Will contain the ID of the created listener, useful for stopping listening when the caller of this function is de-allocated
		//! \warning This function will not work properly if all the files aren't in the same folder
		//! \todo Allow watching for files in subfolders
		DLLEXPORT bool ListenForFileChanges(const std::vector<const wstring*> &filestowatch, 
			boost::function<void (const wstring &, ResourceFolderListener&)> notifyfunction, int &createdid);

		//! \brief Stops a listener with a specific id
		//! \todo Implement this
		//! \param idoflistener The ID returned by WatchForFileChanges in createdid variable
		//! \see WatchForFileChanges
		DLLEXPORT void StopListeningForFileChanges(int idoflistener);


		//! \brief Called by Engine to check are updated files available
		DLLEXPORT void CheckFileStatus();


		//! \brief Marks all files in all listeners matching any of the IDs as not updated
		DLLEXPORT void MarkListenersAsNotUpdated(const std::vector<int> &ids);


		DLLEXPORT static ResourceRefreshHandler* Get();

	protected:

		//! Holds all the active listeners
		std::vector<unique_ptr<ResourceFolderListener>> ActiveFileListeners;


		WantedClockType::time_point NextUpdateTime;




		static ResourceRefreshHandler* Staticaccess;

	};

}
#endif