#ifndef LEVIATHAN_GAMESPECIFICPACKET
#define LEVIATHAN_GAMESPECIFICPACKET
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "SFML/Network/Packet.hpp"


namespace Leviathan{




	//! \brief Base class for all user defined request packets
	class BaseGameSpecificRequestPacket{
	public:
		DLLEXPORT BaseGameSpecificRequestPacket(int typenumber);
		DLLEXPORT virtual ~BaseGameSpecificRequestPacket();

		//! Contains the type of the packet
		//! \see BaseGameSpecificFactory::TypeIDNumber
		int TypeIDNumber;
	};

	//! \brief Base class for all user defined response packets
	class BaseGameSpecificResponsePacket{
	public:
		DLLEXPORT BaseGameSpecificResponsePacket(int typenumber);
		DLLEXPORT virtual ~BaseGameSpecificResponsePacket();


		//! Contains the type of the packet
		//! \see BaseGameSpecificFactory::TypeIDNumber
		int TypeIDNumber;
	};


	//! \brief Class that contains all data associated with a game specific packet
	class GameSpecificPacketData{
	public:
		DLLEXPORT GameSpecificPacketData(BaseGameSpecificResponsePacket* newddata);
		DLLEXPORT GameSpecificPacketData(BaseGameSpecificRequestPacket* newddata);
		DLLEXPORT ~GameSpecificPacketData();

		//! Marks whether this contains BaseGameSpecificRequestPacket or BaseGameSpecificResponsePacket
		bool IsRequest;
		//! Base object pointer if this is a request
		BaseGameSpecificRequestPacket* RequestBaseData;

		//! Base object pointer if this wasn't a request
		BaseGameSpecificResponsePacket* ResponseBaseData;

		//! Contains the type of the packet
		//! \note This is a copy of BaseGameSpecificResponsePacket::TypeIDNumber or BaseGameSpecificRequestPacket::TypeIDNumber
		//! \see BaseGameSpecificFactory::TypeIDNumber
		int TypeIDNumber;
	};

	//! \brief Base class that is passed to the list of type handlers to GameSpecificPacketHandler
	class BaseGameSpecificPacketFactory{
	public:
		DLLEXPORT BaseGameSpecificPacketFactory(int typenumber, bool isrequesttype);
		DLLEXPORT virtual ~BaseGameSpecificPacketFactory();

		DLLEXPORT virtual bool SerializeToPacket(GameSpecificPacketData* data, sf::Packet &packet) = 0;
		DLLEXPORT virtual shared_ptr<GameSpecificPacketData> UnSerializeObjectFromPacket(sf::Packet &packet) = 0;

		//! The integer identifying when this factory needs to be used
		int TypeIDNumber;
		//! Specifies are the TypeIDNumbers corresponding to request or response packets
		bool HandlesRequests;
	};

	//! \brief Handles construction of all game specific packets
	class GameSpecificPacketHandler : public Object{
	public:
		DLLEXPORT GameSpecificPacketHandler(NetworkInterface* usetoreport);
		DLLEXPORT ~GameSpecificPacketHandler();


		DLLEXPORT void PassGameSpecificDataToPacket(GameSpecificPacketData* datatosend, sf::Packet &packet);

		DLLEXPORT shared_ptr<GameSpecificPacketData> ReadGameSpecificPacketFromPacket(bool responsepacket, sf::Packet &packet);

		//! \brief Adds a new type that can be handled
		DLLEXPORT void RegisterNewTypeFactory(BaseGameSpecificPacketFactory* newdfactoryobject);


		DLLEXPORT static GameSpecificPacketHandler* Get();
	protected:

		shared_ptr<BaseGameSpecificPacketFactory> _FindFactoryForType(int typenumber, bool requesttype);

		void _CheckVectorSorting();
		// ------------------------------------ //

		//! \brief Stores a list of custom type factories
		//! \note Map to easily map the ID to a handler
		std::vector<shared_ptr<BaseGameSpecificPacketFactory>> AllPacketFactories;

		//! Marks whether or not the vector is indexed
		bool IsVectorSorted;


		static GameSpecificPacketHandler* Staticaccess;
	};

}
#endif