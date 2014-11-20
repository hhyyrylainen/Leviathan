// ------------------------------------ //
#ifndef PONG_CONSTRAINTS
#include "PongConstraints.h"
#endif
#include "CommonPong.h"
using namespace Pong;
using namespace Leviathan;
using namespace Entity;
// ------------------------------------ //

Pong::PongConstraintSerializer::PongConstraintSerializer(){

}

Pong::PongConstraintSerializer::~PongConstraintSerializer(){

}
// ------------------------------------ //
bool Pong::PongConstraintSerializer::CanHandleType(ENTITY_CONSTRAINT_TYPE type) const{
    return type == static_cast<ENTITY_CONSTRAINT_TYPE>(PONG_CONSTRAINT_TYPE_EMOTIONAL);
}
// ------------------------------------ //
shared_ptr<sf::Packet> Pong::PongConstraintSerializer::SerializeConstraint(BaseConstraint* constraint,
    ENTITY_CONSTRAINT_TYPE &type)
{
    type = constraint->GetType();

    switch(type){
        case PONG_CONSTRAINT_TYPE_EMOTIONAL:
        {
            EmotionalConnection* emotional = dynamic_cast<EmotionalConnection*>(constraint);

            if(!emotional)
                return nullptr;
            
            shared_ptr<sf::Packet> packet = make_shared<sf::Packet>();

            (*packet) << emotional->GetPlayerNumber();

            return packet;
        }
    }

    return nullptr;
}

bool Pong::PongConstraintSerializer::UnSerializeConstraint(BaseObject* object1, BaseObject* object2,
    ENTITY_CONSTRAINT_TYPE type, sf::Packet &packet, bool create /*= true*/)
{
    switch(type){
        case PONG_CONSTRAINT_TYPE_EMOTIONAL:
        {
            if(!create){

                // TODO: unlink an existing connection
                return true;
            }
            
            int32_t number;
            packet >> number;

            if(!packet)
                return false;

            auto casted1 = dynamic_cast<BaseConstraintable*>(object1);

            if(!casted1)
                return false;

            casted1->CreateConstraintWith<EmotionalConnection>(NULL)->SetParameters(number)->Init();
            
            return true;
        }
    }

    return false;
}
// ------------------ EmotionalConnection ------------------ //
EmotionalConnection::EmotionalConnection(GameWorld* world, BaseConstraintable* parent, BaseConstraintable* child) :
    BaseConstraint(static_cast<ENTITY_CONSTRAINT_TYPE>(PONG_CONSTRAINT_TYPE_EMOTIONAL), world,
        parent, child), PlayerNumber(-1)
{

}
EmotionalConnection::~EmotionalConnection(){

    Logger::Get()->Write("Unlinking Object/Paddle from player number: "+Convert::ToString(PlayerNumber));
    
    // Set the slot's object to NULL //
    auto plys = BasePongParts::Get()->GetPlayers()->GetVec();

    auto end = plys.end();
    for(auto iter = plys.begin(); iter != end; ++iter){

        PlayerSlot* curply = (*iter);

        while(curply){

            if(curply->GetPlayerNumber() == PlayerNumber){
                curply->SetPaddleObject(nullptr);
                return;
            }
            curply = curply->GetSplit();
        }
    }
    
}
// ------------------------------------ //
EmotionalConnection* EmotionalConnection::SetParameters(int plyid){

    PlayerNumber = plyid;
    return this;
}
// ------------------------------------ //
bool EmotionalConnection::_CheckParameters(){
    return PlayerNumber >= 0;
}
        
bool EmotionalConnection::_CreateActualJoint(){

    Logger::Get()->Write("Linking Object/Paddle to player number: "+Convert::ToString(PlayerNumber));

    auto plys = BasePongParts::Get()->GetPlayers()->GetVec();

    auto end = plys.end();
    for(auto iter = plys.begin(); iter != end; ++iter){

        PlayerSlot* curply = (*iter);

        while(curply){

            if(curply->GetPlayerNumber() == PlayerNumber){

                curply->SetPaddleObject(OwningWorld->GetSmartPointerForObject(dynamic_cast<BaseObject*>(ParentObject)));
                return true;
            }
            curply = curply->GetSplit();
        }
    }

    // Failed to find a player //
    Logger::Get()->Warning("EmotionalConnection: failed to find a player with number: "+Convert::ToString(
            PlayerNumber));
    return false;
}

