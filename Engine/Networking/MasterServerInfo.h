// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //

namespace Leviathan{
// Used to pass master server info to the application //
struct MasterServerInformation{
    MasterServerInformation(bool iammaster, const std::string &identificationstr) :
        MasterServerIdentificationString(identificationstr), RequireMaster(false),
        IAmMyOwnMaster(true)
    {

    }
    MasterServerInformation() : RequireMaster(false), IAmMyOwnMaster(false){
    }
    MasterServerInformation(const std::string &masterslistfile,
        const std::string &identification,
        const std::string &masterserverlistaddress,
        const std::string &masterserverlistpagename,
        const std::string &loginsession, bool requireconnection = false) :
        MasterListFetchServer(masterserverlistaddress),
        MasterListFetchPage(masterserverlistpagename),
        StoredListFile(masterslistfile), MasterServerIdentificationString(identification),
        LoginStoreFile(loginsession), RequireMaster(requireconnection), IAmMyOwnMaster(false)
    {

    }
        
        
    std::string MasterListFetchServer;
    std::string MasterListFetchPage;
    std::string StoredListFile;
    std::string MasterServerIdentificationString;
    std::string LoginStoreFile;
    bool RequireMaster;
    bool IAmMyOwnMaster;
};

}

#ifdef LEAK_INTO_GLOBAL
using Leviathan::MasterServerInformation;
#endif


