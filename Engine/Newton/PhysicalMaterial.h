// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "PhysicalWorld.h"

#include "ObjectFiles/ObjectFile.h"

//! This callback is called first when the bounding boxes of 2 bodies touch. This can be used
//! then to disable the collision or allow the collision. After this has run if the bodies
//! actually hit each other PhysicsMaterialContactCallback will be called once for each
//! touching point. If no fine grained hit detection is needed this can also be used for that.
//! Returning 0 (as opposed to 1) disables the collision between the objects
using PhysicsMaterialAABBCallback = int (*)(const NewtonMaterial* material,
    const NewtonBody* body0, const NewtonBody* body1, int threadIndex);

//! This function is called every time an actual collision occurs
//! between objects for each collision point between the materials.
//! This is recommended to be used for playing sound effects and other
//! fine grained hit detection.
using PhysicsMaterialContactCallback = void (*)(
    const NewtonJoint* contact, dFloat timestep, int threadIndex);

namespace Leviathan {

class PhysicsMaterialManager;
class PhysicalMaterial;

// Defines properties between two materials //
struct PhysMaterialDataPair {
    DLLEXPORT inline PhysMaterialDataPair(const std::string& othername) :
        OtherName(othername), Collidable(true), Elasticity(0.4f), StaticFriction(0.9f),
        DynamicFriction(0.5f), Softness(0.15f), AABBCallback(nullptr), ContactCallback(nullptr)
    {
    }
    // ------------------ Property setting functions ------------------ //
    // Recommended values are from the newton wiki, more info there //

    //! \brief Sets the material pair to collide or not with each other
    //! \warning setting this to false prevents collision callbacks from being called
    DLLEXPORT inline PhysMaterialDataPair& SetCollidable(bool collidable)
    {
        Collidable = collidable;
        return *this;
    }

    //! \brief Sets the softness of the material pair
    //!
    //! Recommended 1.f and below, default 0.15f, higher value is less soft)
    DLLEXPORT inline PhysMaterialDataPair& SetSoftness(const float& softness)
    {
        Softness = softness;
        return *this;
    }

    //! \brief Sets the elasticity (restitution) of the material pair
    //!
    //! Recommended 1.f and below, default 0.4f, higher value might be more elasticity)
    DLLEXPORT inline PhysMaterialDataPair& SetElasticity(const float& elasticity)
    {
        Elasticity = elasticity;
        return *this;
    }

    //! \brief Sets the friction between the materials
    //!
    //! default static is 0.9f and dynamic (sliding) 0.5f
    DLLEXPORT inline PhysMaterialDataPair& SetFriction(
        const float& staticfriction, const float& dynamicfriction)
    {
        DynamicFriction = dynamicfriction;
        StaticFriction = staticfriction;
        return *this;
    }

    //! \brief Sets the callback functions that are called when the material interacts
    DLLEXPORT inline PhysMaterialDataPair& SetCallbacks(
        const PhysicsMaterialAABBCallback aabb, const PhysicsMaterialContactCallback contact)
    {
        AABBCallback = aabb;
        ContactCallback = contact;
        return *this;
    }


    // Creates the material to the world, value changes won't apply after this //
    void ApplySettingsToWorld(
        NewtonWorld* world, int thisid, int otherid, PhysicalMaterial* materialowner);

    // ------------------------------------ //
    std::string OtherName;
    bool Collidable;
    float Elasticity;
    float StaticFriction;
    float DynamicFriction;
    float Softness;

    // Callbacks //
    PhysicsMaterialAABBCallback AABBCallback;
    PhysicsMaterialContactCallback ContactCallback;
};

class PhysicalMaterial {
    friend PhysicsMaterialManager;

public:
    DLLEXPORT PhysicalMaterial(const std::string& name);
    DLLEXPORT PhysicalMaterial(std::shared_ptr<ObjectFileObject> fileobject);
    DLLEXPORT ~PhysicalMaterial();

    // ------------------ Data pairing functions ------------------ //
    DLLEXPORT PhysMaterialDataPair& FormPairWith(const PhysicalMaterial& other);

    DLLEXPORT int GetMaterialIDIfLoaded(NewtonWorld* world);

    // \todo file loading function //


    DLLEXPORT inline std::string GetName() const
    {
        return Name;
    }

private:
    // Internal to world loading functions //
    void _CreateMaterialToWorld(NewtonWorld* world);
    void _ApplyMaterialPropertiesToWorld(NewtonWorld* world);

    void _ClearFromWorld(NewtonWorld* world);

    // ------------------------------------ //
    std::string Name;
    int EngineID;

    // this material can be loaded into multiple worlds at once, so we need to quickly
    // fetch right id value
    std::map<NewtonWorld*, int> NewtonWorldAndID;

    // values that are sent to newton //
    std::list<std::shared_ptr<PhysMaterialDataPair>> InterractionVariables;
};

} // namespace Leviathan
