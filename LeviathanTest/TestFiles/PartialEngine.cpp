#include "PartialEngine.h"

using namespace Leviathan;

unique_ptr<NewtonManager> NewtonHolder::StaticNewtonManager = nullptr;
unique_ptr<PhysicsMaterialManager> NewtonHolder::StaticPhysicsMaterialManager = nullptr;
