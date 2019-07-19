//! \file Tests for various supporting GUI methods. Doesn't actually
//! try any rendering or anything like that

#include "GUI/VideoPlayer.h"

#include "../PartialEngine.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace Leviathan::Test;
using namespace Leviathan::GUI;


// TEST_CASE("Leviathan VideoPlayer loads correctly", "[gui][video][xrequired][ogre]")
// {

//     // TODO: add leviathan intro video that can be attempted to be opened

//     // Requires audio
//     SoundDevice sound;

//     PartialEngineWithBSF engine(nullptr, &sound);
//     engine.Log.IgnoreWarnings = true;

//     REQUIRE(sound.Init(false, true));

//     VideoPlayer player;

//     REQUIRE(player.Play("Data/Videos/SampleVideo.mkv"));

//     CHECK(player.GetDuration() == 10.336f);

//     CHECK(player.GetVideoWidth() == 1920);
//     CHECK(player.GetVideoHeight() == 1080);
//     CHECK(player.IsStreamValid());

//     player.Stop();

//     sound.Release();
// }
