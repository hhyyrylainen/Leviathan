// Leviathan Game Engine
// Copyright (c) 2012-2020 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include <memory>
#include <tuple>

namespace Diligent {
struct LayoutElement;
}

namespace Leviathan { namespace Rendering {

//! \brief Wrapper for vertex element definitions
class LayoutElements {
public:
    //! \brief Empty elements
    LayoutElements() = default;
    DLLEXPORT ~LayoutElements();

    DLLEXPORT LayoutElements(LayoutElements&& other) noexcept;
    DLLEXPORT LayoutElements(const LayoutElements& other) noexcept;

    //! \brief Owning variant
    DLLEXPORT LayoutElements(
        std::unique_ptr<const Diligent::LayoutElement>&& elements, uint16_t count);

    //! \brief Directly setting all variant. Owned must be true if this is responsible for
    //! deleting the data
    DLLEXPORT LayoutElements(
        const Diligent::LayoutElement* elements, uint16_t count, bool owned);

    DLLEXPORT LayoutElements& operator=(LayoutElements&& other) noexcept;
    DLLEXPORT LayoutElements& operator=(const LayoutElements& other) noexcept;

    inline std::tuple<const Diligent::LayoutElement*, uint16_t> GetElements() const
    {
        return std::make_tuple(Elements, ElementCount);
    }

    //! \brief Hacks around Diligent not working with const layout elements
    inline std::tuple<Diligent::LayoutElement*, uint16_t> GetElementsForDiligent() const
    {
        return std::make_tuple(const_cast<Diligent::LayoutElement*>(Elements), ElementCount);
    }

    //! \returns A pointer to a fresh copy of the data in this
    DLLEXPORT std::unique_ptr<Diligent::LayoutElement> Duplicate() const;

private:
    void _DeletePrevious();

private:
    const Diligent::LayoutElement* Elements = nullptr;

    uint16_t ElementCount = 0;

    //! If this is the owner the data is deleted when this is destroyed
    bool Owned = false;
};

}} // namespace Leviathan::Rendering
