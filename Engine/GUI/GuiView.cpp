#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUIVIEW
#include "GuiView.h"
#endif
#include "OgreTextureManager.h"
#include "Window.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreMaterialManager.h"
#include "OgreSceneManager.h"
#include "OgreManualObject.h"
#include "GlobalCEFHandler.h"
#include "include/cef_browser.h"
#include "Exceptions/ExceptionNotFound.h"
#include "Threading/ThreadingManager.h"
#include "LeviathanJavaScriptAsync.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
using namespace Leviathan;
using namespace Leviathan::Gui;
// ------------------------------------ //
DLLEXPORT Leviathan::Gui::View::View(GuiManager* owner, Window* window, VIEW_SECURITYLEVEL security /*= VIEW_SECURITYLEVEL_ACCESS_ALL*/) : Wind(window), 
	Owner(owner), ID(IDFactory::GetID()), CEFOverlayQuad(NULL),	CEFSNode(NULL), OurFocus(false), ViewSecurity(security), CanPaint(false),
	RenderHolderForMain(new RenderDataHolder()), RenderHolderForPopUp(new RenderDataHolder()), OurBrowserSide(NULL), OurAPIHandler(new LeviathanJavaScriptAsync(this))
{

}

DLLEXPORT Leviathan::Gui::View::~View(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::View::Init(const wstring &filetoload, const NamedVars &headervars){
	// Lock us //
	GUARD_LOCK_THIS_OBJECT();

	// Create the Ogre texture and material first //

	MaterialName = "_ChromeOverlay_for_gui_"+Convert::ToString(ID);
	TextureName = "_texture_"+MaterialName;


	// Create a material and a texture that we can update //
	Texture = Ogre::TextureManager::getSingleton().createManual(TextureName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		Ogre::TEX_TYPE_2D, Wind->GetWidth(), Wind->GetHeight(), 0, Ogre::PF_B8G8R8A8, Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

	// Fill in some test data //
	Ogre::HardwarePixelBufferSharedPtr pixelbuf = Texture->getBuffer();

	// Lock buffer and get a target box for writing //
	pixelbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD);
	const Ogre::PixelBox& pixelbox = pixelbuf->getCurrentLock();

	// Create a pointer to the destination //
	UCHAR* destptr = static_cast<UCHAR*>(pixelbox.data);

	// Fill it with data //
	for(size_t j = 0; j < pixelbox.getHeight(); j++){
		for(size_t i = 0; i < pixelbox.getWidth(); i++){
			// Set it completely empty //
			*destptr++ = 0;
			*destptr++ = 0;
			*destptr++ = 0;
			*destptr++ = 0;
		}

		destptr += pixelbox.getRowSkip() * Ogre::PixelUtil::getNumElemBytes(pixelbox.format);
	}

	// Unlock the buffer //
	pixelbuf->unlock();

	// Create the material //
	Material = Ogre::MaterialManager::getSingleton().create(MaterialName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	Material->getTechnique(0)->getPass(0)->createTextureUnitState(TextureName);
	Material->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
	Material->getTechnique(0)->getPass(0)->setLightingEnabled(false);


	// Create a full screen quad for chrome render result displaying //
	CEFOverlayQuad = Wind->GetOverlayScene()->createManualObject();
	CEFOverlayQuad->setName("GUI_chrome_quad_"+Convert::ToString(ID));


	// Use identity view/projection matrices for 2d rendering //
	CEFOverlayQuad->setUseIdentityProjection(true);
	CEFOverlayQuad->setUseIdentityView(true);

	// Hopefully a triangle strip saves some performance //
	CEFOverlayQuad->begin(MaterialName, Ogre::RenderOperation::OT_TRIANGLE_STRIP);

	CEFOverlayQuad->position(-1.f, -1.f, 0.0f);
	CEFOverlayQuad->textureCoord(Ogre::Vector2(0.f, 1.f));
	CEFOverlayQuad->position( 1.f, -1.f, 0.0f);
	CEFOverlayQuad->textureCoord(Ogre::Vector2(1.f, 1.f));
	CEFOverlayQuad->position( 1.f,  1.f, 0.0f);
	CEFOverlayQuad->textureCoord(Ogre::Vector2(1.f, 0.f));
	CEFOverlayQuad->position(-1.f,  1.f, 0.0f);
	CEFOverlayQuad->textureCoord(Ogre::Vector2(0.f, 0.f));

	CEFOverlayQuad->index(0);
	CEFOverlayQuad->index(1);
	CEFOverlayQuad->index(3);
	CEFOverlayQuad->index(2);

	CEFOverlayQuad->end();

	// We can use infinite AAB to not get culled //
	CEFOverlayQuad->setLocalAabb(Ogre::Aabb::BOX_INFINITE);

	// Render just before overlays
	CEFOverlayQuad->setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY);

	// Attach to scene
	CEFSNode = Wind->GetOverlayScene()->getRootSceneNode()->createChildSceneNode();
	CEFSNode->attachObject(CEFOverlayQuad);

	// Now we can create the browser //
	CefWindowInfo info;

	// We use our own window things //
	info.SetAsOffScreen(Wind->GetHandle());
	// We want the browsers to be able to be transparent //
	info.SetTransparentPainting(true);

	// Customize the settings //
	CefBrowserSettings settings;
	
	// Created asynchronously, the pointer will be linked in the OnAfterCreated function //
	// loads google just for fun //
	CefBrowserHost::CreateBrowser(info, this, filetoload, settings, NULL);

	return true;
}

DLLEXPORT void Leviathan::Gui::View::ReleaseResources(){

	// Stop all events //
	UnRegisterAllEvents();

	// Lock us //
	GUARD_LOCK_THIS_OBJECT();

	// Kill the javascript async //
	OurAPIHandler->BeforeRelease();

	// Destroy the browser first //


	// Force release so nothing can stop it //
	if(OurBrowser.get()){
		OurBrowser->GetHost()->CloseBrowser(true);
	}

	// Release our objects //
	RenderHolderForMain.reset();
	RenderHolderForPopUp.reset();

	SAFE_DELETE(OurAPIHandler);

	// We could leave the Ogre resources hanging, but it might be a good idea to release them right now and not wait for the program to exit //
	// TODO: don't use names here and make it work somehow //
	Ogre::TextureManager::getSingleton().remove(Texture->getName());
	Ogre::MaterialManager::getSingleton().remove(Material->getName());

	Material.setNull();
	Texture.setNull();

	// Destroy the manual object //
	CEFSNode->removeAndDestroyAllChildren();

	CEFOverlayQuad = NULL;
	CEFSNode = NULL;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::View::NotifyWindowResized(){
	if(OurBrowser.get()){

		OurBrowser->GetHost()->WasResized();
	}
}

DLLEXPORT void Leviathan::Gui::View::NotifyFocusUpdate(bool focused){
	// Update our focus //
	OurFocus = focused;

	if(OurBrowser.get()){

		OurBrowser->GetHost()->SendFocusEvent(focused);
	}
}
// ------------------------------------ //
bool Leviathan::Gui::View::GetRootScreenRect(CefRefPtr<CefBrowser> browser, CefRect& rect){
	try{
		// Get the rect //
		Int4 recti = Wind->GetScreenPixelRect();

		// Fill the rect //
		rect.x = recti.X;
		rect.y = recti.Y;
		rect.width = recti.Z;
		rect.height = recti.W;

		return true;

	} catch(ExceptionNotFound){
		return false;
	}
}

bool Leviathan::Gui::View::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect){
	// This should always work //
	rect.x = rect.y = 0;

	// Width and height can be retrieved from the window //
	rect.width = Wind->GetWidth();
	rect.height = Wind->GetHeight();

	return true;
}

bool Leviathan::Gui::View::GetScreenPoint(CefRefPtr<CefBrowser> browser, int viewX, int viewY, int& screenX, int& screenY){
	try{
		// Get the point //
		Int2 point = Wind->TranslateClientPointToScreenPoint(Int2(viewX, viewY));

		screenX = point.X;
		screenY = point.Y;

		return true;

	} catch(ExceptionNotFound){
		return false;
	}
}

bool Leviathan::Gui::View::GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info){
	return false;
}

