// ------------------------------------ //
#ifndef LEVIATHAN_CONSTRAINTSERIALIZERMANAGER
#include "ConstraintSerializerManager.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::ConstraintSerializerManager::ConstraintSerializerManager(){
    Staticinstance = this;
}

DLLEXPORT Leviathan::ConstraintSerializerManager::~ConstraintSerializerManager(){
    Staticinstance = NULL;
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

    GUARD_LOCK();

    auto end = Serializers.end();
    for(auto iter = Serializers.begin(); iter != end; ++iter){

        SAFE_DELETE((*iter));
    }
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ConstraintSerializerManager::AddSerializer(BaseConstraintSerializer* serializer){
    GUARD_LOCK();
    Serializers.push_back(serializer);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ConstraintSerializerManager::CreateConstraint(Constraintable &object1,
    Constraintable &object2, ENTITY_CONSTRAINT_TYPE type, sf::Packet &packet, bool create)
{

    auto end = Serializers.end();
    for(auto iter = Serializers.begin(); iter != end; ++iter){

        if((*iter)->CanHandleType(type)){

            // Found the right one, try to create a constraint //
            return (*iter)->UnSerializeConstraint(object1, object2, type, packet, create);
        }
    }

    // None handled it so it can't have been created //
    return false;
}
// ------------------------------------ //
DLLEXPORT std::shared_ptr<sf::Packet> Leviathan::ConstraintSerializerManager::SerializeConstraintData(
    BaseConstraint* constraint)
{
    if(!constraint)
        return nullptr;
    
    auto type = constraint->GetType();
    
    GUARD_LOCK();
    
    // Find a serializer for it and return whatever it returns //
    auto end = Serializers.end();
    for(auto iter = Serializers.begin(); iter != end; ++iter){

        if((*iter)->CanHandleType(type)){

            return (*iter)->SerializeConstraint(constraint, type);
        }
    }

    // No matching serializer //
    return nullptr;
}











