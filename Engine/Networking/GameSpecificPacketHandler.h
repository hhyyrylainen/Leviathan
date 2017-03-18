// Leviathan Game Engine
// Copyright (c) 2012-2016 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "../Common/SFMLPackets.h"
#include <memory>

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

		//! Marks whether this contains BaseGameSpecificRequestPacket or
        //! BaseGameSpecificResponsePacket
		bool IsRequest;
        
		//! Base object pointer if this is a request
		BaseGameSpecificRequestPacket* RequestBaseData;

		//! Base object pointer if this wasn't a request
		BaseGameSpecificResponsePacket* ResponseBaseData;

		//! Contains the type of the packet
		//! \note This is a copy of BaseGameSpecificResponsePacket::TypeIDNumber
        //! or BaseGameSpecificRequestPacket::TypeIDNumber
		//! \see BaseGameSpecificFactory::TypeIDNumber
		int TypeIDNumber;
	};

	//! \brief Base class that is passed to the list of type handlers to GameSpecificPacketHandler
	class BaseGameSpecificPacketFactory{
	public:
		DLLEXPORT BaseGameSpecificPacketFactory(int typenumber, bool isrequesttype);
		DLLEXPORT virtual ~BaseGameSpecificPacketFactory();

		//! \brief Function for factories to pass their object data to a packet when requested
		//! \note Should not throw anything
		DLLEXPORT virtual bool SerializeToPacket(GameSpecificPacketData* data, sf::Packet &packet) = 0;
		//! \brief Called when a factory needs to extract data from a packet
		//! \note Should not throw, instead should return NULL when invalid data is encountered
		DLLEXPORT virtual std::shared_ptr<GameSpecificPacketData>
        UnSerializeObjectFromPacket(sf::Packet &packet) = 0;

		//! The integer identifying when this factory needs to be used
		int TypeIDNumber;
		//! Specifies are the TypeIDNumbers corresponding to request or response packets
		bool HandlesRequests;
	};

	//! \brief Handles construction of all game specific packets
	class GameSpecificPacketHandler{
	public:
		DLLEXPORT GameSpecificPacketHandler(NetworkInterface* usetoreport);
		DLLEXPORT ~GameSpecificPacketHandler();


		DLLEXPORT void PassGameSpecificDataToPacket(GameSpecificPacketData* datatosend,
            sf::Packet &packet);

		DLLEXPORT std::shared_ptr<GameSpecificPacketData> ReadGameSpecificPacketFromPacket(
            bool responsepacket, sf::Packet &packet);

		//! \brief Adds a new type that can be handled
		DLLEXPORT void RegisterNewTypeFactory(BaseGameSpecificPacketFactory* newdfactoryobject);


		DLLEXPORT static GameSpecificPacketHandler* Get();
	protected:

        std::shared_ptr<BaseGameSpecificPacketFactory> _FindFactoryForType(int typenumber,
            bool requesttype);

		void _CheckVectorSorting();
		// ------------------------------------ //

		//! \brief Stores a list of custom type factories
		//! \note Map to easily map the ID to a handler
		std::vector<std::shared_ptr<BaseGameSpecificPacketFactory>> AllPacketFactories;

		//! Marks whether or not the vector is indexed
		bool IsVectorSorted;


		static GameSpecificPacketHandler* Staticaccess;
	};

}

#ifdef LEAK_INTO_GLOBAL
using Leviathan::GameSpecificPacketData;
#endif