void Leviathan::Gui::View::OnPopupShow(CefRefPtr<CefBrowser> browser, bool show){

}

void Leviathan::Gui::View::OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect){
	
}

void Leviathan::Gui::View::OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor){
#ifdef _WIN32

	HWND hwnd = Wind->GetHandle();

	if(!hwnd)
		return;

	// Should not do this, but whatever //
	// TODO: custom cursors
	SetClassLongPtr(hwnd, GCLP_HCURSOR,
		static_cast<LONG>(reinterpret_cast<LONG_PTR>(cursor)));
	SetCursor(cursor);

#endif // _WIN32


}

void Leviathan::Gui::View::OnScrollOffsetChanged(CefRefPtr<CefBrowser> browser){
	
}
// ------------------------------------ //
void Leviathan::Gui::View::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height){
	// Seems like we need to wait for main thread to handle this //

	// Calculate the size of the buffer //
	size_t newbufsize = width*height*Ogre::PixelUtil::getNumElemBytes(Ogre::PF_B8G8R8A8);

	// Lock us, just for fun //
	GUARD_LOCK_THIS_OBJECT();

	RenderDataHolder* ptrtotarget;

	switch(type){
	case PET_POPUP:
		{
			ptrtotarget = RenderHolderForPopUp.get();
			ptrtotarget->Type = PET_POPUP;
		}
		break;
	case PET_VIEW:
		{
			ptrtotarget = RenderHolderForMain.get();
			ptrtotarget->Type = PET_VIEW;
		}
		break;
	}

	// We need to allocate a new buffer if it isn't the same size //
	if(newbufsize != ptrtotarget->BufferSize){
		// Delete the old buffer //
		SAFE_DELETE(ptrtotarget->Buffer);

		// Set new size //
		ptrtotarget->BufferSize = newbufsize;

		ptrtotarget->Buffer = new char[ptrtotarget->BufferSize];
	}

	// Set data //
	ptrtotarget->Width = width;
	ptrtotarget->Height = height;

	// Mark as updated //
	ptrtotarget->Updated = true;

	// We probably need to copy the buffer over //
	memcpy(ptrtotarget->Buffer, buffer, ptrtotarget->BufferSize);
}

