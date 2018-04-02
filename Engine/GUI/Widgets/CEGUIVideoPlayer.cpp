// ------------------------------------ //
#include "CEGUIVideoPlayer.h"

#include "OgreTextureManager.h"
#include "OgreMaterialManager.h"
#include "OgreMaterial.h"
#include "OgreTechnique.h"
#include "OgrePass.h"

#include "CEGUI/ImageManager.h"
#include "CEGUI/RendererModules/Ogre/Texture.h"

using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //

CEGUIVideoPlayer::CEGUIVideoPlayer(const CEGUI::String& type, const CEGUI::String& name) :
    CEGUI::Window(type, name), InstanceNumber(ImageNumber++),
    ImagePropertyName("Leviathan/GeneratedVideo_" + std::to_string(InstanceNumber))
{
    VideoMaterial = Ogre::MaterialManager::getSingleton().create("CEGUIVideoMaterial_" +
        std::to_string(InstanceNumber),
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    VideoMaterialPass = VideoMaterial->getTechnique(0)->getPass(0);
    VideoMaterialTextureUnit = VideoMaterialPass->createTextureUnitState();    

    CEGUISideImage = static_cast<CEGUI::BitmapImage*>(
        &CEGUI::ImageManager::getSingleton().create(
            "BitmapImage", ImagePropertyName));
}

CEGUIVideoPlayer::~CEGUIVideoPlayer(){

    Stop();

    if(CEGUISideImage){
        CEGUI::ImageManager::getSingleton().destroy(*CEGUISideImage);
        CEGUISideImage = nullptr;
    }    

    VideoMaterialTextureUnit = nullptr;
    VideoMaterialPass = nullptr;

    if(VideoMaterial){
        Ogre::MaterialManager::getSingleton().remove(VideoMaterial);
        VideoMaterial.reset();
    }
}
    
const CEGUI::String CEGUIVideoPlayer::WidgetTypeName = "Leviathan/VideoPlayer";
std::atomic<int> CEGUIVideoPlayer::ImageNumber = {0};
// ------------------------------------ //
bool CEGUIVideoPlayer::Play(const std::string &videoFile){

    if(!Player.Play(videoFile))
        return false;

    // If we need to disable shadows this might be the way to do it
    //VideoMaterial->setMacroblock(const Ogre::HlmsMacroblock &macroblock)
    VideoMaterialTextureUnit->setTextureName(Player.GetTextureName());

    CEGUI::Texture& texture = CEGUI::System::getSingleton().getRenderer()->createTexture(
        "VideoPlayer_CEGUISideTexture__" + std::to_string(InstanceNumber));

    CEGUISideTexture = &texture;

    CEGUI::OgreTexture& rendererTexture = static_cast<CEGUI::OgreTexture&>(texture);

    rendererTexture.setOgreTexture(Ogre::TextureManager::getSingleton().getByName(
            Player.GetTextureName()), false);

    CEGUI::OgreRenderer* ogreRenderer = static_cast<CEGUI::OgreRenderer*>(
        CEGUI::System::getSingleton().getRenderer());

    // Set the area of the new CEGUI texture //
    CEGUI::Rectf imageArea;
    int videoWidth = Player.GetVideoWidth();
    int videoHeight = Player.GetVideoHeight();

    const bool isTextureTargetVerticallyFlipped = ogreRenderer->isTexCoordSystemFlipped();    

    if(isTextureTargetVerticallyFlipped){
        imageArea = CEGUI::Rectf(0.0f, videoWidth, videoHeight, 0.0f);
    } else {
        imageArea = CEGUI::Rectf(0.0f, 0.0f, videoWidth, videoHeight);
    }
    
    CEGUISideImage->setImageArea(imageArea);

    // We don't want full autoscaling for the texture so we disable the scaling
    // TODO: we may want to have height or width based scaling that respects aspect ratio
    // This option may not work correctly if this is inside a Generic/Image widget
    CEGUISideImage->setAutoScaled(CEGUI::AutoScaledMode::Disabled);
    CEGUISideImage->setTexture(&rendererTexture);

    // Attach the output image to our drawing material //
    setProperty("Image", ImagePropertyName);    

    return true;
}

void CEGUIVideoPlayer::Stop(){

    if(CEGUISideTexture){
        CEGUI::System::getSingleton().getRenderer()->destroyTexture(*CEGUISideTexture);
        CEGUISideTexture = nullptr;
    }

    CEGUISideImage->setTexture(nullptr);

    Player.Stop();
}
// ------------------------------------ //
float CEGUIVideoPlayer::GetCurrentTime() const{

    return Player.GetCurrentTime();
}
// ------------------------------------ //
Delegate* CEGUIVideoPlayer::GetOnPlaybackEnded(){

    Player.OnPlayBackEnded.AddRef();
    return &Player.OnPlayBackEnded;
}


