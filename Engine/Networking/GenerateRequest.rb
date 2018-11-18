#!/bin/ruby
# Generates classes for all the different requests

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
     Variable.new("SecureType", "CONNECTION_ENCRYPTION", serializeas: "int32_t", ),
     Variable.new("PublicKey", "std::string", default: ""),
     Variable.new("AdditionalSettings", "std::string", default: ""),
   ]],
  
  ["Authenticate",
   [
     Variable.new("UserName", "std::string", default: ""),
     Variable.new("AuthToken", "uint64_t", serializeas: "sf::Uint64", default: "0"),
     Variable.new("AuthPasswd", "std::string", default: ""),
   ]],

  ["Identification",
   [
     Variable.new("DDOSBlock", "std::string", default: "\"#{"v" * 65}\""),
   ]],

  ["RemoteConsoleOpen",
   [
     Variable.new("SessionToken", "int32_t"),
   ]],

  ["RemoteConsoleAccess",
   [
     Variable.new("SessionToken", "int32_t"),
   ]],

  ["JoinServer",
   [
     Variable.new("MasterServerToken", "int32_t"),
   ]],

  ["JoinGame",
   [
     Variable.new("Options", "std::string", default: %{""}),
   ]],

  ["GetSingleSyncValue",
   [
     Variable.new("NameOfValue", "std::string"),
   ]],

  ["RequestCommandExecution",
   [
     Variable.new("Command", "std::string"),
   ]],

  ["ConnectInput",
   [
     Variable.new("DataForObject", "sf::Packet", move: true),
   ]],

  ["WorldClockSync",
   [
     Variable.new("WorldID", "int32_t"),
     Variable.new("Ticks", "int32_t"),
     Variable.new("EngineMSTweak", "int32_t"),
     Variable.new("Absolute", "bool"),
   ]],

  ["DoRemoteConsoleOpen",
   [
     Variable.new("Token", "int32_t"),
   ]],
  
]

# Add all classes
thingsToGenerate.each do |type|

  created = ResponseClass.new("Request#{type[0]}")
  
  created.base("NetworkRequest")
  created.addDeserializeArg("uint32_t idforresponse")
  created.deserializeBase("idforresponse") 
  created.baseConstructor("NETWORK_REQUEST_TYPE::#{type[0]}")
  
  # Parameters
  type[1].each do |n|

    # n is a hash of the arguments
    created.addMember n
  end

  generator.add created
end


# Output the file
generator.run

