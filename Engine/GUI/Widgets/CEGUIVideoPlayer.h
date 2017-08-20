// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "GUI/VideoPlayer.h"
#include "CEGUIInclude.h"
#include "CEGUI/Window.h"
#include "CEGUI/BitmapImage.h"

#include <atomic>

namespace Leviathan{
namespace GUI{

//! \brief Wraps a Leviathan::GUI::VideoPlayer to be used in a CEGUI widget
//!
//! This approach is based on how the version in Thrive used to work.
class CEGUIVideoPlayer : public CEGUI::Window{
public:

    CEGUIVideoPlayer(const CEGUI::String& type, const CEGUI::String& name);
    ~CEGUIVideoPlayer();
    

    //! \see Leviathan::GUI::VideoPlayer::Play
    bool Play(const std::string &videoFile);

    //! \see Leviathan::GUI::VideoPlayer::Stop
    void Stop();

    //! \see Leviathan::GUI::VideoPlayer::GetCurrentTime
    float GetCurrentTime() const;
    
    
    //! Window factory name.
    static const CEGUI::String WidgetTypeName;

protected:
    
    VideoPlayer Player;

    //! Number in the name of materials created by this. Taken from ImageNumber to be unique
    const int InstanceNumber;

    // These are for connecting the output dynamic texture of our
    // VideoPlayer to the CEGUI Image
    //! This is kept in order to delete the material when it is no longer used
    Ogre::MaterialPtr VideoMaterial;
    Ogre::Pass* VideoMaterialPass = nullptr;
    Ogre::TextureUnitState* VideoMaterialTextureUnit = nullptr;

    //! Reference to our created CEGUI image
    CEGUI::BitmapImage* CEGUISideImage = nullptr;

    //! Reference to our CEGUI side CEGUI::OgreRenderer texture. Used
    //! to delete it after we are done
    CEGUI::Texture* CEGUISideTexture = nullptr;
    

    //! Name of our CEGUISideImage, needed to destroy it properly later
    const std::string ImagePropertyName;

    static std::atomic<int> ImageNumber;
};

}
}


