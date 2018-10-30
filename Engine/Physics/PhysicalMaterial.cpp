// ------------------------------------ //
#include "PhysicalMaterial.h"

using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::PhysicalMaterial::PhysicalMaterial(const std::string& name, int id) :
    Name(name), ID(id)
{}

DLLEXPORT Leviathan::PhysicalMaterial::~PhysicalMaterial() {}
// ------------------------------------ //
DLLEXPORT PhysMaterialDataPair& Leviathan::PhysicalMaterial::FormPairWith(
    const PhysicalMaterial& other)
{
    return InterractionsWith[other.ID] = PhysMaterialDataPair();
}
// ------------------------------------ //
