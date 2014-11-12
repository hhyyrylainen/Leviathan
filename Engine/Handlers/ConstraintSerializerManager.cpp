// ------------------------------------ //
#ifndef LEVIATHAN_CONSTRAINTSERIALIZERMANAGER
#include "ConstraintSerializerManager.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::ConstraintSerializerManager::ConstraintSerializerManager(){

}

DLLEXPORT Leviathan::ConstraintSerializerManager::~ConstraintSerializerManager(){

}

DLLEXPORT ConstraintSerializerManager* Leviathan::ConstraintSerializerManager::Get(){

    return Staticinstance;
}

ConstraintSerializerManager* Leviathan::ConstraintSerializerManager::Staticinstance = NULL;
// ------------------------------------ //
DLLEXPORT bool Leviathan::ConstraintSerializerManager::Init(){

    Serializers.push_back(new BaseConstraintSerializer());
    return true;
}

DLLEXPORT void Leviathan::ConstraintSerializerManager::Release(){

    GUARD_LOCK_THIS_OBJECT();

    auto end = Serializers.end();
    for(auto iter = Serializers.begin(); iter != end; ++iter){

        SAFE_DELETE((*iter));
    }
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ConstraintSerializerManager::CreateConstraint(BaseObject* object1,
    BaseObject* object2, Entity::ENTITY_CONSTRAINT_TYPE type, sf::Packet &packet)
{
    DEBUG_BREAK;

}
// ------------------------------------ //
DLLEXPORT shared_ptr<sf::Packet> Leviathan::ConstraintSerializerManager::SerializeConstraintData(
    Entity::BaseConstraint* constraint)
{

    DEBUG_BREAK;
}