DLLEXPORT void Leviathan::Gui::View::CheckRender(){
	// Lock us //
	GUARD_LOCK_THIS_OBJECT();

	// Update all that are needed //
	for(int i = 0; i < 2; i++){
		RenderDataHolder* ptrtotarget;
		if(i == 0)
			ptrtotarget = RenderHolderForMain.get();
		else if(i == 1)
			ptrtotarget = RenderHolderForPopUp.get();

		// Check and update the texture //
		if(!ptrtotarget->Updated){
			// No need to update //
			continue;
		}

		// Get the right target //
		Ogre::TexturePtr targettexture;

		switch(i){
		case 0:
			{
				targettexture = Texture;
			}
			break;
		case 1:
			{
				// We don't know how to handle this //
				continue;
			}
			break;
		}


		// Make sure our texture is large enough //
		if(targettexture->getWidth() != ptrtotarget->Width || targettexture->getHeight() != ptrtotarget->Height){
			// Free resources and then change the size //
			targettexture->freeInternalResources();
			targettexture->setWidth(ptrtotarget->Width);
			targettexture->setHeight(ptrtotarget->Height);
			targettexture->createInternalResources();

			Logger::Get()->Info(L"GuiView: recreated texture for CEF browser");
		}

		// Copy it to our texture buffer //
		Ogre::HardwarePixelBufferSharedPtr pixelbuf = targettexture->getBuffer();

		// Lock buffer and get a target box for writing //
		pixelbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD);
		const Ogre::PixelBox& pixelbox = pixelbuf->getCurrentLock();

		// Copy out the pointer //
		void* destptr = pixelbox.data;

		// Copy the data over //
		memcpy(destptr, ptrtotarget->Buffer, ptrtotarget->BufferSize);

		// Unlock the buffer //
		pixelbuf->unlock();

		// Mark as no longer needs updating //
		ptrtotarget->Updated = false;
	}
}
// ------------------------------------ //
CefRefPtr<CefRenderHandler> Leviathan::Gui::View::GetRenderHandler(){
	return this;
}

CefRefPtr<CefRequestHandler> Leviathan::Gui::View::GetRequestHandler(){
	return this;
}

CefRefPtr<CefLoadHandler> Leviathan::Gui::View::GetLoadHandler(){
	return this;
}

CefRefPtr<CefLifeSpanHandler> Leviathan::Gui::View::GetLifeSpanHandler(){
	return this;
}

CefRefPtr<CefKeyboardHandler> Leviathan::Gui::View::GetKeyboardHandler(){
	return this;
}

CefRefPtr<CefGeolocationHandler> Leviathan::Gui::View::GetGeolocationHandler(){
	return this;
}

