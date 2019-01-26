// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
// ------------------------------------ //

namespace Leviathan {

struct WorldNetworkSettings {

    //! \brief Creates default settings for not doing any networking
    inline WorldNetworkSettings() {}

    //! \brief Creates settings for a dedicated server
    static inline WorldNetworkSettings GetSettingsForServer()
    {
        WorldNetworkSettings settings;
        settings.IsAuthoritative = true;
        settings.AutoCreateNetworkComponents = true;
        settings.DoInterpolation = false;

        return settings;
    }

    //! \brief Creates settings for a client
    static inline WorldNetworkSettings GetSettingsForClient()
    {
        WorldNetworkSettings settings;
        settings.IsAuthoritative = false;
        settings.AutoCreateNetworkComponents = true;
        settings.DoInterpolation = true;

        return settings;
    }

    //! \brief Creates settings for acting both as a server and having a local player (listen
    //! server)
    static inline WorldNetworkSettings GetSettingsForHybrid()
    {
        WorldNetworkSettings settings;
        settings.IsAuthoritative = true;
        settings.AutoCreateNetworkComponents = true;
        settings.DoInterpolation = true;

        return settings;
    }

    //! \brief Creates settings for worlds that have no interest in networking (singleplayer
    //! only)
    static inline WorldNetworkSettings GetSettingsForSinglePlayer()
    {
        WorldNetworkSettings settings;
        settings.IsAuthoritative = true;
        settings.AutoCreateNetworkComponents = false;
        settings.DoInterpolation = false;

        return settings;
    }

    //! This is true on the server
    bool IsAuthoritative = false;

    //! This controls if the World automatically creates things needed for sending updates if
    //! IsAuthoritative and receiving updates otherwise
    bool AutoCreateNetworkComponents = false;

    //! Enables clientside interpolation functions
    bool DoInterpolation = true;
};


} // namespace Leviathan
