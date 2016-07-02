#!/bin/ruby
# Generates classes for all the different responses

require_relative '../../Helpers/CommonCode'
require_relative '../../Helpers/FileGen'

abort "no target file provided" if ARGV.count < 1

generator = Generator.new ARGV[0]

thingsToGenerate = [
  ["Identification",
   [
     { type: "std::string", name: "UserReadableData" },
     { type: "std::string", name: "GameName" },
     { type: "std::string", name: "GameVersionString" },
     { type: "std::string", name: "LeviathanVersionString",
       default: "LEVIATHAN_VERSION_ANSIS" }
   ]],

  ["InvalidRequest",
   [
     { type: "NETWORK_RESPONSE_INVALIDREASON", as: "uint8_t", name: "Invalidness" },
     { type: "std::string", name: "AdditionalInfo", default: "\"\"" }
   ]],

  ["ServerStatus",
   [
     { type: "std::string", name: "ServerNameString" },
     { type: "bool", name: "Joinable" },
     { type: "SERVER_JOIN_RESTRICT", as: "uint8_t", name: "JoinRestriction" },
     { type: "SERVER_STATUS", as: "uint8_t", name: "ServerStatus" },
     { type: "int32_t", name: "Players" },
     { type: "int32_t", name: "MaxPlayers" },
     { type: "int32_t", name: "Bots" },
     { type: "int32_t", name: "AdditionalFlags", default: "0" },
   ]],

  ["DisconnectInput",
   [
     { type: "int32_t", name: "InputID" },
     { type: "int32_t", name: "OwnerID" }
   ]],
  
  ["ServerDisallow",
   [
     { type: "std::string", name: "Message" },
     { type: "NETWORK_RESPONSE_INVALIDREASON", as: "uint8_t", name: "Reason" }
   ]],

  ["ServerAllow",
   [
     { type: "SERVER_ACCEPTED_TYPE", as: "uint8_t", name: "ServerAcceptedWhat" },
     { type: "std::string", name: "Message", default: "\"\"" }
   ]],

  ["SyncValData",
   [
     { type: "NamedVariableList", name: "SyncValueData" }
   ]],
  
  ["SyncDataEnd",
   [
     { type: "bool", name: "Succeeded" }
   ]],

  ["SyncResourceData",
   [
     { type: "std::string", name: "OurCustomData" }
   ]],

  ["CreateNetworkedInput",
   [
     { type: "sf::Packet", name: "OurCustomData", move: true }
   ]],

  ["UpdateNetworkedInput",
   [
     { type: "int32_t", name: "InputID" },
     { type: "sf::Packet", name: "UpdateData", move: true }
   ]],

  ["EntityCreation",
   [
     { type: "int32_t", name: "WorldID" },
     { type: "sf::Packet", name: "InitialEntity", move: true}
   ]],

  ["EntityDestruction",
   [
     { type: "int32_t", name: "WorldID" },
     { type: "ObjectID", name: "EntityID" }
   ]],

  ["WorldFrozen",
   [
     { type: "int32_t", name: "WorldID" },
     { type: "bool", name: "Frozen" },
     { type: "int32_t", name: "TickNumber" }
   ]],

  ["EntityConstraint",
   [
     { type: "int32_t", name: "WorldID" },
     { type: "bool", name: "Create" },
     { type: "int32_t", name: "ConstraintID" },
     { type: "ObjectID", name: "EntityID1" },
     { type: "ObjectID", name: "EntityID2" },
     { type: "ENTITY_CONSTRAINT_TYPE", as: "uint16_t", name: "ConstraintType" },
     { type: "ObjectID", name: "ConstraintData" }
   ]],
  
  ["EntityUpdate",
   [
     { type: "int32_t", name: "WorldID" },
     { type: "int32_t", name: "TickNumber" },
     { type: "int32_t", name: "ReferenceTick" },
     { type: "ObjectID", name: "EntityID" },
     { type: "sf::Packet", name: "UpdateData", move: true}
   ]],
  
  ["CacheUpdated",
   [
     { type: "NamedVariableList", name: "Variable" }
   ]],

  ["CacheRemoved",
   [
     { type: "std::string", name: "Name" }
   ]],

  
]

# Add all classes
thingsToGenerate.each do |type|

  created = ResponseClass.new("Response#{type[0]}")

  created.base("NetworkResponse")
  created.baseConstructor("NETWORK_RESPONSE_TYPE::#{type[0]}, responseid")
  created.constructorMember "uint32_t responseid"
  
  # Parameters
  type[1].each do |n|

    # n is a hash of the arguments
    created.addMember n
  end

  generator.add created
end

# Output the file
generator.run

