// ------------------------------------ //
#include "Layout.h"

#include "DiligentCore/Graphics/GraphicsEngine/interface/InputLayout.h"

using namespace Leviathan;
using namespace Leviathan::Rendering;
// ------------------------------------ //
DLLEXPORT LayoutElements::~LayoutElements()
{
    _DeletePrevious();
}
// ------------------------------------ //
DLLEXPORT LayoutElements::LayoutElements(LayoutElements&& other) noexcept :
    Elements(other.Elements), ElementCount(other.ElementCount), Owned(other.Owned)
{
    if(Owned)
        other.Elements = nullptr;
}

DLLEXPORT LayoutElements::LayoutElements(const LayoutElements& other) noexcept :
    ElementCount(other.ElementCount), Owned(other.Owned)
{
    if(!Owned) {
        Elements = other.Elements;
    } else {
        Elements = other.Duplicate().release();
    }
}
// ------------------------------------ //
DLLEXPORT LayoutElements::LayoutElements(
    std::unique_ptr<const Diligent::LayoutElement>&& elements, uint16_t count) :
    Elements(elements.release()),
    ElementCount(count), Owned(true)
{}
// ------------------------------------ //
DLLEXPORT LayoutElements::LayoutElements(
    const Diligent::LayoutElement* elements, uint16_t count, bool owned) :
    Elements(elements),
    ElementCount(count), Owned(owned)
{}
// ------------------------------------ //
DLLEXPORT LayoutElements& LayoutElements::operator=(LayoutElements&& other) noexcept
{
    _DeletePrevious();

    Elements = other.Elements;
    ElementCount = other.ElementCount;
    Owned = other.Owned;

    if(Owned)
        other.Elements = nullptr;

    return *this;
}

DLLEXPORT LayoutElements& LayoutElements::operator=(const LayoutElements& other) noexcept
{
    _DeletePrevious();

    ElementCount = other.ElementCount;
    Owned = other.Owned;

    if(!Owned) {
        Elements = other.Elements;
    } else {
        Elements = other.Duplicate().release();
    }

    return *this;
}
// ------------------------------------ //
DLLEXPORT std::unique_ptr<Diligent::LayoutElement> LayoutElements::Duplicate() const
{
    if(!Elements || ElementCount < 1)
        return nullptr;

    std::unique_ptr<Diligent::LayoutElement> data(reinterpret_cast<Diligent::LayoutElement*>(
        std::malloc(sizeof(Diligent::LayoutElement) * ElementCount)));

    for(decltype(ElementCount) i = 0; i < ElementCount; ++i) {
        new(&data.get()[i]) Diligent::LayoutElement(Elements[i]);
    }

    return data;
}
// ------------------------------------ //
void LayoutElements::_DeletePrevious()
{
    if(Owned && Elements) {
        delete Elements;
        Elements = nullptr;
    }
}
