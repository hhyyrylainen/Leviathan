// ------------------------------------ //
#ifndef PONG_CONSTRAINTS
#include "PongConstraints.h"
#endif
#include "CommonPong.h"
#include "Entities/Objects/TrackEntityController.h"
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
    if(type == static_cast<ENTITY_CONSTRAINT_TYPE>(PONG_CONSTRAINT_TYPE_EMOTIONAL))
        return true;
    if(type == static_cast<ENTITY_CONSTRAINT_TYPE>(PONG_CONSTRAINT_TYPE_GAME_BALL))
        return true;
    return false;
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

            (*packet) << emotional->GetPlayerNumber() << static_cast<int32_t>(emotional->GetType());

            return packet;
        }
        case PONG_CONSTRAINT_TYPE_GAME_BALL:
        {
            GameBallConnection* ballconstraint = dynamic_cast<GameBallConnection*>(constraint);

            if(!ballconstraint)
                return nullptr;
            
            shared_ptr<sf::Packet> packet = make_shared<sf::Packet>();

            (*packet) << int8_t(42);

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
            int32_t type;
            packet >> number >> type;

            if(!packet)
                return false;

            auto casted1 = dynamic_cast<BaseConstraintable*>(object1);
            LINK_TYPE actualtype = static_cast<LINK_TYPE>(type);

            if(!casted1)
                return false;

            casted1->CreateConstraintWith<EmotionalConnection>(NULL)->SetParameters(number, actualtype)->Init();
            
            return true;
        }
        case PONG_CONSTRAINT_TYPE_GAME_BALL:
        {
            if(!create){

                return true;
            }

            // Magic value check //
            int8_t themeaning;

            packet >> themeaning;

            if(!packet || themeaning != 42)
                return false;
            
            auto casted1 = dynamic_cast<BaseConstraintable*>(object1);

            if(!casted1)
                return false;

            casted1->CreateConstraintWith<GameBallConnection>(NULL)->Init();
            
            return true;
        }
    }

    return false;
}
// ------------------ EmotionalConnection ------------------ //
EmotionalConnection::EmotionalConnection(GameWorld* world, BaseConstraintable* parent, BaseConstraintable* child) :
    BaseConstraint(static_cast<ENTITY_CONSTRAINT_TYPE>(PONG_CONSTRAINT_TYPE_EMOTIONAL), world,
        parent, child), PlayerNumber(-1), TypeToLink(LINK_TYPE_LAST)
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

                switch(TypeToLink){
                    case LINK_TYPE_PADDLE:
                        curply->SetPaddleObject(nullptr);
                        break;
                    case LINK_TYPE_TRACK:
                        curply->SetTrackObject(nullptr, nullptr);
                        break;
                    case LINK_TYPE_GOAL:
                        curply->SetGoalAreaObject(nullptr);
                        break;
                }
                return;
            }
            curply = curply->GetSplit();
        }
    }
    
}
// ------------------------------------ //
EmotionalConnection* EmotionalConnection::SetParameters(int plyid, LINK_TYPE whattolink){

    PlayerNumber = plyid;
    TypeToLink = whattolink;
    return this;
}
// ------------------------------------ //
bool EmotionalConnection::_CheckParameters(){
    if(TypeToLink != LINK_TYPE_PADDLE && TypeToLink != LINK_TYPE_TRACK)
        return false;
    
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
                switch(TypeToLink){
                    case LINK_TYPE_PADDLE:
                        curply->SetPaddleObject(
                            OwningWorld->GetSmartPointerForObject(dynamic_cast<BaseObject*>(
                                    ParentObject)));
                        break;
                    case LINK_TYPE_TRACK:
                        curply->SetTrackObject(
                            OwningWorld->GetSmartPointerForObject(dynamic_cast<BaseObject*>(
                                    ParentObject)),
                            dynamic_cast<TrackEntityController*>(ParentObject));
                        break;
                    case LINK_TYPE_GOAL:
                        curply->SetGoalAreaObject(
                            OwningWorld->GetSmartPointerForObject(dynamic_cast<BaseObject*>(
                                    ParentObject)));
                        break;
                    default:
                        return false;
                }
                
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
// ------------------ GameBallConnection ------------------ //
Pong::GameBallConnection::GameBallConnection(Leviathan::GameWorld* world, Leviathan::BaseConstraintable* parent,
    Leviathan::BaseConstraintable* child) :
    Entity::BaseConstraint(static_cast<ENTITY_CONSTRAINT_TYPE>(PONG_CONSTRAINT_TYPE_GAME_BALL),
        world, parent, child)
{

}

Pong::GameBallConnection::~GameBallConnection(){

    BasePongParts::Get()->GetArena()->RegisterBall(nullptr);
}
// ------------------------------------ //
bool Pong::GameBallConnection::_CheckParameters(){

    return true;
}

bool Pong::GameBallConnection::_CreateActualJoint(){

    Logger::Get()->Info("Pong: registering ball");
    
    BasePongParts::Get()->GetArena()->RegisterBall(OwningWorld->GetSmartPointerForObject(dynamic_cast<BaseObject*>(
                ParentObject)));    
}












