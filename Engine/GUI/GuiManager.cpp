// ------------------------------------ //
#include "GuiManager.h"
// ------------------------------------ //
#include "../CEGUIInclude.h"
#include "Common/DataStoring/DataBlock.h"
#include "Common/DataStoring/DataStore.h"
#include "../TimeIncludes.h"
#include "Engine.h"
#include "Exceptions.h"
#include "FileSystem.h"
#include "GuiCollection.h"
#include "Handlers/ResourceRefreshHandler.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreManualObject.h"
#include "OgreRoot.h"
#include "OgreSceneManager.h"
#include "Rendering/GUI/FontManager.h"
#include "Rendering/GraphicalInputEntity.h"
#include "Rendering/Graphics.h"
#include "Script/ScriptExecutor.h"
#include "Window.h"
#include <boost/assign/list_of.hpp>
#include <thread>

#ifdef __linux__
// On linux the GuiManager has to create an Xlib window which requires this include...
#include "XLibInclude.h"

#endif

// ------------------------------------ //

// ------------------ GuiClipboardHandler ------------------ //
//! \brief Platform dependent clipboard handler
//! \todo Add support for linux
class Leviathan::Gui::GuiClipboardHandler :
    public CEGUI::NativeClipboardProvider, public ThreadSafe{
public:
	GuiClipboardHandler(Leviathan::Window* windprovider) :
    #ifdef _WIN32
        HWNDSource(windprovider),
    #endif
        OurOwnedBuffer(NULL)
    {
#ifdef __linux
        WaitingClipboard = false;
        
        // This will throw if it fails...
        SetupClipboardWindow();
#endif
    }

	~GuiClipboardHandler(){
        GUARD_LOCK();
#ifdef __linux
        XFlush(XDisplay);
        
        CleanUpWindow(guard);
#endif
        
		SAFE_DELETE(OurOwnedBuffer);
	}

	//! \brief Returns true when this object can manage the clipboard on this platform
	static bool WorksOnPlatform(){
#ifdef WIN32

		return true;
#elif __linux

		// Now on linux!
		return true;

#else
		return false;

#endif // WIN32
	}


#ifdef WIN32

	virtual void sendToClipboard(const CEGUI::String& mimeType, void* buffer, size_t size){
        GUARD_LOCK();
		// Ignore non-text setting //
		if(mimeType != "text/plain"){

			return;
		}


		if(!OpenClipboard(HWNDSource->GetHandle()))	{

			Logger::Get()->Error("GuiClipboardHandler: failed to open the clipboard");
			return;
		}

		// Clear the clipboard //
		if(!EmptyClipboard()){

			Logger::Get()->Error("GuiClipboardHandler: failed to empty the clipboard");
			return;
		}

		// Convert the line endings //
		std::string convertedstring = StringOperations::ChangeLineEndsToWindowsString(
            std::string(reinterpret_cast<char*>(buffer), size));
		

		// Allocate global data for the text //
		HGLOBAL globaldata = GlobalAlloc(GMEM_FIXED, convertedstring.size()+1);

		memcpy_s(globaldata, convertedstring.size()+1, convertedstring.c_str(), convertedstring.size());

		reinterpret_cast<char*>(globaldata)[convertedstring.size()] = 0;

		// Set the text data //
		if(::SetClipboardData(CF_TEXT, globaldata) == NULL){

			Logger::Get()->Error("GuiClipboardHandler: failed to set the clipboard contents");
			CloseClipboard();
			GlobalFree(globaldata);
			return;
		}

		// All done, close clipboard to allow others to use it, too //
		CloseClipboard();
	}

	virtual void retrieveFromClipboard(CEGUI::String& mimeType, void*& buffer, size_t& size){
        GUARD_LOCK();
		// Open the clipboard first //
		if(OpenClipboard(HWNDSource->GetHandle())){

			// Only retrieve text //
			HANDLE datahandle = GetClipboardData(CF_TEXT);

			if(datahandle == INVALID_HANDLE_VALUE || datahandle == NULL){
				return;
			}

			// Lock the data for reading //
			char* sourcebuff = reinterpret_cast<char*>(GlobalLock(datahandle));
			
			std::string fromclipdata = sourcebuff;

			// Unlock the global memory and clipboard //
			GlobalUnlock(datahandle);
			CloseClipboard();

			// Convert line ends to something nice //
			fromclipdata = StringOperations::ChangeLineEndsToUniversalString(fromclipdata);

			// Clear old data //
			SAFE_DELETE(OurOwnedBuffer);

			// Create a new data buffer //
			OurOwnedBuffer = new char[fromclipdata.size()+1];

			// Return the data //
			buffer = OurOwnedBuffer;

			memcpy_s(buffer, fromclipdata.size()+1, fromclipdata.c_str(), fromclipdata.size());

			// Make sure there is a null terminator //
			reinterpret_cast<char*>(buffer)[fromclipdata.size()] = 0;

			mimeType = "text/plain";
			size = fromclipdata.size()+1;
		}
	}

#elif __linux

	virtual void sendToClipboard(const CEGUI::String& mimeType, void* buffer, size_t size){
        GUARD_LOCK();
        
        // Ignore non-text setting //
		if(mimeType != "text/plain"){

			return;
		}

        // We must stop the message loop first //
        StopXMessageLoop(guard);
        
        // "Sending" to the clipboard is the easy part, responding to the requests is the harder part //
        Atom clipboardtarget = XInternAtom(XDisplay, "CLIPBOARD", false);
        
        // Tell Xlib that we know own the clipboard stuff //
        XSetSelectionOwner(XDisplay, clipboardtarget, ClipboardWindow, CurrentTime);
        XFlush(XDisplay);

        Logger::Get()->Info("Copied text to the X clipboard");

        // Store the text for retrieving later //
        SAFE_DELETE(OurOwnedBuffer);

        OurOwnedBuffer = new char[size];

        assert(OurOwnedBuffer && "failed to allocate buffer for clipboard text");
        
        // Copy the data //
        memcpy(OurOwnedBuffer, buffer, size);

        // Start handling clipboard data requests //
        StartXMessageLoop(guard);
        
	}

	virtual void retrieveFromClipboard(CEGUI::String& mimeType, void*& buffer, size_t& size){

        GUARD_LOCK_NAME(lockit);
        
        // We need to stop the message processing here, too //
        StopXMessageLoop(lockit);
        
        // Create a request //

        Atom targetproperty = XInternAtom(XDisplay, "CUT_BUFFER1", false);

        lockit.unlock();
        
        // We want the stuff in the clipboard //
        XConvertSelection(XDisplay, XA_CLIPBOARD(XDisplay), XA_STRING, targetproperty,
            ClipboardWindow, CurrentTime);
        XFlush(XDisplay);

        // Now we wait for the request to be completed //
        {
            Lock lock(ClipboardRetrieveMutex);

            WaitingClipboard = true;

            // We need to loop to run for it to process the response //
            StartXMessageLoop(lockit);

            while(WaitingClipboard){
                
                WaitForClipboard.wait_for(lock, MillisecondDuration(5));
            }

            lockit.lock();
        
            if(!ClipboardRequestSucceeded){

                Logger::Get()->Info("The clipboard was empty");
                size = 0;
                mimeType = "";
                buffer = nullptr;
                return;
            }
        }

        // We probably need to stop the processing loop again //
        StopXMessageLoop(lockit);

        // First read 0 bytes, to get the total size //
        Atom actualreturntype;
        int receivedformat;
        unsigned long receiveditems;
        unsigned long totalbytes;

        unsigned char* xbuffer;
        
        XGetWindowProperty(XDisplay, ClipboardWindow, targetproperty, 0, 0, false, XA_STRING,
            &actualreturntype, &receivedformat, &receiveditems, &totalbytes, &xbuffer);

        if(xbuffer)
            XFree(xbuffer);
        
        // All that is left to do is to retrieve the property text //
        // Might as well delete the data after this get
        XGetWindowProperty(XDisplay, ClipboardWindow, targetproperty, 0, totalbytes, true,
            XA_STRING, &actualreturntype, &receivedformat, &receiveditems, &totalbytes, &xbuffer);

        // Do last final checks on the data to make sure it is fine //
        if(receivedformat != 8){

            Logger::Get()->Warning("GuiClipboardHandler: received clipboard data is not 8 bit "
                "(one byte) aligned chars, actual type: "+Convert::ToString(receivedformat));
            goto finishprocessingthing;
        }

        if(!xbuffer || receiveditems < 1){

            Logger::Get()->Warning("GuiClipboardHandler: received empty xbuffer from clipboard "
                "property");
            goto finishprocessingthing;
        }

        if(totalbytes != 0){

            Logger::Get()->Warning("GuiClipboardHandler: failed to retrieve whole clipboard, "
                "bytes left: "+Convert::ToString(totalbytes));
        }

        // Reserve space and copy the string to our place //
        ReceivedClipboardData.resize(receiveditems);

        // The receiveditems might not be in bytes if other receivedformats than 8 are accepted...
        memcpy(const_cast<char*>(ReceivedClipboardData.c_str()), xbuffer, receiveditems);

        // Successfull retrieve is always text //
        mimeType = "text/plain";

        size = ReceivedClipboardData.size();

        // Set the CEGUI data pointer to our string
        buffer = const_cast<char*>(ReceivedClipboardData.c_str());

        Logger::Get()->Info("Succesfully retrieved text from clipboard, text is of length: "+
            Convert::ToString(ReceivedClipboardData.size()));

finishprocessingthing:
        
        // The buffer needs to be always released //
        if(xbuffer)
            XFree(xbuffer);

        // The message loop has to start again after all the Xlib calls //
        StartXMessageLoop(lockit);
	}
    

#else

	// Nothing //
	virtual void sendToClipboard(const CEGUI::String& mimeType, void* buffer, size_t size){

        

        
	}

	virtual void retrieveFromClipboard(CEGUI::String& mimeType, void*& buffer, size_t& size){
	
	}

#endif // WIN32


private:

    // Common data //
#ifdef _WIN32
    Leviathan::Window* HWNDSource;
#endif

	//! The owned buffer, which has to be deleted by this
	char* OurOwnedBuffer;

#ifdef __linux
private:
    // Linux specific parts //

    //! The message loop for the Xlib thread
    void RunXMessageLoop(){

        XEvent event;

        while(RunMessageProcessing){

            XNextEvent(XDisplay, &event);

            switch(event.type){
                case VisibilityNotify:
                {
                    if(event.xvisibility.state != VisibilityFullyObscured)
                        Logger::Get()->Warning("GuiClipboardHandler: clipboard window shoulnd't "
                            "become visible");
                }
                break;
                case PropertyNotify:
                {
                    // Ignore removed properties //
                    if(event.xproperty.state == PropertyDelete){

                        break;
                    }

                    // Might want to get this somewhere else
                    Atom pasteresponse = XInternAtom(XDisplay, "CUT_BUFFER1", false);
                    
                    // Check what changed //
                    if(event.xproperty.atom == XA_STRING){

                        // This is the stop message //
                        Logger::Get()->Info("Received the stop property");

                        
                    } else if(event.xproperty.atom == pasteresponse){
                        
                        Logger::Get()->Info("Received clipboard request's property");
                            
                        // We received the clipboard contents //
                        {
                            Lock lock(ClipboardRetrieveMutex);

                            ClipboardRequestSucceeded = true;
                            WaitingClipboard = false;
                        }

                        WaitForClipboard.notify_all();
                    }

                }
                break;
                case SelectionRequest:
                {
                    // Prepare a response for the requester //
                    XEvent response;
                    GUARD_LOCK();

                    // Ignore if we got nothing //
                    if(OurOwnedBuffer){
                    
                        if((event.xselectionrequest.target == XA_STRING ||
                                (event.xselectionrequest.target == XA_UTF8_STRING(XDisplay)))
                            &&
                                event.xselectionrequest.selection == XA_CLIPBOARD(XDisplay))
                        {

                            if(response.xselection.property == None){

                                Logger::Get()->Warning("Clipboard request property is None, "
                                    "and we decided that CUT_BUFFER1 is a good choice");

                                // Let's just use this property //
                                response.xselection.property = XInternAtom(XDisplay,
                                    "CUT_BUFFER1", false);
                            }

                            Logger::Get()->Info("Sending clipboard text to requester");
                            
                            // Send the text to the requester //
                            XChangeProperty(XDisplay, event.xselectionrequest.requestor,
                                event.xselectionrequest.property,
                                XA_STRING, 8, PropModeReplace,
                                reinterpret_cast<unsigned char*>(OurOwnedBuffer),
                                static_cast<int>(strlen(OurOwnedBuffer)));
                        
                            response.xselection.property = event.xselectionrequest.property;
                        
                        } else if(event.xselectionrequest.target == XA_TARGETS(XDisplay)
                            && event.xselectionrequest.selection == XA_CLIPBOARD(XDisplay)
                            && OurOwnedBuffer)
                        {
                            Logger::Get()->Info("Sending supported formats to clipboard "
                                "requester");
                            
                            // We need to tell the requester what types are supported
                            Atom supported[] = {XA_UTF8_STRING(XDisplay), XA_STRING};
                        
                            XChangeProperty(XDisplay, event.xselectionrequest.requestor,
                                event.xselectionrequest.property,
                                XA_TARGETS(XDisplay), 32, PropModeReplace,
                                reinterpret_cast<unsigned char*>(&supported),
                                sizeof(supported)/sizeof(supported[0]));
                        
                        } else {

                            Logger::Get()->Info("Don't know how to respond to clipboard request");
                            response.xselection.property = None;
                        }
                    } else {

                        response.xselection.property = None;
                        Logger::Get()->Info("Clipboard is empty...");
                    }
                    
                    response.xselection.type = SelectionNotify;
                    response.xselection.display = event.xselectionrequest.display;
                    response.xselection.requestor = event.xselectionrequest.requestor;
                    response.xselection.selection = event.xselectionrequest.selection;
                    response.xselection.target = event.xselectionrequest.target;
                    response.xselection.time = event.xselectionrequest.time;
                    
                    XSendEvent(XDisplay, event.xselectionrequest.requestor, 0, 0, &response);
                    XFlush(XDisplay);
                }
                break;
                case SelectionClear:
                {
                    // We no longer hold the clipboard //
                    Logger::Get()->Info("We now no longer own the clipboard");
                    SAFE_DELETE(OurOwnedBuffer);

                }
                break;
                case SelectionNotify:
                {
                    Lock lock(ClipboardRetrieveMutex);
                    
                    // We received something from the selection owner //
                    if(event.xselection.property == None){

                        Logger::Get()->Info("Nothing to paste, clipboard empty");
                        ClipboardRequestSucceeded = false;
                        WaitingClipboard = false;
                        WaitForClipboard.notify_all();
                        break;
                    }

                    // No reason that it would have failed... //
                    ClipboardRequestSucceeded = true;
                    WaitForClipboard.notify_all();
                }
                break;


            }
        }

    }


    //! Sets up the clipboard window for usage
    void SetupClipboardWindow(){

        GUARD_LOCK();
        
        // First get the default display //
        XDisplay = XOpenDisplay(NULL);

        if(!XDisplay)
            throw InvalidState("cannot open XDisplay");

        // Setup the window attributes //
        XSetWindowAttributes properties;

        properties.event_mask = VisibilityChangeMask | PropertyChangeMask;

        
        // Then we create the window //
        ClipboardWindow = XCreateWindow(XDisplay, DefaultRootWindow(XDisplay), 0, 0, 1, 1, 0,
            CopyFromParent, InputOutput, CopyFromParent,
            CWEventMask, &properties);

        if(!ClipboardWindow)
            throw InvalidState("cannot create clipboard window");


        // Hide the window //
        XEvent xev;
        Atom wm_state = XInternAtom(XDisplay, "_NET_WM_STATE", false);
        Atom statehidden = XInternAtom(XDisplay, "_NET_WM_STATE_HIDDEN", false);
        Atom skiptaskbar = XInternAtom(XDisplay, "_NET_WM_STATE_SKIP_TASKBAR", false);
        Atom nopager = XInternAtom(XDisplay, "_NET_WM_STATE_SKIP_PAGER", false);

        Atom addstate = XInternAtom(XDisplay, "_NET_WM_STATE_ADD", false);

        memset(&xev, 0, sizeof(xev));
        xev.type = ClientMessage;
        xev.xclient.window = ClipboardWindow;
        xev.xclient.message_type = wm_state;
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = addstate;
        xev.xclient.data.l[1] = statehidden;
        xev.xclient.data.l[2] = skiptaskbar;

        XSendEvent(XDisplay, DefaultRootWindow(XDisplay), false, SubstructureNotifyMask, &xev);

        // We need to set more properties... //
        memset(&xev, 0, sizeof(xev));
        xev.type = ClientMessage;
        xev.xclient.window = ClipboardWindow;
        xev.xclient.message_type = wm_state;
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = addstate;
        xev.xclient.data.l[1] = nopager;

        
        XSendEvent(XDisplay, DefaultRootWindow(XDisplay), false, SubstructureNotifyMask, &xev);
        
        // The loop should run //
        StartXMessageLoop(guard);
    }

    //! Cleans up the window
    void CleanUpWindow(Lock &guard){
        
        // First stop the message loop //
        StopXMessageLoop(guard);
        
        Logger::Get()->Info("GUI clipboard is ready to be destroyed");

        // Now the window is ready for closing //
        XDestroyWindow(XDisplay, ClipboardWindow);
        
        // And then the connection //
        XCloseDisplay(XDisplay);

        XDisplay = 0;
        ClipboardWindow = 0;
    }

    //! \brief Starts the Xlib message loop for responding to requests
    //!
    //! For use after stopping the loop
    void StartXMessageLoop(Lock &guard){

        RunMessageProcessing = true;
        
        XMessageThread = std::thread(&GuiClipboardHandler::RunXMessageLoop, this);
    }

    //! \brief Stops the message processing
    void StopXMessageLoop(Lock &guard){

        // First signal the loop to stop //
        RunMessageProcessing = false;

        // Then send an event for it to process... //
        unsigned char stopproperty[] = "stop";

        XChangeProperty(XDisplay, ClipboardWindow, XA_STRING, XA_STRING, 8,
            PropModeReplace, stopproperty, 5);

        XFlush(XDisplay);

        // Then wait for the message loop to end //
        XMessageThread.join();
    }
    
    // These are used for clipboard access //


    //! Holds the received text from the clipboard
    std::string ReceivedClipboardData;

    //! The current XDisplay
    Display* XDisplay;

    //! Our hidden clipboard window
    ::Window ClipboardWindow;
        
    std::thread XMessageThread;

    //! Denotes whether the window loop should run
    bool RunMessageProcessing;

    //! This is waited for while the clipboard is accessed
    std::condition_variable_any WaitForClipboard;

    //! Set to true while waiting for clipboard thread to do something
    bool WaitingClipboard;

    //! This is a lock for clipboard content retrieve
    Mutex ClipboardRetrieveMutex;

    //! Denotes whether clipboard grap failed or succeeded
    bool ClipboardRequestSucceeded;
        
#endif

    

};



