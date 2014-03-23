#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUIVIEW
#include "GuiView.h"
#endif
#include "OgreTextureManager.h"
#include "Common/Window.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreMaterialManager.h"
#include "OgreSceneManager.h"
#include "OgreManualObject.h"
#include "GlobalCEFHandler.h"
#include "include/cef_browser.h"
#include "Exceptions/ExceptionNotFound.h"
#include "Threading/ThreadingManager.h"
using namespace Leviathan;
using namespace Leviathan::Gui;
// ------------------------------------ //
DLLEXPORT Leviathan::Gui::View::View(GuiManager* owner, Window* window, VIEW_SECURITYLEVEL security /*= VIEW_SECURITYLEVEL_ACCESS_ALL*/) : Wind(window), 
	Owner(owner), ID(IDFactory::GetID()), CEFOverlayQuad(NULL),	CEFSNode(NULL), OurFocus(false), ViewSecurity(security), CanPaint(false),
	TextureToCopy(new RenderDataHolder(this))
{

}

DLLEXPORT Leviathan::Gui::View::~View(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::View::Init(const wstring &filetoload, const NamedVars &headervars){
	// Lock our object //
	ObjectLock guard1(*TextureToCopy.get());
	
	// Lock us //
	ObjectLock guard(*this);

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
	CEFOverlayQuad = Wind->GetOverlayScene()->createManualObject("GUI_chrome_quad_"+Convert::ToString(ID));

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
	Ogre::AxisAlignedBox aabInf;
	aabInf.setInfinite();
	CEFOverlayQuad->setBoundingBox(aabInf);

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

	// It's now valid //
	TextureToCopy->IsStillValid = true;

	return true;
}

DLLEXPORT void Leviathan::Gui::View::ReleaseResources(){
	// Lock our object //
	ObjectLock guard1(*TextureToCopy.get());

	// It's no longer valid //
	TextureToCopy->IsStillValid = false;

	// Lock us //
	ObjectLock guard(*this);

	// Destroy the browser first //

	// Force release so nothing can stop it //
	if(OurBrowser.get()){
		OurBrowser->GetHost()->CloseBrowser(true);
	}

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
	// Seems like we need to pass this to another thread for handling //

	// Calculate the size of the buffer //
	size_t newbufsize = width*height*Ogre::PixelUtil::getNumElemBytes(Ogre::PF_B8G8R8A8);

	{
		// Lock the buffer //
		ObjectLock guard1(*TextureToCopy.get());

		// Lock us, just for fun //
		ObjectLock guard(*this);

		// We need to allocate a new buffer if it isn't the same size //
		if(newbufsize != TextureToCopy->BufferSize){
			// Delete the old buffer //
			SAFE_DELETE(TextureToCopy->Buffer);

			// Set new size //
			TextureToCopy->BufferSize = newbufsize;

			TextureToCopy->Buffer = new char[TextureToCopy->BufferSize];
		}

		// Set data //
		TextureToCopy->Type = type;
		TextureToCopy->Width = width;
		TextureToCopy->Height = height;

		// We probably need to copy the buffer over //
		memcpy(TextureToCopy->Buffer, buffer, TextureToCopy->BufferSize);
	}

	// Queue the task //
	ThreadingManager::Get()->QueueTask(shared_ptr<QueuedTask>(new QueuedTask(boost::bind<void>([](shared_ptr<RenderDataHolder> dataobj){

		// Lock it  //
		ObjectLock guard1(*dataobj.get());

		// Check is it still valid //
		if(!dataobj->IsStillValid)
			return;

		// Lock the object itself //
		ObjectLock guard2(*dataobj->MyView);

		// Copy it to the texture //
		if(dataobj->Type == PET_POPUP){
			// We don't know how to paint this //

			return;
		}

		// Make sure our texture is large enough //
		if(dataobj->MyView->Texture->getWidth() != dataobj->Width || dataobj->MyView->Texture->getHeight() != dataobj->Height){
			// Free resources and then change the size //
			dataobj->MyView->Texture->freeInternalResources();
			dataobj->MyView->Texture->setWidth(dataobj->Width);
			dataobj->MyView->Texture->setHeight(dataobj->Height);
			dataobj->MyView->Texture->createInternalResources();

			Logger::Get()->Info(L"GuiView: recreated texture for CEF browser");
		}

		// Copy it to our texture buffer //
		Ogre::HardwarePixelBufferSharedPtr pixelbuf = dataobj->MyView->Texture->getBuffer();

		// Lock buffer and get a target box for writing //
		pixelbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD);
		const Ogre::PixelBox& pixelbox = pixelbuf->getCurrentLock();

		// Copy out the pointer //
		void* destptr = pixelbox.data;

		// Copy the data over //
		memcpy(destptr, dataobj->Buffer, dataobj->BufferSize);

		// Unlock the buffer //
		pixelbuf->unlock();





	}, TextureToCopy))));
}

DLLEXPORT void Leviathan::Gui::View::CheckRender(){

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
}

void Leviathan::Gui::View::OnAfterCreated(CefRefPtr<CefBrowser> browser){
	// Browser window created successfully...
	OurBrowser = browser;
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
	Logger::Get()->Write(L"[CEF] "+wstring(message)+L"\n");
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


