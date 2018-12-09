#!/bin/ruby
# Generates classes for all the different responses

require_relative '../../RubySetupSystem/RubyCommon.rb'
require_relative '../../Helpers/FileGen.rb'

abort "no target file provided" if ARGV.count < 1

generator = Generator.new ARGV[0]

thingsToGenerate = [

  ["Connect",
   [
     Variable.new("CheckValue", "int32_t", default: "42"),
   ]],
  
  ["Security",
   [
     Variable.new("SecureType", "CONNECTION_ENCRYPTION", serializeas: "int32_t"),
     Variable.new("PublicKey", "std::string", default: ""),
     Variable.new("EncryptedSymmetricKey", "std::string", default: ""),
   ]],
   
  ["Authenticate",
   [
     Variable.new("UserID", "int32_t"),
     Variable.new("UserToken", "uint64_t", serializeas: "sf::Uint64", default: "0"),
   ]],


  ["Identification",
   [
     Variable.new("UserReadableData", "std::string"),
     Variable.new("GameName", "std::string"),
     Variable.new("GameVersionString", "std::string"),
     Variable.new("LeviathanVersionString", "std::string", default: "LEVIATHAN_VERSION_ANSIS"),
   ]],

  ["InvalidRequest",
   [
     Variable.new("Invalidness", "NETWORK_RESPONSE_INVALIDREASON", serializeas: "uint8_t"),
     Variable.new("AdditionalInfo", "std::string", default: ""),
   ]],

  ["ServerStatus",
   [
     Variable.new("ServerNameString", "std::string"),
     Variable.new("Joinable", "bool"),
     Variable.new("JoinRestriction", "SERVER_JOIN_RESTRICT", serializeas: "uint8_t"),
     Variable.new("ServerStatus", "SERVER_STATUS", serializeas: "uint8_t"),
     Variable.new("Players", "int32_t"),
     Variable.new("MaxPlayers", "int32_t"),
     Variable.new("Bots", "int32_t"),
     Variable.new("AdditionalFlags", "int32_t", default: "0"),
   ]],

  ["DisconnectInput",
   [
     Variable.new("InputID", "int32_t"),
     Variable.new("OwnerID", "int32_t"),
   ]],
  
  ["ServerDisallow",
   [
     Variable.new("Message", "std::string"),
     Variable.new("Reason", "NETWORK_RESPONSE_INVALIDREASON", serializeas: "uint8_t"),
   ]],

  ["ServerAllow",
   [
     Variable.new("ServerAcceptedWhat", "SERVER_ACCEPTED_TYPE", serializeas: "uint8_t"),
     Variable.new("Message", "std::string", default: "\"\""),
   ]],

  ["SyncValData",
   [
     Variable.new("SyncValueData", "NamedVariableList"),
   ]],
  
  ["SyncDataEnd",
   [
     Variable.new("Succeeded", "bool"),
   ]],

  ["SyncResourceData",
   [
     Variable.new("OurCustomData", "std::string"),
   ]],

  ["CreateNetworkedInput",
   [
     Variable.new("OurCustomData", "sf::Packet", move: true ),
   ]],

  ["UpdateNetworkedInput",
   [
     Variable.new("InputID", "int32_t"),
     Variable.new("UpdateData", "sf::Packet", move: true ),
   ]],

  ["StartWorldReceive",
   [
     Variable.new("WorldID", "int32_t"),
     Variable.new("WorldType", "int32_t"),
     Variable.new("ExtraOptions", "std::string", default: "\"\""),
   ]],

  ["EntityCreation",
   [
     Variable.new("WorldID", "int32_t"),
     Variable.new("EntityID", "int32_t"),
     Variable.new("ComponentCount", "uint32_t"),
     Variable.new("InitialComponentData", "sf::Packet", move: true),
   ]],

  ["EntityDestruction",
   [
     Variable.new("WorldID", "int32_t"),
     Variable.new("EntityID", "ObjectID"),
   ]],

  ["EntityLocalControlStatus",
   [
     Variable.new("WorldID", "int32_t"),
     Variable.new("EntityID", "ObjectID"),
     Variable.new("Enabled", "bool"),
   ]],

  ["WorldFrozen",
   [
     Variable.new("WorldID", "int32_t"),
     Variable.new("Frozen", "bool"),
     Variable.new("TickNumber", "int32_t"),
   ]],

  # ["EntityConstraint",
  #  [
  #    Variable.new("WorldID", "int32_t"),
  #    Variable.new("Create", "bool"),
  #    Variable.new("ConstraintID", "int32_t"),
  #    Variable.new("EntityID1", "ObjectID"),
  #    Variable.new("EntityID2", "ObjectID"),
  #    Variable.new("ConstraintType", "ENTITY_CONSTRAINT_TYPE", serializeas: "uint16_t"),
  #    Variable.new("ConstraintData", "ObjectID"),
  #  ]],
  
  ["EntityUpdate",
   [
     Variable.new("WorldID", "int32_t"),
     Variable.new("TickNumber", "int32_t"),
     Variable.new("ReferenceTick", "int32_t"),
     Variable.new("EntityID", "ObjectID"),
     Variable.new("UpdateData", "sf::Packet", move: true),
   ]],
  
  ["CacheUpdated",
   [
     Variable.new("Variable", "NamedVariableList"),
   ]],

  ["CacheRemoved",
   [
     Variable.new("Name", "std::string"),
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