using namespace Leviathan;
using namespace Gui;
using namespace std;
// ------------------ GuiManager ------------------ //
Leviathan::Gui::GuiManager::GuiManager() :
    ID(IDFactory::GetID())
{
	
}
Leviathan::Gui::GuiManager::~GuiManager(){

	
}
// ------------------------------------ //
bool Leviathan::Gui::GuiManager::Init(Graphics* graph, GraphicalInputEntity* window,
    bool ismain)
{
	GUARD_LOCK();

	ThisWindow = window;
    MainGuiManager = ismain;
	
	// Create the clipboard handler for this window (only one is required,
    // so only create if this is the main window's gui
    if(MainGuiManager){
        
        try{
            _GuiClipboardHandler = new GuiClipboardHandler(window->GetWindow());
            
        } catch(const Exception &e){

            // Clipboard isn't usable... //
            Logger::Get()->Warning("GuiManager: failed to create a ClipboardHandler: "
                "cannot copy or paste text, exception:");
            e.PrintToLog();
            
            _GuiClipboardHandler = NULL;
        }
    }
    
	// Setup this window's context //
	GuiContext = &CEGUI::System::getSingleton().createGUIContext(
        ThisWindow->GetCEGUIRenderer()->getDefaultRenderTarget());

	// Setup input for the context //
	ContextInput = new CEGUI::InputAggregator(GuiContext);
	ContextInput->initialise(false);

	// Set Simonetta as the default font //
    GuiContext->setDefaultFont("Simonetta-Regular");


	// Set the taharez looks active //
	SetMouseTheme(guard, "TaharezLook/MouseArrow");
	GuiContext->setDefaultTooltipType("TaharezLook/Tooltip");


	// Make the clipboard play nice //
	if(MainGuiManager == 1){

		// Only one clipboard is needed //
		if(_GuiClipboardHandler && _GuiClipboardHandler->WorksOnPlatform())
			CEGUI::System::getSingleton().getClipboard()->setNativeProvider(
                _GuiClipboardHandler);
	}

	// Store the initial time //
	LastTimePulseTime = Time::GetThreadSafeSteadyTimePoint();

	return true;
}