CefRefPtr<CefDragHandler> Leviathan::Gui::View::GetDragHandler(){
	return this;
}

CefRefPtr<CefDownloadHandler> Leviathan::Gui::View::GetDownloadHandler(){
	return this;
}

CefRefPtr<CefDisplayHandler> Leviathan::Gui::View::GetDisplayHandler(){
	return this;
}

CefRefPtr<CefContextMenuHandler> Leviathan::Gui::View::GetContextMenuHandler(){
	return this;
}
// ------------------------------------ //
void Leviathan::Gui::View::OnProtocolExecution(CefRefPtr<CefBrowser> browser, const CefString& url, bool& allow_os_execution){
	// Handle execution of external protocols...
}

void Leviathan::Gui::View::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus status){
	// A render process has crashed...
	OurBrowserSide->OnRenderProcessTerminated(browser);
}

void Leviathan::Gui::View::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl){
	// A frame has failed to load content...
}

void Leviathan::Gui::View::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode){
	// A frame has finished loading content...

	// Let's try fix some focusing issues //
	if(frame->IsMain()){
		// Store our original focus //
		bool origfocus = OurFocus;
		// Lose focus and then gain it if we have focus //
		NotifyFocusUpdate(false);
		if(origfocus)
			NotifyFocusUpdate(true);
	}
}

void Leviathan::Gui::View::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame){
	// A frame has started loading content...
}

void Leviathan::Gui::View::OnBeforeClose(CefRefPtr<CefBrowser> browser){
	// Browser window is closed, perform cleanup...
	OurBrowser = NULL;

	OurBrowserSide->OnBeforeClose(browser);

}

void Leviathan::Gui::View::OnAfterCreated(CefRefPtr<CefBrowser> browser){
	// Browser window created successfully...
	OurBrowser = browser;

	// Create messaging functionality //
	CefMessageRouterConfig config;
	config.js_query_function = "cefQuery";
	config.js_cancel_function = "cefQueryCancel";

	OurBrowserSide = CefMessageRouterBrowserSide::Create(config);

	OurBrowserSide->AddHandler(OurAPIHandler, true);

}

bool Leviathan::Gui::View::OnBeforePopup(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& target_url, const CefString& target_frame_name, const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo, CefRefPtr<CefClient>& client, CefBrowserSettings& settings, bool* no_javascript_access){
	// Allow or block popup windows, customize popup window creation...
	return false;
}

void Leviathan::Gui::View::OnRequestGeolocationPermission(CefRefPtr<CefBrowser> browser, const CefString& requesting_url, int request_id, CefRefPtr<CefGeolocationCallback> callback){
	// Allow or deny geolocation API access...
}

void Leviathan::Gui::View::OnDownloadUpdated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, CefRefPtr<CefDownloadItemCallback> callback){
	// Update the download status...
}

void Leviathan::Gui::View::OnBeforeDownload(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, const CefString& suggested_name, CefRefPtr<CefBeforeDownloadCallback> callback){
	// Specify a file path or cancel the download...
}

bool Leviathan::Gui::View::OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line){
	// Log a console message...
	// Allow passing to default console //
	Logger::Get()->Write(L"[CEF] "+wstring(message)+L". \n\tIn: "+wstring(source)+L" ("+Convert::ToWstring(line)+L")");
	return false;
}

void Leviathan::Gui::View::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title){
	// Update the browser window title...
}

void Leviathan::Gui::View::OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url){
	// Update the URL in the address bar...
}

void Leviathan::Gui::View::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward){
	// Update UI for browser state...
}
// ------------------------------------ //
bool Leviathan::Gui::View::OnKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent& event, CefEventHandle os_event){
	// Notify the window about this //
	if(Wind)
		Wind->ReportKeyEventAsUsed();
	return false;
}

DLLEXPORT CefRefPtr<CefBrowserHost> Leviathan::Gui::View::GetBrowserHost(){
	if(OurBrowser.get()){

		return OurBrowser->GetHost();
	}

	return NULL;
}

DLLEXPORT void Leviathan::Gui::View::SetAllowPaintStatus(bool canpaintnow){
	if(CanPaint == canpaintnow)
		return;
	// Update //
	CanPaint = canpaintnow;

	if(CanPaint){
		// Mark the whole area as dirty //
		// This might be a better way to do this //
		//OurBrowser->GetHost()->WasResized();
	}
}

