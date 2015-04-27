#include "PartialEngine.h"

using namespace Leviathan;

std::unique_ptr<NewtonManager> NewtonHolder::StaticNewtonManager = nullptr;
std::unique_ptr<PhysicsMaterialManager> NewtonHolder::StaticPhysicsMaterialManager = nullptr;
