// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Widget.h"

namespace Leviathan { namespace GUI {

//! \brief A simple image showing widget
class ImageWidget : public WidgetWithStandardResources {
protected:
    // These are protected for only constructing properly reference
    // counted instances through MakeShared
    friend ReferenceCounted;
    DLLEXPORT ImageWidget(const std::string& image);

public:
    DLLEXPORT ~ImageWidget();

    //! \brief Sets the image shown by this widget
    //! \todo Implement this for when this is already shown
    DLLEXPORT void SetImage(const std::string& imagename);

    // Widget overrides
    DLLEXPORT virtual bool CanExpand() const override
    {
        return true;
    }

    REFERENCE_COUNTED_PTR_TYPE(ImageWidget);

protected:
    DLLEXPORT virtual void _AcquireRenderResources() override;

private:
    std::string ImageName;
};

}} // namespace Leviathan::GUI
