// ------------------------------------ //
#include "GuiManager.h"
// ------------------------------------ //
#include "CEGUI/AnimationInstance.h"
#include "CEGUI/AnimationManager.h"
#include "CEGUI/Clipboard.h"
#include "CEGUI/InputAggregator.h"
#include "CEGUI/RenderTarget.h"
#include "CEGUI/System.h"
#include "CEGUI/Window.h"
#include "CEGUI/WindowManager.h"
#include "Common/DataStoring/DataBlock.h"
#include "Common/DataStoring/DataStore.h"
#include "Common/Misc.h"
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
#include <boost/thread.hpp>

#ifdef __linux
// On linux the GuiManager has to create an Xlib window which requires this include...
#include "XLibInclude.h"

#endif

// ------------------------------------ //

// ------------------ GuiClipboardHandler ------------------ //
//! \brief Platform dependent clipboard handler
//! \todo Add support for linux
class Leviathan::Gui::GuiClipboardHandler : public CEGUI::NativeClipboardProvider, public ThreadSafe{
public:
	GuiClipboardHandler(Leviathan::Window* windprovider) : HWNDSource(windprovider), OurOwnedBuffer(NULL)
    {
#ifdef __linux
        // This will throw if it fails...
        SetupClipboardWindow();
#endif
    }

	~GuiClipboardHandler(){
        GUARD_LOCK_THIS_OBJECT();
#ifdef __linux
        XFlush(XDisplay);
        
        CleanUpWindow();
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
        GUARD_LOCK_THIS_OBJECT();
		// Ignore non-text setting //
		if(mimeType != "text/plain"){

			return;
		}


		if(!OpenClipboard(HWNDSource->GetHandle()))	{

			Logger::Get()->Error(L"GuiClipboardHandler: failed to open the clipboard", GetLastError());
			return;
		}

		// Clear the clipboard //
		if(!EmptyClipboard()){

			Logger::Get()->Error(L"GuiClipboardHandler: failed to empty the clipboard", GetLastError());
			return;
		}

		// Convert the line endings //
		string convertedstring = StringOperations::ChangeLineEndsToWindowsString(
            string(reinterpret_cast<char*>(buffer), size));
		

		// Allocate global data for the text //
		HGLOBAL globaldata = GlobalAlloc(GMEM_FIXED, convertedstring.size()+1);

		memcpy_s(globaldata, convertedstring.size()+1, convertedstring.c_str(), convertedstring.size());

		reinterpret_cast<char*>(globaldata)[convertedstring.size()] = 0;

		// Set the text data //
		if(::SetClipboardData(CF_TEXT, globaldata) == NULL){

			Logger::Get()->Error(L"GuiClipboardHandler: failed to set the clipboard contents", GetLastError());
			CloseClipboard();
			GlobalFree(globaldata);
			return;
		}

		// All done, close clipboard to allow others to use it, too //
		CloseClipboard();
	}

