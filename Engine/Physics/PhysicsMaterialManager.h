// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "PhysicalMaterial.h"

namespace Leviathan {

//! \brief Contains a material list for applying automatic properties to PhysicsBody and
//! collision callbacks
//! \todo file loading function
class PhysicsMaterialManager {
public:
    //! \brief Adds a physics material. This now takes effect instantly and all worlds using
    //! this material manager will see the change on next physics update.
    //! This will set the material ID.
    DLLEXPORT void LoadedMaterialAdd(std::unique_ptr<PhysicalMaterial>&& material);

    //! \brief Gets the ID of a material based on name
    DLLEXPORT int GetMaterialID(const std::string& name);

    //! \brief Accesses material based on name
    //! \note If you modify the material only new physics bodies will get the changed
    //! properties
    DLLEXPORT PhysicalMaterial* GetMaterial(const std::string& name);

    //! \brief Accesses material by ID
    DLLEXPORT PhysicalMaterial* GetMaterial(int id);


private:
    //! Map for fast finding
    std::map<std::string, std::unique_ptr<PhysicalMaterial>> LoadedMaterials;
    //! Also for finding by id
    std::map<int, PhysicalMaterial*> LoadedMaterialsByID;
};

} // namespace Leviathan