bool Leviathan::Gui::View::OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool is_redirect){

	OurBrowserSide->OnBeforeBrowse(browser, frame);
	return false;
}

bool Leviathan::Gui::View::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request){
	return false;
}

CefRefPtr<CefResourceHandler> Leviathan::Gui::View::GetResourceHandler(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request){
	return NULL;
}

void Leviathan::Gui::View::OnResourceRedirect(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& old_url, CefString& new_url){

}

bool Leviathan::Gui::View::GetAuthCredentials(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, bool isProxy, const CefString& host, int port, const CefString& realm, const CefString& scheme, CefRefPtr<CefAuthCallback> callback){
	return false;
}

bool Leviathan::Gui::View::OnQuotaRequest(CefRefPtr<CefBrowser> browser, const CefString& origin_url, int64 new_size, CefRefPtr<CefQuotaCallback> callback){
	return false;
}

bool Leviathan::Gui::View::OnCertificateError(cef_errorcode_t cert_error, const CefString& request_url, CefRefPtr<CefAllowCertificateErrorCallback> callback){
	return false;
}

bool Leviathan::Gui::View::OnBeforePluginLoad(CefRefPtr<CefBrowser> browser, const CefString& url, const CefString& policy_url, CefRefPtr<CefWebPluginInfo> info){
	return false;
}

void Leviathan::Gui::View::OnPluginCrashed(CefRefPtr<CefBrowser> browser, const CefString& plugin_path){

}
// ------------------------------------ //
bool Leviathan::Gui::View::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message){
	// Handle IPC messages from the render process...
	if(OurBrowserSide->OnProcessMessageReceived(browser, source_process, message))
		return true;
	if(_PMCheckIsEvent(message))
		return true;


	// Not handled //
	return false;
}

bool Leviathan::Gui::View::_PMCheckIsEvent(CefRefPtr<CefProcessMessage> &message){
	// Check does name match something //
	if(message->GetName() == "LGeneric"){
		// Get the name of the event //
		const wstring toregister = message->GetArgumentList()->GetString(0);

		// Only add if not already added //
		auto iter = RegisteredGenerics.find(toregister);

		if(iter == RegisteredGenerics.end()){
			// Add it //
			RegisterForEvent(toregister);
		}

		return true;

	} else if(message->GetName() == "LEvent"){
		// Get the event that we need to register for //
		EVENT_TYPE toregister = static_cast<EVENT_TYPE>(message->GetArgumentList()->GetInt(0));

		// Only add if not already added //
		auto iter = RegisteredEvents.find(toregister);

		if(iter == RegisteredEvents.end()){
			// Add it //
			RegisterForEvent(toregister);
		}


		return true;

	} else if(message->GetName() == "LEvents"){

		// This always means "unregister all" //
		UnRegisterAllEvents();
		return true;
	}


	// Not handled //
	return false;
}

DLLEXPORT int Leviathan::Gui::View::OnEvent(Event** pEvent){
	// Serialize it to a packet //
	sf::Packet tmppacket;

	(*pEvent)->AddDataToPacket(tmppacket);

	tmppacket.getData();

	// Create the message //
	CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("OnEvent");

	CefRefPtr<CefListValue> args = message->GetArgumentList();
	// Set the packet as binary data //
	args->SetBinary(0, CefBinaryValue::Create(tmppacket.getData(), tmppacket.getDataSize()));

	OurBrowser->SendProcessMessage(PID_RENDERER, message);

	return 3;
}

DLLEXPORT int Leviathan::Gui::View::OnGenericEvent(GenericEvent** pevent){
	// Serialize it to a packet //
	sf::Packet tmppacket;

	(*pevent)->AddDataToPacket(tmppacket);

	tmppacket.getData();

	// Create the message //
	CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("OnGeneric");

	CefRefPtr<CefListValue> args = message->GetArgumentList();
	// Set the packet as binary data //
	args->SetBinary(0, CefBinaryValue::Create(tmppacket.getData(), tmppacket.getDataSize()));

	OurBrowser->SendProcessMessage(PID_RENDERER, message);

	return 3;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::View::ToggleElement(const string &name){
	auto frame = OurBrowser->GetMainFrame();
	frame->ExecuteJavaScript("$(\""+name+"\").toggle()", frame->GetURL(), 0);
}