void Leviathan::Gui::GuiManager::Release(){
	GUARD_LOCK();
	

	// Stop with the file updates //
	if(FileChangeID){


		auto tmphandler = ResourceRefreshHandler::Get();

		if(tmphandler){

			tmphandler->StopListeningForFileChanges(FileChangeID);
		}
        
		FileChangeID = 0;
	}

	// Default mouse back //
	SetMouseTheme(guard, "none");

	// Release objects first //

	for(size_t i = 0; i < Objects.size(); i++){

		Objects[i]->ReleaseData();
		SAFE_RELEASE(Objects[i]);
	}
    
	Objects.clear();

	// GuiCollections are now also reference counted //
	for(size_t i = 0; i < Collections.size(); i++){
		SAFE_RELEASE(Collections[i]);
	}

	Collections.clear();

	ContextInput->removeAllEvents();
	SAFE_DELETE(ContextInput);

	// Destroy the GUI //
	CEGUI::System::getSingleton().destroyGUIContext(*GuiContext);
	GuiContext = NULL;

    // If we are the main window unhook the clipboard //
    if(_GuiClipboardHandler && MainGuiManager){

        CEGUI::System::getSingleton().getClipboard()->setNativeProvider(NULL);
        Logger::Get()->Info("GuiManager: main manager has detached the clipboard successfully");
    }
    
	SAFE_DELETE(_GuiClipboardHandler);

    Logger::Get()->Info("GuiManager: Gui successfully closed on window");
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::GuiManager::ProcessKeyDown(int32_t key, int specialmodifiers){
    
	GUARD_LOCK();
    
	for(size_t i = 0; i < Collections.size(); i++){
		if(Collections[i]->GetTogglingKey().Match(key, specialmodifiers, false) &&
            Collections[i]->GetAllowEnable())
        {
			// Is a match, toggle //
			Collections[i]->ToggleState(guard);

			return true;
		}
	}

	return false;
}

DLLEXPORT void Leviathan::Gui::GuiManager::SetCollectionState(const string &name, bool state){
	GUARD_LOCK();
	// find collection with name and set it's state //
	for(size_t i = 0; i < Collections.size(); i++){
		if(Collections[i]->GetName() == name){
			// set state //
			if(Collections[i]->GetState() != state){
                
				Collections[i]->ToggleState(guard);
			}
			return;
		}
	}
	// Complain //
	Logger::Get()->Warning("GuiManager: SetCollectionState: couldn't find a collection "
        "with name: " + name);
}

DLLEXPORT void Leviathan::Gui::GuiManager::SetCollectionAllowEnableState(const string &name,
    bool allow /*= true*/)
{
	GUARD_LOCK();
	// find collection with name and set it's allow enable state //
	for(size_t i = 0; i < Collections.size(); i++){
		if(Collections[i]->GetName() == name){
			// set state //
			if(Collections[i]->GetAllowEnable() != allow){
				Logger::Get()->Info("Setting Collection "+Collections[i]->GetName()+
                    " allow enable state "+Convert::ToString(allow));
				Collections[i]->ToggleAllowEnable();
			}
			return;
		}
	}
}
// ------------------------------------ //
void Leviathan::Gui::GuiManager::GuiTick(int mspassed){
	GUARD_LOCK();

    if(ReloadQueued){

        ReloadQueued = false;

        // Reload //
        LOG_INFO("GuiManager: reloading file: " + MainGUIFile);
        
        // Store the current state //
        auto currentstate = GetGuiStates(guard);

        UnLoadGUIFile(guard);

        // Now load it //
        if(!LoadGUIFile(guard, MainGUIFile, true)){

            Logger::Get()->Error("GuiManager: file changed: couldn't load updated file: "+
                MainGUIFile);
        }

        // Apply back the old states //
        ApplyGuiStates(guard, currentstate.get());
    }

	// check if we want mouse //
	if(GuiMouseUseUpdated){
		GuiMouseUseUpdated = false;

		// scan if any collections keep GUI active //
		bool active = false;


		for(size_t i = 0; i < Collections.size(); i++){

			if(Collections[i]->KeepsGUIActive()){
				active = true;
				break;
			}
		}

		if(active != GuiDisallowMouseCapture){
			// state updated //
			GuiDisallowMouseCapture = active;

			if(GuiDisallowMouseCapture){
				// disable mouse capture //
				ThisWindow->SetMouseCapture(false);

			} else {

                // Prevent capturing the mouse if disabled //
                if(DisableGuiMouseCapture){

                    GuiDisallowMouseCapture = true;
                    
                } else {
                
                    // activate direct mouse capture //
                    if(!ThisWindow->SetMouseCapture(true)){
                        // failed, GUI must be forced to stay on //
                        OnForceGUIOn();
                        GuiDisallowMouseCapture = true;
                        GuiMouseUseUpdated = true;
                    }
                }
			}
		}
	}
}

DLLEXPORT void Leviathan::Gui::GuiManager::OnForceGUIOn(){
	DEBUG_BREAK;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::GuiManager::SetDisableMouseCapture(bool newvalue){

    DisableGuiMouseCapture = newvalue;
    // This will cause the capture state to be checked next tick
    GuiMouseUseUpdated = true;
}

// ------------------------------------ //
void Leviathan::Gui::GuiManager::Render(){
	GUARD_LOCK();

	// Pass time //
	auto newtime = Time::GetThreadSafeSteadyTimePoint();
	
	SecondDuration elapsed = newtime-LastTimePulseTime;

	float changval = elapsed.count();

	GuiContext->injectTimePulse(changval);

	// Potentially pass to system //
	if(MainGuiManager){

		CEGUI::System::getSingleton().injectTimePulse(changval);
	}

	LastTimePulseTime = newtime;

    guard.unlock();
    
	// Update inputs //


}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::GuiManager::OnResize(){
	GUARD_LOCK();

	// Notify the CEGUI system //
    // TODO: only to the wanted context
	CEGUI::System* const sys = CEGUI::System::getSingletonPtr();


    int32_t width, height;
    ThisWindow->GetWindow()->GetSize(width, height);
    
	if(sys)
		sys->notifyDisplaySizeChanged(CEGUI::Sizef((float)width,
                (float)height));
}

DLLEXPORT void Leviathan::Gui::GuiManager::OnFocusChanged(bool focused){
	GUARD_LOCK();
	
	// Notify our context //
	if(!focused)
		ContextInput->injectMouseLeaves();

}
// ------------------------------------ //
bool Leviathan::Gui::GuiManager::AddGuiObject(Lock &guard, BaseGuiObject* obj){
	Objects.push_back(obj);
	return true;
}

void Leviathan::Gui::GuiManager::DeleteObject(int id){
	GUARD_LOCK();
	for(size_t i = 0; i < Objects.size(); i++){
		if(Objects[i]->GetID() == id){

			Objects[i]->ReleaseData();
			SAFE_RELEASE(Objects[i]);
			Objects.erase(Objects.begin()+i);
			return;
		}
	}
}

int Leviathan::Gui::GuiManager::GetObjectIndexFromId(int id){
	GUARD_LOCK();
	for(size_t i = 0; i < Objects.size(); i++){
		if(Objects[i]->GetID() == id)
			return static_cast<int>(i);
	}
	return -1;
}

BaseGuiObject* Leviathan::Gui::GuiManager::GetObject(unsigned int index){
	GUARD_LOCK();
	if(index < Objects.size()){
        
		return Objects[index];
	}
	return NULL;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::GuiManager::LoadGUIFile(Lock &guard, const string &file,
    bool nochangelistener, int iteration /*= 0*/)
{
    if(iteration > 10){

        Logger::Get()->Warning("GuiManager: aborting file load on iteration "+
            Convert::ToString(iteration));
        return false;
    }
    
	// Parse the file //
	auto data = ObjectFileProcessor::ProcessObjectFile(file, Logger::Get());

	if(!data){
		return false;
	}

	MainGUIFile = file;

	NamedVars& varlist = *data->GetVariables();

	string relativepath;
    
	// Get path //
	ObjectFileProcessor::LoadValueFromNamedVars<string>(
        varlist, "GUIBaseFile", relativepath, "", Logger::Get(),
        "GuiManager: LoadGUIFile: no base file defined (in "+file+") : ");

    // This can be used to verify that CEGUI events are properly hooked //
    bool requireevent;

    ObjectFileProcessor::LoadValueFromNamedVars<bool>(
        varlist, "RequireCEGUIHooked", requireevent, false);

	if(!relativepath.size()){

		return false;
	}
	
	// Load it //
	CEGUI::Window* rootwindow = NULL;
	try{

		rootwindow = CEGUI::WindowManager::getSingleton().loadLayoutFromFile(relativepath);

	} catch(const Ogre::Exception &e){

		Logger::Get()->Error("GuiManager: LoadGUIFile: failed to locate file: "+relativepath+":");
		Logger::Get()->Write(string("\t> ")+e.what());
		return false;
	} catch(const CEGUI::GenericException &e2){

		Logger::Get()->Error("GuiManager: LoadGUIFile: failed to parse CEGUI layout: "+
            relativepath+":");
		Logger::Get()->Write(string("\t> ")+e2.what());
		return false;
	} catch(const CEGUI::InvalidRequestException &e3){

		Logger::Get()->Error("GuiManager: LoadGUIFile: failed to parse CEGUI layout: "+
            relativepath+":");
		Logger::Get()->Write(string("\t> ")+e3.what());
		return false;
	}

	// Check did it work //
	if(!rootwindow){

		Logger::Get()->Error("GuiManager: LoadGUIFile: failed to parse layout file: "+
            relativepath);
		return false;
	}

	// Look for animation files //
	auto animslist = varlist.GetValueDirectRaw("GUIAnimations");

	if(animslist){

		if(!animslist->CanAllBeCastedToType<string>()){

			Logger::Get()->Warning("GuiManager: LoadGUIFile: gui file has defined gui animation "
                "files in wrong format (expected a list of strings), file: "+relativepath);

		} else {

			// Load them //
			for(size_t i = 0; i < animslist->GetVariableCount(); i++){

				string curfile;
				animslist->GetValue(i).ConvertAndAssingToVariable<string>(curfile);

				// Check is the file already loaded //
				{
					Lock gguard(GlobalGUIMutex);

					if(IsAnimationFileLoaded(gguard, curfile)){

						// Don't load again //
						continue;
					}

					// Set as loaded //
					SetAnimationFileLoaded(gguard, curfile);
				}

				try{

					CEGUI::AnimationManager::getSingleton().loadAnimationsFromXML(curfile);

				} catch(const Ogre::Exception &e){

					Logger::Get()->Warning("GuiManager: LoadGUIFile: failed to locate gui "
                        " animation file: "+curfile+":");
					Logger::Get()->Write(string("\t> ")+e.what());

				} catch(const CEGUI::GenericException &e2){

					Logger::Get()->Error("GuiManager: LoadGUIFile: failed to parse CEGUI "
                        "animation file layout: "+curfile+":");
					Logger::Get()->Write(string("\t> ")+e2.what());
				}
			}
		}
	}

	// Set it as the visible sheet //
	GuiContext->setRootWindow(rootwindow);


	// temporary object data stores //
	vector<BaseGuiObject*> TempOs;

	// reserve space //
	size_t totalcount = data->GetTotalObjectCount();

	TempOs.reserve(totalcount);


	for(size_t i = 0; i < totalcount; i++){

		auto objecto = data->GetObjectFromIndex(i);

		// Check what type the of the object is //
		if(objecto->GetTypeName() == "GuiCollection"){

			if(!GuiCollection::LoadCollection(guard, this, *objecto)){

				// report error //
				Logger::Get()->Error("GuiManager: ExecuteGuiScript: failed to load collection, "
                    "named "+objecto->GetName());
				continue;
			}

			continue;

		} else if(objecto->GetTypeName() == "GuiObject"){

			// try to load //
			if(!BaseGuiObject::LoadFromFileStructure(guard, this, TempOs, *objecto)){

				// report error //
				Logger::Get()->Error("GuiManager: ExecuteGuiScript: failed to load GuiObject, "
                    "named "+objecto->GetName());
                
				continue;
			}

			continue;
		}

		Logger::Get()->Error("GuiManager: ExecuteGuiScript: Unrecognized type! typename: "+
            objecto->GetTypeName());
	}

    // Verify loaded hooks, if wanted //
    if(requireevent){

        bool foundsomething = false;

        auto end = TempOs.end();
        for(auto iter = TempOs.begin(); iter != end; ++iter){

            if((*iter)->IsCEGUIEventHooked()){

                foundsomething = true;
                break;
            }
        }

        if(!foundsomething){

            // Report that we will attempt to reload the file //
            Logger::Get()->Warning("GuiManager: while trying to load \""+file+
                "\" RequireCEGUIHooked is true, but no GUI object has any hooked CEGUI "
                "listeners, retrying load: ");

            UnLoadGUIFile();

            for(size_t i = 0; i < TempOs.size(); i++){
		
                TempOs[i]->ReleaseData();
                SAFE_RELEASE(TempOs[i]);
            }

            Logger::Get()->Write("\t> Doing iteration "+Convert::ToString(iteration+1));

            return LoadGUIFile(guard, file, nochangelistener, iteration+1);
        }
    }
	

	for(size_t i = 0; i < TempOs.size(); i++){

		// add to real objects //
		AddGuiObject(guard, TempOs[i]);
	}

	// This avoids having more and more change listeners each reload //
	if(!nochangelistener){
		// Listen for file changes //
		auto tmphandler = ResourceRefreshHandler::Get();
	
		if(tmphandler){

			// \todo Detect if the files are in different folders and start multiple listeners
			std::vector<const string*> targetfiles =
                boost::assign::list_of(&file)(&relativepath);

			tmphandler->ListenForFileChanges(targetfiles, std::bind(&GuiManager::_FileChanged,
                    this, placeholders::_1, placeholders::_2),
                FileChangeID);
		}
	}

	return true;
}

DLLEXPORT void Leviathan::Gui::GuiManager::UnLoadGUIFile(Lock &guard){

	// Unload all objects //
	for(size_t i = 0; i < Objects.size(); i++){
		
		Objects[i]->ReleaseData();
		SAFE_RELEASE(Objects[i]);
	}

	Objects.clear();

	// Unload all collections //
	for(size_t i = 0; i < Collections.size(); i++){
		SAFE_RELEASE(Collections[i]);
	}

	Collections.clear();


	// Unload the CEGUI file //
	auto curroot = GuiContext->getRootWindow();

	CEGUI::WindowManager::getSingleton().destroyWindow(curroot);

	GuiContext->setRootWindow(NULL);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::GuiManager::SetMouseTheme(Lock &guard, const string &tname){

	if(tname == "none"){

		// show default window cursor //
		ThisWindow->GetWindow()->SetHideCursor(false);
		return;
	}

	// Set it active //
	GuiContext->getCursor().setDefaultImage(tname);

	// hide window cursor //
	ThisWindow->GetWindow()->SetHideCursor(true);
}
// ------------------------------------ //
DLLEXPORT CEGUI::GUIContext* Leviathan::Gui::GuiManager::GetMainContext(Lock &guard){
	return GuiContext;
}
// ----------------- collection managing --------------------- //
void GuiManager::AddCollection(Lock &guard, GuiCollection* add){
    
	Collections.push_back(add);
}

GuiCollection* Leviathan::Gui::GuiManager::GetCollection(const int &id, const string &name){
	GUARD_LOCK();
	// look for collection based on id or name //
	for(size_t i = 0; i < Collections.size(); i++){
		if(id >= 0){
			if(Collections[i]->GetID() != id){
				// no match //
				continue;
			}
		} else {
			// name should be specified, check for it //
			if(Collections[i]->GetName() != name){
				continue;
			}
		}

		// match
		return Collections[i];
	}

	return NULL;
}
// ------------------------------------ //
void Leviathan::Gui::GuiManager::_FileChanged(const string &file,
    ResourceFolderListener &caller)
{
	// Any updated file will cause whole reload //
	GUARD_LOCK();

    ReloadQueued = true;

	// Mark everything as non-updated //
	caller.MarkAllAsNotUpdated();
}
// ------------------ Static part ------------------ //
std::vector<string> Leviathan::Gui::GuiManager::LoadedAnimationFiles;

Mutex Leviathan::Gui::GuiManager::GlobalGUIMutex;

bool Leviathan::Gui::GuiManager::IsAnimationFileLoaded(Lock &lock, const string &file){
    
	for(size_t i = 0; i < LoadedAnimationFiles.size(); i++){

		if(LoadedAnimationFiles[i] == file){

			return true;
		}
	}

	// Not found, must not be loaded then //
	return false;
}

void Leviathan::Gui::GuiManager::SetAnimationFileLoaded(Lock &lock, const string &file){

	LoadedAnimationFiles.push_back(file);
}

DLLEXPORT void Leviathan::Gui::GuiManager::KillGlobalCache(){
	Lock guard(GlobalGUIMutex);

	// Release the memory to not look like a leak //
	LoadedAnimationFiles.clear();

	auto single = CEGUI::AnimationManager::getSingletonPtr();

	if(single)
		single->destroyAllAnimations();
}
// ------------------------------------ //
DLLEXPORT CEGUI::Window* Leviathan::Gui::GuiManager::GetWindowByStringName(Lock &guard,
    const string &namepath)
{
	try{

		return GuiContext->getRootWindow()->getChild(namepath);

	} catch(const CEGUI::UnknownObjectException&){

		// Not found //
		return NULL;
	}
}

DLLEXPORT bool Leviathan::Gui::GuiManager::PlayAnimationOnWindow(Lock &guard,
    const string &windowname, const string &animationname, bool applyrecursively,
    const string &ignoretypenames)
{
	// First get the window //
	auto wind = GetWindowByStringName(guard, windowname);

	if(!wind)
		return false;

	// Next create the animation instance //
	CEGUI::Animation* animdefinition = NULL;
	
	try{

		animdefinition = CEGUI::AnimationManager::getSingleton().getAnimation(animationname);

	} catch(const CEGUI::UnknownObjectException&){

		return false;
	}


	if(!animdefinition)
		return false;


	_PlayAnimationOnWindow(guard, wind, animdefinition, applyrecursively, ignoretypenames);
	return true;
}

DLLEXPORT bool Leviathan::Gui::GuiManager::PlayAnimationOnWindowProxy(const string &windowname,
    const string &animationname)
{
    GUARD_LOCK();
	return PlayAnimationOnWindow(guard, windowname, animationname);
}

void Leviathan::Gui::GuiManager::_PlayAnimationOnWindow(Lock &guard,
    CEGUI::Window* targetwind, CEGUI::Animation* animdefinition, bool recurse,
    const string &ignoretypenames)
{
	// Apply only if the typename doesn't match ignored names //
	if(ignoretypenames.find(targetwind->getType().c_str()) == string::npos &&
        targetwind->getName().at(0) != '_')
    {

		// Create an animation instance //
		CEGUI::AnimationInstance* createdanim =
            CEGUI::AnimationManager::getSingleton().instantiateAnimation(animdefinition);

		// Apply the instance //
		createdanim->setTargetWindow(targetwind);

		createdanim->start();
	}

	// Recurse to child elements if desired //
	if(recurse){

		// Find all child windows and call this method on them //
		for(size_t i = 0; i < targetwind->getChildCount(); i++){
			auto newtarget = targetwind->getChildAtIdx(i);

			_PlayAnimationOnWindow(guard, newtarget, animdefinition, recurse, ignoretypenames);
		}
	}
}
// ------------------------------------ //
DLLEXPORT std::unique_ptr<GuiCollectionStates> Leviathan::Gui::GuiManager::GetGuiStates(Lock &guard) const{
    
	// Create the result object using the size of Collections as all of them are added there //
	unique_ptr<GuiCollectionStates> result(new GuiCollectionStates(Collections.size()));

	// Add all the states and names of the collections //
	for(size_t i = 0; i < Collections.size(); i++){

		result->AddNewEntry(Collections[i]->GetName(), Collections[i]->GetState());
	}

	return result;
}

DLLEXPORT void Leviathan::Gui::GuiManager::ApplyGuiStates(Lock &guard, const GuiCollectionStates* states){

	// Apply all the states from the object //
	for(size_t i = 0; i < states->CollectionNames.size(); i++){

		auto foundcollect = GetCollection(-1, *states->CollectionNames[i]->Name);

		// If found check whether the states match if not change to the right state //
		if(foundcollect && foundcollect->GetState() != states->CollectionNames[i]->IsEnabled){

			// Change the state //
			foundcollect->UpdateState(guard, states->CollectionNames[i]->IsEnabled);
		}
	}
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::GuiManager::InjectPasteRequest(){
	return ContextInput->injectPasteRequest();
}

DLLEXPORT bool Leviathan::Gui::GuiManager::InjectCopyRequest(){
	return ContextInput->injectCopyRequest();
}

DLLEXPORT bool Leviathan::Gui::GuiManager::InjectCutRequest(){
	return ContextInput->injectCutRequest();
}
