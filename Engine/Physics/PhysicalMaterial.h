// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "PhysicalWorld.h"

#include "ObjectFiles/ObjectFile.h"


namespace Leviathan {

class PhysicsBody;
class PhysicalWorld;

//! This callback is called first when the bounding boxes of 2 bodies touch. This can be used
//! then to disable the collision or allow the collision. After this has run if the bodies
//! actually hit each other PhysicsMaterialContactCallback will be called once for each
//! touching point. If no fine grained hit detection is needed this can also be used for that.
//! Returning false disables the collision between the objects. Returning true enables the
//! collision
using PhysicsMaterialAABBCallback = bool (*)(PhysicalWorld& world, PhysicsBody&, PhysicsBody&);


//! This function is called every time an actual collision occurs
//! between objects for each collision point between the materials.
//! This is recommended to be used for playing sound effects and other
//! fine grained hit detection.
using PhysicsMaterialContactCallback = void (*)(
    PhysicalWorld& world, PhysicsBody&, PhysicsBody&);



//! Defines properties between two materials
struct PhysMaterialDataPair {
    DLLEXPORT inline PhysMaterialDataPair() :
        Collidable(true), AABBCallback(nullptr), ContactCallback(nullptr)
    {}

    DLLEXPORT inline PhysMaterialDataPair(PhysMaterialDataPair&& other) :
        Collidable(other.Collidable), AABBCallback(std::move(other.AABBCallback)),
        ContactCallback(std::move(other.ContactCallback))
    {}

    DLLEXPORT inline PhysMaterialDataPair& operator=(PhysMaterialDataPair&& other)
    {
        Collidable = other.Collidable;
        AABBCallback = std::move(other.AABBCallback);
        ContactCallback = std::move(other.ContactCallback);
        return *this;
    }

    //! \brief Sets the material pair to collide or not with each other
    //! \warning setting this to false prevents collision callbacks from being called
    //! \note Collision masks are more effective. This basically does collision rejecting
    //! automatically like PhysicsMaterialAABBCallback would
    //! \todo Remove this
    DLLEXPORT inline PhysMaterialDataPair& SetCollidable(bool collidable)
    {
        Collidable = collidable;
        return *this;
    }

    // //! \brief Sets the softness of the material pair
    // //!
    // //! Recommended 1.f and below, default 0.15f, higher value is less soft)
    // DLLEXPORT inline PhysMaterialDataPair& SetSoftness(const float& softness)
    // {
    //     Softness = softness;
    //     return *this;
    // }

    // //! \brief Sets the elasticity (restitution) of the material pair
    // //!
    // //! Recommended 1.f and below, default 0.4f, higher value might be more elasticity)
    // DLLEXPORT inline PhysMaterialDataPair& SetElasticity(const float& elasticity)
    // {
    //     Elasticity = elasticity;
    //     return *this;
    // }

    // //! \brief Sets the friction between the materials
    // //!
    // //! default static is 0.9f and dynamic (sliding) 0.5f
    // DLLEXPORT inline PhysMaterialDataPair& SetFriction(
    //     const float& staticfriction, const float& dynamicfriction)
    // {
    //     DynamicFriction = dynamicfriction;
    //     StaticFriction = staticfriction;
    //     return *this;
    // }

    //! \brief Sets the callback functions that are called when the material interacts
    DLLEXPORT inline PhysMaterialDataPair& SetCallbacks(
        const PhysicsMaterialAABBCallback aabb, const PhysicsMaterialContactCallback contact)
    {
        AABBCallback = aabb;
        ContactCallback = contact;
        return *this;
    }

    // ------------------------------------ //
    bool Collidable;
    // float Elasticity;
    // float StaticFriction;
    // float DynamicFriction;
    // float Softness;

    // Callbacks //
    PhysicsMaterialAABBCallback AABBCallback;
    PhysicsMaterialContactCallback ContactCallback;
};

class PhysicalMaterial {
    friend PhysicsMaterialManager;

public:
    //! \note ID must be unique
    DLLEXPORT PhysicalMaterial(const std::string& name, int id);
    // DLLEXPORT PhysicalMaterial(std::shared_ptr<ObjectFileObject> fileobject);
    DLLEXPORT ~PhysicalMaterial();

    //! \brief Data pairing
    //! \note The returned reference is valid only until the next call to FormPairWith
    DLLEXPORT PhysMaterialDataPair& FormPairWith(const PhysicalMaterial& other);

    //! \brief Returns data for this material to interact with another
    inline auto* GetPairWith(int otherid) const
    {
        const auto found = InterractionsWith.find(otherid);

        if(found == InterractionsWith.end())
            return static_cast<const PhysMaterialDataPair*>(nullptr);

        return &found->second;
    }

    DLLEXPORT int GetID()
    {
        return ID;
    }

    // \todo file loading function //


    DLLEXPORT inline std::string GetName() const
    {
        return Name;
    }

private:
    const std::string Name;
    const int ID;

    //! The key is the ID of the other material
    std::unordered_map<int, PhysMaterialDataPair> InterractionsWith;
};

} // namespace Leviathan
