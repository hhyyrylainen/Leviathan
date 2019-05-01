// ------------------------------------ //
#include "ImageWidget.h"

#include "GUI/BaseGuiContainer.h"

using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //
DLLEXPORT ImageWidget::ImageWidget(const std::string& image) : ImageName(image)
{
    DatablockName = "image_widget_" + std::to_string(ID);
}

DLLEXPORT ImageWidget::~ImageWidget() {}
// ------------------------------------ //
DLLEXPORT void ImageWidget::SetImage(const std::string& imagename)
{
    ImageName = imagename;

    // Update the rendering if this widget is already displayed
    if(Node) {
        DEBUG_BREAK;

        ContainedIn->OnSizeChanged();
    }
}
// ------------------------------------ //
DLLEXPORT void ImageWidget::_AcquireRenderResources()
{
    WidgetWithStandardResources::_AcquireRenderResources();

    _CreateDatablockWithTextureName(ImageName, &NaturalWidth, &NaturalHeight);
    _CreateStandardMeshAndItem(
        "image_widget_" + std::to_string(ID) + "_mesh", NaturalWidth, NaturalHeight);
}
