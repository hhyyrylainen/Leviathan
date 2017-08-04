// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "CEGUIInclude.h"
#include "CEGUI/widgets/PushButton.h"

#include <memory>

namespace Leviathan{
namespace GUI{

class AlpaHitStoredTextureData;

//! \brief Implements a window type that handles hit detection by
//! image alpha channel
//! \note Any image used by this class is loaded into memory and won't be released until
//! the current AlphaHitCache is released
//! \todo This doesn't support scaled images. So
//! if that is needed the pixel check needs to be converted to use
//! percentages or a more complex approach for different CEGUI auto
//! scaling types
class AlphaHitButton : public CEGUI::PushButton{
public:

    AlphaHitButton(const CEGUI::String& type, const CEGUI::String& name);
    ~AlphaHitButton();

    //! The one overridden method that makes this work
    bool isHit(const glm::vec2 &position, const bool allow_disabled = false) const override;

    //! Window factory name.
    static const CEGUI::String WidgetTypeName;
    
protected:
    
    //! Once the texture and position in it has been determine it is stored here
    //!
    //! This is mutable because isHit has to be a const method
    mutable std::shared_ptr<AlpaHitStoredTextureData> HitTestTexture;
};


}
}