	virtual void retrieveFromClipboard(CEGUI::String& mimeType, void*& buffer, size_t& size){
        GUARD_LOCK_THIS_OBJECT();
		// Open the clipboard first //
		if(OpenClipboard(HWNDSource->GetHandle())){

			// Only retrieve text //
			HANDLE datahandle = GetClipboardData(CF_TEXT);

			if(datahandle == INVALID_HANDLE_VALUE || datahandle == NULL){
				return;
			}

			// Lock the data for reading //
			char* sourcebuff = reinterpret_cast<char*>(GlobalLock(datahandle));
			
			string fromclipdata = sourcebuff;

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
        GUARD_LOCK_THIS_OBJECT();
        
        // Ignore non-text setting //
		if(mimeType != "text/plain"){

			return;
		}

        // We must stop the message loop first //
        StopXMessageLoop();
        
        // "Sending" to the clipboard is the easy part, responding to the requests is the harder part //
        Atom clipboardtarget = XInternAtom(XDisplay, "CLIPBOARD", false);
        
        // Tell Xlib that we know own the clipboard stuff //
        XSetSelectionOwner(XDisplay, clipboardtarget, ClipboardWindow, CurrentTime);
        XFlush(XDisplay);

        Logger::Get()->Info(L"Copied text to the X clipboard");

        // Store the text for retrieving later //
        SAFE_DELETE(OurOwnedBuffer);

        OurOwnedBuffer = new char[size];

        assert(OurOwnedBuffer && "failed to allocate buffer for clipboard text");
        
        // Copy the data //
        memcpy(OurOwnedBuffer, buffer, size);

        // Start handling clipboard data requests //
        StartXMessageLoop();
        
	}

	virtual void retrieveFromClipboard(CEGUI::String& mimeType, void*& buffer, size_t& size){

        UNIQUE_LOCK_THIS_OBJECT();
        
        // We need to stop the message processing here, too //
        StopXMessageLoop();
        
        // Create a request //

        Atom targetproperty = XInternAtom(XDisplay, "CUT_BUFFER1", false);

        lockit.unlock();
        
        // We want the stuff in the clipboard //
        XConvertSelection(XDisplay, XA_CLIPBOARD(XDisplay), XA_STRING, targetproperty, ClipboardWindow, CurrentTime);
        XFlush(XDisplay);

        // Now we wait for the request to be completed //
        {
            boost::unique_lock<boost::recursive_mutex> lock(ClipboardRetrieveMutex);

            // We need to loop to run for it to process the response //
            StartXMessageLoop();
        
            WaitForClipboard.wait(lock);

            lockit.lock();
        
            if(!ClipboardRequestSucceeded){

                Logger::Get()->Info(L"The clipboard was empty");
                return;
            }
        }

        // We probably need to stop the processing loop again //
        StopXMessageLoop();

        // First read 0 bytes, to get the total size //
        Atom actualreturntype;
        int receivedformat;
        unsigned long receiveditems;
        unsigned long totalbytes;

        unsigned char* xbuffer;
        
        XGetWindowProperty(XDisplay, ClipboardWindow, targetproperty, 0, 0, false, XA_STRING, &actualreturntype,
            &receivedformat, &receiveditems, &totalbytes, &xbuffer);

        if(xbuffer)
            XFree(xbuffer);
        
        // All that is left to do is to retrieve the property text //
        // Might as well delete the data after this get
        XGetWindowProperty(XDisplay, ClipboardWindow, targetproperty, 0, totalbytes, true, XA_STRING, &actualreturntype,
            &receivedformat, &receiveditems, &totalbytes, &xbuffer);

        // Do last final checks on the data to make sure it is fine //
        if(receivedformat != 8){

            Logger::Get()->Warning(L"GuiClipboardHandler: received clipboard data is not 8 bit (one byte) aligned"
                L" chars, actual type: "+Convert::ToWstring(receivedformat));
            goto finishprocessingthing;
        }

        if(!xbuffer || receiveditems < 1){

            Logger::Get()->Warning(L"GuiClipboardHandler: received empty xbuffer from clipboard property");
            goto finishprocessingthing;
        }

        if(totalbytes != 0){

            Logger::Get()->Warning(L"GuiClipboardHandler: failed to retrieve whole clipboard, bytes left: "+
                Convert::ToWstring(totalbytes));
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

        Logger::Get()->Info(L"Succesfully retrieved text from clipboard, text is of length: "+
            Convert::ToWstring(ReceivedClipboardData.size()));

finishprocessingthing:
        
        // The buffer needs to be always released //
        if(xbuffer)
            XFree(xbuffer);

        // The message loop has to start again after all the Xlib calls //
        StartXMessageLoop();
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
    
    Leviathan::Window* HWNDSource;


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
                        Logger::Get()->Warning(L"GuiClipboardHandler: clipboard window shoulnd't become visible");
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
                        Logger::Get()->Info(L"Received the stop property");

                        
                    } else if(event.xproperty.atom == pasteresponse){
                        
                        Logger::Get()->Info(L"Received clipboard request's property");
                            
                        // We received the clipboard contents //
                        {
                            boost::unique_lock<boost::recursive_mutex> lock(ClipboardRetrieveMutex);

                            ClipboardRequestSucceeded = true;
                        }

                        WaitForClipboard.notify_all();
                    }

                }
                break;
                case SelectionRequest:
                {
                    // Prepare a response for the requester //
                    XEvent response;
                    GUARD_LOCK_THIS_OBJECT();

                    // Ignore if we got nothing //
                    if(OurOwnedBuffer){
                    
                        if((event.xselectionrequest.target == XA_STRING || (event.xselectionrequest.target ==
                                    XA_UTF8_STRING(XDisplay)) &&
                                event.xselectionrequest.selection == XA_CLIPBOARD(XDisplay)))
                        {

                            if(response.xselection.property == None){

                                Logger::Get()->Warning(L"Clipboard request property is None, "
                                    L"and we decided that CUT_BUFFER1 is a good choice");

                                // Let's just use this property //
                                response.xselection.property = XInternAtom(XDisplay, "CUT_BUFFER1", false);
                            }

                            Logger::Get()->Info(L"Sending clipboard text to requester");
                            
                            // Send the text to the requester //
                            XChangeProperty(XDisplay, event.xselectionrequest.requestor,
                                event.xselectionrequest.property,
                                XA_STRING, 8, PropModeReplace, reinterpret_cast<unsigned char*>(OurOwnedBuffer),
                                static_cast<int>(strlen(OurOwnedBuffer)));
                        
                            response.xselection.property = event.xselectionrequest.property;
                        
                        } else if(event.xselectionrequest.target == XA_TARGETS(XDisplay)
                            && event.xselectionrequest.selection == XA_CLIPBOARD(XDisplay)
                            && OurOwnedBuffer)
                        {
                            Logger::Get()->Info(L"Sending supported formats to clipboard requester");
                            
                            // We need to tell the requester what types are supported
                            Atom supported[] = {XA_UTF8_STRING(XDisplay), XA_STRING};
                        
                            XChangeProperty(XDisplay, event.xselectionrequest.requestor,
                                event.xselectionrequest.property,
                                XA_TARGETS(XDisplay), 32, PropModeReplace,
                                reinterpret_cast<unsigned char*>(&supported),
                                sizeof(supported)/sizeof(supported[0]));
                        
                        } else {

                            Logger::Get()->Info(L"Don't know how to respond to clipboard request");
                            response.xselection.property = None;
                        }
                    } else {

                        response.xselection.property = None;
                        Logger::Get()->Info(L"Clipboard is empty...");
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
                    Logger::Get()->Info(L"We now no longer own the clipboard");
                    SAFE_DELETE(OurOwnedBuffer);

                }
                break;
                case SelectionNotify:
                {
                    boost::unique_lock<boost::recursive_mutex> lock(ClipboardRetrieveMutex);
                    
                    // We received something from the selection owner //
                    if(event.xselection.property == None){

                        Logger::Get()->Info(L"Nothing to paste, clipboard empty");
                        ClipboardRequestSucceeded = false;
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
    void SetupClipboardWindow() THROWS{

        GUARD_LOCK_THIS_OBJECT();
        
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
        StartXMessageLoop();
    }

    //! Cleans up the window
    void CleanUpWindow(){
        
        GUARD_LOCK_THIS_OBJECT();

        // First stop the message loop //
        StopXMessageLoop();
        
        Logger::Get()->Info(L"GUI clipboard is ready to be destroyed");

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
    void StartXMessageLoop(){

        GUARD_LOCK_THIS_OBJECT();
        
        RunMessageProcessing = true;
        
        XMessageThread = boost::thread(&GuiClipboardHandler::RunXMessageLoop, this);
    }

    //! \brief Stops the message processing
    void StopXMessageLoop(){

        GUARD_LOCK_THIS_OBJECT();

        // First signal the loop to stop //
        RunMessageProcessing = false;

        // Then send an event for it to process... //
        unsigned char stopproperty[] = "stop";
        int count = 1;

        XChangeProperty(XDisplay, ClipboardWindow, XA_STRING, XA_STRING, 8,
            PropModeReplace, stopproperty, 5);

        XFlush(XDisplay);

        // Then wait for the message loop to end //
        XMessageThread.join();
    }
    
    // These are used for clipboard access //


    //! Holds the received text from the clipboard
    string ReceivedClipboardData;

    //! The current XDisplay
    Display* XDisplay;

    //! Our hidden clipboard window
    ::Window ClipboardWindow;
        
    boost::thread XMessageThread;

    //! Denotes whether the window loop should run
    bool RunMessageProcessing;

    //! This is waited for while the clipboard is accessed
    boost::condition_variable_any WaitForClipboard;

    //! This is a lock for clipboard content retrieve
    boost::recursive_mutex ClipboardRetrieveMutex;

    //! Denotes whether clipboard grap failed or succeeded
    bool ClipboardRequestSucceeded;
        
#endif

    

};



using namespace Leviathan;
using namespace Leviathan::Gui;

// ------------------ GuiManager ------------------ //
Leviathan::Gui::GuiManager::GuiManager() :
    ID(IDFactory::GetID()), Visible(true), GuiMouseUseUpdated(true), GuiDisallowMouseCapture(true),
    LastTimePulseTime(Misc::GetThreadSafeSteadyTimePoint()), MainGuiManager(false), ThisWindow(NULL), GuiContext(NULL),
    FileChangeID(0), _GuiClipboardHandler(NULL), ContextInput(NULL), DisableGuiMouseCapture(false)
{
	
}
Leviathan::Gui::GuiManager::~GuiManager(){

	
}
// ------------------------------------ //
bool Leviathan::Gui::GuiManager::Init(Graphics* graph, GraphicalInputEntity* window, bool ismain){
	GUARD_LOCK_THIS_OBJECT();

	ThisWindow = window;
    MainGuiManager = ismain;
	
	// Create the clipboard handler for this window (only one is required,
    // so only create if this is the main window's gui
    if(MainGuiManager){
        
        try{
            _GuiClipboardHandler = new GuiClipboardHandler(window->GetWindow());
            
        } catch(const Exception &e){

            // Clipboard isn't usable... //
            Logger::Get()->Warning(L"GuiManager: failed to create a ClipboardHandler: cannot copy or paste text, "
                L"exception:");
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
	SetMouseTheme(L"TaharezLook/MouseArrow");
	GuiContext->setDefaultTooltipType("TaharezLook/Tooltip");


	// Make the clipboard play nice //
	if(MainGuiManager == 1){

		// Only one clipboard is needed //
		if(_GuiClipboardHandler && _GuiClipboardHandler->WorksOnPlatform())
			CEGUI::System::getSingleton().getClipboard()->setNativeProvider(_GuiClipboardHandler);
	}


	// Store the initial time //
	LastTimePulseTime = Misc::GetThreadSafeSteadyTimePoint();


	return true;
}

void Leviathan::Gui::GuiManager::Release(){
	GUARD_LOCK_THIS_OBJECT();
	

	// Stop with the file updates //
	if(FileChangeID){


		auto tmphandler = ResourceRefreshHandler::Get();

		if(tmphandler){

			tmphandler->StopListeningForFileChanges(FileChangeID);
		}
        
		FileChangeID = 0;
	}

	// Default mouse back //
	SetMouseTheme(L"none");

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
        Logger::Get()->Info(L"GuiManager: main manager has detached the clipboard successfully");
    }
    
	SAFE_DELETE(_GuiClipboardHandler);

    Logger::Get()->Info(L"GuiManager: Gui successfully closed on window");
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::GuiManager::ProcessKeyDown(OIS::KeyCode key, int specialmodifiers){
	GUARD_LOCK_THIS_OBJECT();
	for(size_t i = 0; i < Collections.size(); i++){
		if(Collections[i]->GetTogglingKey().Match(key, specialmodifiers, false) && Collections[i]->GetAllowEnable()){
			// is a match, toggle //
			Collections[i]->ToggleState();

			return true;
		}
	}


	return false;
}

DLLEXPORT void Leviathan::Gui::GuiManager::SetCollectionStateProxy(string name, bool state){
	// call the actual function //
	SetCollectionState(Convert::StringToWstring(name), state);
}

DLLEXPORT void Leviathan::Gui::GuiManager::SetCollectionState(const wstring &name, bool state){
	GUARD_LOCK_THIS_OBJECT();
	// find collection with name and set it's state //
	for(size_t i = 0; i < Collections.size(); i++){
		if(Collections[i]->GetName() == name){
			// set state //
			if(Collections[i]->GetState() != state){
                
				Collections[i]->ToggleState();
			}
			return;
		}
	}
	// Complain //
	Logger::Get()->Warning(L"GuiManager: SetCollectionState: couldn't find a collection with name: "+name);
}

DLLEXPORT void Leviathan::Gui::GuiManager::SetCollectionAllowEnableState(const wstring &name, bool allow /*= true*/){
	GUARD_LOCK_THIS_OBJECT();
	// find collection with name and set it's allow enable state //
	for(size_t i = 0; i < Collections.size(); i++){
		if(Collections[i]->GetName() == name){
			// set state //
			if(Collections[i]->GetAllowEnable() != allow){
				Logger::Get()->Info(L"Setting Collection "+Collections[i]->GetName()+
                    L" allow enable state "+Convert::ToWstring(allow));
				Collections[i]->ToggleAllowEnable();
			}
			return;
		}
	}
}
// ------------------------------------ //
void Leviathan::Gui::GuiManager::GuiTick(int mspassed){
	GUARD_LOCK_THIS_OBJECT();

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
	GUARD_LOCK_THIS_OBJECT();

	// Pass time //
	auto newtime = Misc::GetThreadSafeSteadyTimePoint();
	
	SecondDuration elapsed = newtime-LastTimePulseTime;

	float changval = elapsed.count();

	GuiContext->injectTimePulse(changval);

	// Potentially pass to system //
	if(MainGuiManager){

		CEGUI::System::getSingleton().injectTimePulse(changval);
	}

	LastTimePulseTime = newtime;

	// Update inputs //
	ThisWindow->GetWindow()->GatherInput(ContextInput);

}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::GuiManager::OnResize(){
	GUARD_LOCK_THIS_OBJECT();

	// Notify the CEGUI system //
    // TODO: only to the wanted context
	CEGUI::System* const sys = CEGUI::System::getSingletonPtr();
	if(sys)
		sys->notifyDisplaySizeChanged(CEGUI::Sizef((float)ThisWindow->GetWindow()->GetWidth(),
                (float)ThisWindow->GetWindow()->GetHeight()));
}

DLLEXPORT void Leviathan::Gui::GuiManager::OnFocusChanged(bool focused){
	GUARD_LOCK_THIS_OBJECT();
	
	// Notify our context //
	if(!focused)
		ContextInput->injectMouseLeaves();

}
// ------------------------------------ //
bool Leviathan::Gui::GuiManager::AddGuiObject(BaseGuiObject* obj){
	GUARD_LOCK_THIS_OBJECT();
	Objects.push_back(obj);
	return true;
}

void Leviathan::Gui::GuiManager::DeleteObject(int id){
	GUARD_LOCK_THIS_OBJECT();
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
	GUARD_LOCK_THIS_OBJECT();
	for(size_t i = 0; i < Objects.size(); i++){
		if(Objects[i]->GetID() == id)
			return i;
	}
	return -1;
}

BaseGuiObject* Leviathan::Gui::GuiManager::GetObject(unsigned int index){
	GUARD_LOCK_THIS_OBJECT();
	ARR_INDEX_CHECK(index, Objects.size()){
		return Objects[index];
	}
	return NULL;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::GuiManager::LoadGUIFile(const wstring &file, bool nochangelistener /*= false*/,
    int iteration /*= 0*/)
{
    if(iteration > 10){

        Logger::Get()->Warning("GuiManager: aborting file load on iteration "+Convert::ToString(iteration));
        return false;
    }
    
	// Parse the file //
	auto data = ObjectFileProcessor::ProcessObjectFile(file);

	if(!data){
		return false;
	}

	MainGUIFile = file;

	NamedVars& varlist = *data->GetVariables();

	wstring relativepath;
    
	// Get path //
	ObjectFileProcessor::LoadValueFromNamedVars<wstring>(varlist, L"GUIBaseFile", relativepath, L"", true,
		L"GuiManager: LoadGUIFile: no base file defined (in "+file+L") : ");

    // This can be used to verify that CEGUI events are properly hooked //
    bool requireevent;

    ObjectFileProcessor::LoadValueFromNamedVars<bool>(varlist, L"RequireCEGUIHooked", requireevent, false, false, L"");

	if(!relativepath.size()){

		return false;
	}
	
	// Load it //
	CEGUI::Window* rootwindow = NULL;
	try{

		rootwindow = CEGUI::WindowManager::getSingleton().loadLayoutFromFile(Convert::WstringToString(relativepath));

	} catch(const Ogre::Exception &e){

		Logger::Get()->Error(L"GuiManager: LoadGUIFile: failed to locate file: "+relativepath+L":");
		Logger::Get()->Write(L"\t> "+Convert::StringToWstring(e.what()));
		return false;
	} catch(const CEGUI::GenericException &e2){

		Logger::Get()->Error(L"GuiManager: LoadGUIFile: failed to parse CEGUI layout: "+relativepath+L":");
		Logger::Get()->Write(L"\t> "+Convert::StringToWstring(e2.what()));
		return false;
	} catch(const CEGUI::InvalidRequestException &e3){

		Logger::Get()->Error(L"GuiManager: LoadGUIFile: failed to parse CEGUI layout: "+relativepath+L":");
		Logger::Get()->Write(L"\t> "+Convert::StringToWstring(e3.what()));
		return false;
	}

	// Check did it work //
	if(!rootwindow){

		Logger::Get()->Error(L"GuiManager: LoadGUIFile: failed to parse layout file: "+relativepath);
		return false;
	}

	// We need to lock now //
	GUARD_LOCK_THIS_OBJECT();



	// Look for animation files //
	auto animslist = varlist.GetValueDirectRaw(L"GUIAnimations");

	if(animslist){

		if(!animslist->CanAllBeCastedToType<wstring>()){

			Logger::Get()->Warning(L"GuiManager: LoadGUIFile: gui file has defined gui animation files in wrong format"
                L"(expected list of strings), file: "+relativepath);

		} else {

			// Load them //
			for(size_t i = 0; i < animslist->GetVariableCount(); i++){

				wstring curfile;
				animslist->GetValue(i).ConvertAndAssingToVariable<wstring>(curfile);

				// Check is the file already loaded //
				{
					ObjectLock gguard(GlobalGUIMutex);

					if(IsAnimationFileLoaded(gguard, curfile)){

						// Don't load again //
						continue;
					}

					// Set as loaded //
					SetAnimationFileLoaded(gguard, curfile);
				}

				try{

					CEGUI::AnimationManager::getSingleton().loadAnimationsFromXML(Convert::WstringToString(curfile));

				} catch(const Ogre::Exception &e){

					Logger::Get()->Warning(L"GuiManager: LoadGUIFile: failed to locate gui animation file: "+curfile+
                        L":");
					Logger::Get()->Write(L"\t> "+Convert::StringToWstring(e.what()));

				} catch(const CEGUI::GenericException &e2){

					Logger::Get()->Error(L"GuiManager: LoadGUIFile: failed to parse CEGUI animation file layout: "
                        +curfile+L":");
					Logger::Get()->Write(L"\t> "+Convert::StringToWstring(e2.what()));
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
		if(objecto->GetTypeName() == L"GuiCollection"){

			if(!GuiCollection::LoadCollection(this, *objecto)){

				// report error //
				Logger::Get()->Error(L"GuiManager: ExecuteGuiScript: failed to load collection, named "+
                    objecto->GetName());
				continue;
			}

			continue;

		} else if(objecto->GetTypeName() == L"GuiObject"){

			// try to load //
			if(!BaseGuiObject::LoadFromFileStructure(this, TempOs, *objecto)){

				// report error //
				Logger::Get()->Error(L"GuiManager: ExecuteGuiScript: failed to load GuiObject, named "+
                    objecto->GetName());
				continue;
			}

			continue;
		}

		Logger::Get()->Error(L"GuiManager: ExecuteGuiScript: Unrecognized type! typename: "+objecto->GetTypeName());
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
            Logger::Get()->Warning(L"GuiManager: while trying to load \""+file+L"\" RequireCEGUIHooked is true, "
                L"but no GUI object has any hooked CEGUI listeners, retrying load: ");

            UnLoadGUIFile();

            for(size_t i = 0; i < TempOs.size(); i++){
		
                TempOs[i]->ReleaseData();
                SAFE_RELEASE(TempOs[i]);
            }

            Logger::Get()->Write("\t> Doing iteration "+Convert::ToString(iteration+1));

            return LoadGUIFile(file, nochangelistener, iteration+1);
        }
    }
	

	for(size_t i = 0; i < TempOs.size(); i++){

		// add to real objects //
		AddGuiObject(TempOs[i]);
	}

	// This avoids having more and more change listeners each reload //
	if(!nochangelistener){
		// Listen for file changes //
		auto tmphandler = ResourceRefreshHandler::Get();
	
		if(tmphandler){

			// \todo Detect if the files are in different folders and start multiple listeners
			std::vector<const wstring*> targetfiles = boost::assign::list_of(&file)(&relativepath);

			tmphandler->ListenForFileChanges(targetfiles, boost::bind(&GuiManager::_FileChanged, this, _1, _2),
                FileChangeID);
		}
	}



	return true;
}

DLLEXPORT void Leviathan::Gui::GuiManager::UnLoadGUIFile(){

	GUARD_LOCK_THIS_OBJECT();

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
DLLEXPORT void Leviathan::Gui::GuiManager::SetMouseTheme(const wstring &tname){
	GUARD_LOCK_THIS_OBJECT();

	if(tname == L"none"){

		// show default window cursor //
		ThisWindow->GetWindow()->SetHideCursor(false);
		return;
	}

	// Set it active //
	GuiContext->getCursor().setDefaultImage(Convert::WstringToString(tname));

	

	// hide window cursor //
	ThisWindow->GetWindow()->SetHideCursor(true);
}

DLLEXPORT void Leviathan::Gui::GuiManager::SetMouseFileVisibleState(bool state){
	GUARD_LOCK_THIS_OBJECT();
	// Set mouse drawing flag //
}
// ------------------------------------ //
DLLEXPORT CEGUI::GUIContext* Leviathan::Gui::GuiManager::GetMainContext(){
	return GuiContext;
}
// ----------------- collection managing --------------------- //
void GuiManager::AddCollection(GuiCollection* add){
	GUARD_LOCK_THIS_OBJECT();
	Collections.push_back(add);
}

GuiCollection* Leviathan::Gui::GuiManager::GetCollection(const int &id, const wstring &name){
	GUARD_LOCK_THIS_OBJECT();
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
void Leviathan::Gui::GuiManager::_FileChanged(const wstring &file, ResourceFolderListener &caller){
	// Any updated file will cause whole reload //
	GUARD_LOCK_THIS_OBJECT();

	// Store the current state //
	auto currentstate = GetGuiStates();

	UnLoadGUIFile();

	// Now load it //
	if(!LoadGUIFile(MainGUIFile, true)){

		Logger::Get()->Error(L"GuiManager: file changed: couldn't load updated file: "+MainGUIFile);
	}

	// Apply back the old states //
	ApplyGuiStates(currentstate.get());

	// Mark everything as non-updated //
	caller.MarkAllAsNotUpdated();

}
// ------------------ Static part ------------------ //
std::vector<wstring> Leviathan::Gui::GuiManager::LoadedAnimationFiles;

boost::recursive_mutex Leviathan::Gui::GuiManager::GlobalGUIMutex;

bool Leviathan::Gui::GuiManager::IsAnimationFileLoaded(ObjectLock &lock, const wstring &file){
	assert(lock.owns_lock(&GlobalGUIMutex) && "Wrong object locked in GuiManager::IsAnimationFileLoaded");

	for(size_t i = 0; i < LoadedAnimationFiles.size(); i++){

		if(LoadedAnimationFiles[i] == file){

			return true;
		}
	}

	// Not found, must not be loaded then //
	return false;
}

void Leviathan::Gui::GuiManager::SetAnimationFileLoaded(ObjectLock &lock, const wstring &file){
	assert(lock.owns_lock(&GlobalGUIMutex) && "Wrong object locked in GuiManager::IsAnimationFileLoaded");

	LoadedAnimationFiles.push_back(file);
}

DLLEXPORT void Leviathan::Gui::GuiManager::KillGlobalCache(){
	ObjectLock guard(GlobalGUIMutex);

	// Release the memory to not look like a leak //
	LoadedAnimationFiles.clear();

	auto single = CEGUI::AnimationManager::getSingletonPtr();

	if(single)
		single->destroyAllAnimations();
}
// ------------------------------------ //
DLLEXPORT CEGUI::Window* Leviathan::Gui::GuiManager::GetWindowByStringName(const string &namepath){
	GUARD_LOCK_THIS_OBJECT();
	try{

		return GuiContext->getRootWindow()->getChild(namepath);

	} catch(const CEGUI::UnknownObjectException&){

		// Not found //
		return NULL;
	}
}

DLLEXPORT bool Leviathan::Gui::GuiManager::PlayAnimationOnWindow(const string &windowname, const string &animationname,
    bool applyrecursively, const string &ignoretypenames)
{
	// First get the window //
	auto wind = GetWindowByStringName(windowname);

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


	_PlayAnimationOnWindow(wind, animdefinition, applyrecursively, ignoretypenames);
	return true;
}

DLLEXPORT bool Leviathan::Gui::GuiManager::PlayAnimationOnWindowProxy(const string &windowname,
    const string &animationname)
{
	return PlayAnimationOnWindow(windowname, animationname);
}

void Leviathan::Gui::GuiManager::_PlayAnimationOnWindow(CEGUI::Window* targetwind, CEGUI::Animation* animdefinition,
    bool recurse, const string &ignoretypenames)
{
	// Apply only if the typename doesn't match ignored names //
	if(ignoretypenames.find(targetwind->getType().c_str()) == string::npos && targetwind->getName().at(0) != '_'){

		//Logger::Get()->Write(L"Animating thing: "+Convert::CharPtrToWstring(targetwind->getNamePath().c_str()));

		// Create an animation instance //
		CEGUI::AnimationInstance* createdanim = CEGUI::AnimationManager::getSingleton().instantiateAnimation(
            animdefinition);

		// Apply the instance //
		createdanim->setTargetWindow(targetwind);

		createdanim->start();
	}

	// Recurse to child elements if desired //
	if(recurse){

		// Find all child windows and call this method on them //
		for(size_t i = 0; i < targetwind->getChildCount(); i++){
			auto newtarget = targetwind->getChildAtIdx(i);

			_PlayAnimationOnWindow(newtarget, animdefinition, recurse, ignoretypenames);
		}
	}
}
// ------------------------------------ //
DLLEXPORT unique_ptr<GuiCollectionStates> Leviathan::Gui::GuiManager::GetGuiStates() const{
	GUARD_LOCK_THIS_OBJECT();
	// Create the result object using the size of Collections as all of them are added there //
	unique_ptr<GuiCollectionStates> result(new GuiCollectionStates(Collections.size()));

	// Add all the states and names of the collections //
	for(size_t i = 0; i < Collections.size(); i++){

		result->AddNewEntry(Collections[i]->GetName(), Collections[i]->GetState());
	}

	return result;
}

DLLEXPORT void Leviathan::Gui::GuiManager::ApplyGuiStates(const GuiCollectionStates* states){
	GUARD_LOCK_THIS_OBJECT();
	// Apply all the states from the object //
	for(size_t i = 0; i < states->CollectionNames.size(); i++){

		auto foundcollect = GetCollection(-1, *states->CollectionNames[i]->Name);

		// If found check whether the states match if not change to the right state //
		if(foundcollect && foundcollect->GetState() != states->CollectionNames[i]->IsEnabled){

			// Change the state //
			foundcollect->UpdateState(states->CollectionNames[i]->IsEnabled);
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
