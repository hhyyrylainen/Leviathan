// ------------------------------------ //
#ifndef LEVIATHAN_BASECONSTRAINTSERIALIZER
#include "BaseConstraintSerializer.h"
#endif
#include "Entities/Bases/BaseConstraintable.h"
using namespace Leviathan;
using namespace Entity;
// ------------------------------------ //
DLLEXPORT Leviathan::BaseConstraintSerializer::BaseConstraintSerializer(){

}

DLLEXPORT Leviathan::BaseConstraintSerializer::~BaseConstraintSerializer(){


}
// ------------------------------------ //
DLLEXPORT bool Leviathan::BaseConstraintSerializer::CanHandleType(Entity::ENTITY_CONSTRAINT_TYPE type) const{

    switch(type){
        case ENTITY_CONSTRAINT_TYPE_SLIDER:
            return true;
        default:
            return false;
    }
}
// ------------------------------------ //
DLLEXPORT shared_ptr<sf::Packet> Leviathan::BaseConstraintSerializer::SerializeConstraint(Entity::BaseConstraint*
    constraint, Entity::ENTITY_CONSTRAINT_TYPE &type)
{
    // Get the type and then cast to the subclass //
    type = constraint->GetType();

    switch(type){
        case ENTITY_CONSTRAINT_TYPE_SLIDER:
        {
            SliderConstraint* slider = dynamic_cast<SliderConstraint*>(constraint);

            if(!slider)
                return nullptr;

            auto data = make_shared<sf::Packet>();

            const Float3 &axis = slider->GetAxis();
            
            (*data) << axis.X << axis.Y << axis.Z;

            Logger::Get()->Write("Sending constraint: "+Convert::ToString(axis.X)+", "+Convert::ToString(axis.Y)+", "
                +Convert::ToString(axis.Z));
            
            return data;
        }
    }

    // Unknown type... //
    Logger::Get()->Error("BaseConstraintSerializer: tried to serialize wrong type...");
    return nullptr;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::BaseConstraintSerializer::UnSerializeConstraint(BaseObject* object1,
    BaseObject* object2, Entity::ENTITY_CONSTRAINT_TYPE type, sf::Packet &packet,
    bool create /*= true*/)
{

    switch(type){
        case ENTITY_CONSTRAINT_TYPE_SLIDER:
        {
            if(!create){
                // Break the one matching the type //
                DEBUG_BREAK;
                return true;
            }

            // TODO: could try to find a way around this cast //
            auto firstobj = dynamic_cast<BaseConstraintable*>(object1);
            auto secondobj = dynamic_cast<BaseConstraintable*>(object2);
            
            if(!firstobj || !secondobj)
                return false;

            // Get the custom data //
            Float3 axis;

            packet >> axis.X >> axis.Y >> axis.Z;

            if(!packet)
                return false;

            Logger::Get()->Write("Creating constraint: "+Convert::ToString(axis.X)+", "+Convert::ToString(axis.Y)+", "
                +Convert::ToString(axis.Z));

            // Create the constraint //
            firstobj->CreateConstraintWith<SliderConstraint>(secondobj)->SetParameters(axis)->
                Init();
            
            
            return true;
        }
    }

    return false;
}











