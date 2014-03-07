#ifndef PONG_PACKETS
#define PONG_PACKETS
// ------------------------------------ //
#ifndef PONGINCLUDES
#include "PongIncludes.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Networking/GameSpecificPacketHandler.h"


// Define all the types of packets //
#define PONG_PACKET_JOINGAME_REQUEST				1
#define PONG_PACKET_JOINGAME_RESPONSE				1


namespace Pong{

	//! \brief Class that holds custom packet's data
	class PongJoinGameRequest : public Leviathan::BaseGameSpecificRequestPacket{
	public:
		PongJoinGameRequest() : Leviathan::BaseGameSpecificRequestPacket(PONG_PACKET_JOINGAME_REQUEST){


		}
	};
	
	enum PONG_JOINGAMERESPONSE_TYPE {PONG_JOINGAMERESPONSE_TYPE_LOBBY, PONG_JOINGAMERESPONSE_TYPE_MATCH, PONG_JOINGAMERESPONSE_TYPE_GAMEEND};

	//! \brief Class that holds custom packet's data
	class PongJoinGameResponse : public Leviathan::BaseGameSpecificResponsePacket{
	public:
		PongJoinGameResponse(PONG_JOINGAMERESPONSE_TYPE type) : Leviathan::BaseGameSpecificResponsePacket(PONG_PACKET_JOINGAME_RESPONSE), RType(type){

		}

		PONG_JOINGAMERESPONSE_TYPE RType;
	};

	//! \brief Allows sending and receiving of custom packet, PongJoingGame
	class PongJoingGameRequestFactory : public Leviathan::BaseGameSpecificPacketFactory{
	public:
		PongJoingGameRequestFactory() : Leviathan::BaseGameSpecificPacketFactory(PONG_PACKET_JOINGAME_REQUEST, true){

		}

		virtual bool SerializeToPacket(GameSpecificPacketData* data, sf::Packet &packet){

			// Request has no data //
			return true;
		}

		virtual shared_ptr<GameSpecificPacketData> UnSerializeObjectFromPacket(sf::Packet &packet){

			return shared_ptr<GameSpecificPacketData>(new GameSpecificPacketData(new PongJoinGameRequest()));
		}
	};
	
	//! \brief Allows sending and receiving of custom packet, PongJoingGame
	class PongJoingGameResponseFactory : public Leviathan::BaseGameSpecificPacketFactory{
	public:
		PongJoingGameResponseFactory() : Leviathan::BaseGameSpecificPacketFactory(PONG_PACKET_JOINGAME_RESPONSE, false){

		}

		virtual bool SerializeToPacket(GameSpecificPacketData* data, sf::Packet &packet){

			// Response does have data //
			packet << static_cast<PongJoinGameResponse*>(data->ResponseBaseData)->RType;

			return true;
		}

		virtual shared_ptr<GameSpecificPacketData> UnSerializeObjectFromPacket(sf::Packet &packet){

			int tmptype;
			// Try to extract the type data //
			if(!(packet >> tmptype)){

				return NULL;
			}

			return shared_ptr<GameSpecificPacketData>(new GameSpecificPacketData(new PongJoinGameResponse(
				static_cast<PONG_JOINGAMERESPONSE_TYPE>(tmptype))));
		}
	};



	//! \brief Registers all custom packets that Pong needs
	class PongPackets{
	public:
		static void RegisterAllPongPacketTypes(){
			// Register all the factories //
			Leviathan::GameSpecificPacketHandler::Get()->RegisterNewTypeFactory(new PongJoingGameResponseFactory());
			Leviathan::GameSpecificPacketHandler::Get()->RegisterNewTypeFactory(new PongJoingGameRequestFactory());



		}
	};


}


#endif