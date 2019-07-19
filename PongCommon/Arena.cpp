// ------------------------------------ //
#include "Arena.h"

#include "CommonPong.h"
#include "Entities/Components.h"
#include "Handlers/ObjectLoader.h"
#include "Physics/PhysicsMaterialManager.h"
#include "TextureGenerator.h"
#include "Utility/Random.h"

#include <iostream>
using namespace Pong;
using namespace std;
// ------------------------------------ //
Pong::Arena::Arena(Leviathan::GameWorld* world) :
    TargetWorld(world), BottomBrush(0), TrailKeeper(0), Ball(0)
{}

Pong::Arena::~Arena() {}
// ------------------------------------ //
bool Pong::Arena::GenerateArena(BasePongParts* game, PlayerList& plys)
{

    QUICKTIME_THISSCOPE;
    GUARD_LOCK();

    std::vector<PlayerSlot*>& plyvec = plys.GetVec();

    // Check sanity of values //
    if(plyvec.empty() || plyvec.size() > 4) {
        game->SetError("Player count must be over 1");
        return false;
    }

    _ClearPointers(guard);


    // Fast access to objects //
    DEBUG_BREAK;
    // NewtonWorld* nworld = TargetWorld->GetPhysicalWorld()->GetNewtonWorld();

    VerifyTrail(guard);

    // Set the options with the unified function //
    ColourTheBallTrail(guard, Float4(1.f));


    // calculate sizes //
    float width = 20 * BASE_ARENASCALE;
    float height = width;

    float mheight = 3 * BASE_ARENASCALE;
    float sideheight = 2 * BASE_ARENASCALE;
    float paddleheight = 1 * BASE_ARENASCALE;
    float paddlewidth = 3 * BASE_ARENASCALE;
    float bottomthickness = 0.5 * BASE_ARENASCALE;

    float paddlethicknesswhole = 1 * BASE_ARENASCALE;
    float paddlethicknesssplit = 0.5f * BASE_ARENASCALE;

    float sidexsize = width / 20.f;
    float sideysize = height / 20.f;

    float paddlemass = 60.f;
    // float paddlemass = 0.f;

    string materialbase = "Material.001";
    string sidematerialtall = "Material.001";
    string sidematerialshort = "Material.001";
    string materialclosedpaddlearea = "Material.001";

    bool infiniteloop = false;

newtonmaterialfetchstartlabel:




    // int ArenaMatID = Leviathan::PhysicsMaterialManager::Get()->GetMaterialIDForWorld(
    //     "ArenaMaterial", nworld);
    // int PaddleID = Leviathan::PhysicsMaterialManager::Get()->GetMaterialIDForWorld(
    //     "PaddleMaterial", nworld);
    // int GoalAreaMatID = Leviathan::PhysicsMaterialManager::Get()->GetMaterialIDForWorld(
    //     "GoalAreaMaterial", nworld);
    // int ArenaBaseID = Leviathan::PhysicsMaterialManager::Get()->GetMaterialIDForWorld(
    //     "ArenaBottomMaterial", nworld);

    // if(ArenaMatID == -1){
    //     // All are probably invalid, force world adding //
    //     if(infiniteloop){

    //         Logger::Get()->Error("Stuck infinitely regenerating materials");
    //         return false;
    //     }

    //     Logger::Get()->Warning("Arena: GenerateArena: world doesn't have materials,
    //     regenerating");
    //     Leviathan::PhysicsMaterialManager::Get()->CreateActualMaterialsForWorld(nworld);
    //     infiniteloop = true;
    //     goto newtonmaterialfetchstartlabel;
    // }

    // create brushes //



    // WARNING: Huge mess ahead!
    DEBUG_BREAK;
    //
    //    // base surface brush //
    //    BottomBrush = ObjectLoader::LoadBrushToWorld(TargetWorld, materialbase,
    //        Float3(width, bottomthickness, height), 0.f, ArenaBaseID,
    //        {Float3(0.f, -bottomthickness/2.f, 0.f), Float4::IdentityQuaternion()});
    //
    //
    //    // Arena ceiling that keeps the ball in //
    //    auto topbrush = ObjectLoader::LoadBrushToWorld(TargetWorld, "",
    //        Float3(width, bottomthickness, height), 0.f, ArenaBaseID,
    //        {Float3(0.f, paddleheight+bottomthickness/2.f+BASE_ARENASCALE/2.f, 0.f),
    //                Float4::IdentityQuaternion()});
    //
    //    auto& node = TargetWorld->GetComponent<RenderNode>(topbrush);
    //    node.Marked = true;
    //    node.Hidden = true;
    //
    //    // arena sides //
    //
    //    // left top //
    //    ObjectLoader::LoadBrushToWorld(TargetWorld, sidematerialtall,
    //        Float3(sidexsize, mheight, sideysize),
    //        0.f, ArenaMatID, {Float3(-width/2.f+sidexsize/2.f, mheight/2.f,
    //        -height/2.f+sideysize/2.f),
    //                Float4::IdentityQuaternion()});
    //
    //    ObjectLoader::LoadBrushToWorld(TargetWorld, sidematerialshort,
    //        Float3(sidexsize, sideheight, sideysize),  0.f, ArenaMatID, {
    //            Float3(-width/2.f+sidexsize*1.5f, sideheight/2.f, -height/2.f+sideysize/2.f),
    //                Float4::IdentityQuaternion()});
    //
    //    ObjectLoader::LoadBrushToWorld(TargetWorld, sidematerialshort,
    //        Float3(sidexsize, sideheight, sideysize),
    //        0.f, ArenaMatID, {
    //            Float3(-width/2.f+sidexsize/2.f, sideheight/2.f, -height/2.f+sideysize*1.5f),
    //                Float4::IdentityQuaternion()});
    //
    //    // top right //
    //    ObjectLoader::LoadBrushToWorld(TargetWorld, sidematerialtall,
    //        Float3(sidexsize, mheight, sideysize),
    //        0.f, ArenaMatID, {
    //            Float3(width/2.f-sidexsize/2.f, mheight/2.f, -height/2.f+sideysize/2.f),
    //                Float4::IdentityQuaternion() });
    //
    //    ObjectLoader::LoadBrushToWorld(TargetWorld, sidematerialshort,
    //        Float3(sidexsize, sideheight, sideysize),
    //        0.f, ArenaMatID, {
    //            Float3(width/2.f-sidexsize*1.5f, sideheight/2.f, -height/2.f+sideysize/2.f),
    //                Float4::IdentityQuaternion() });
    //
    //    ObjectLoader::LoadBrushToWorld(TargetWorld, sidematerialshort,
    //        Float3(sidexsize, sideheight, sideysize),
    //        0.f, ArenaMatID, {
    //            Float3(width/2.f-sidexsize/2.f, sideheight/2.f, -height/2.f+sideysize*1.5f),
    //                Float4::IdentityQuaternion() });
    //
    //
    //    // bottom left //
    //    ObjectLoader::LoadBrushToWorld(TargetWorld, sidematerialtall,
    //        Float3(sidexsize, mheight, sideysize),
    //        0.f, ArenaMatID, {
    //            Float3(-width/2.f+sidexsize/2.f, mheight/2.f, height/2.f-sideysize/2.f),
    //                Float4::IdentityQuaternion() });
    //
    //    ObjectLoader::LoadBrushToWorld(TargetWorld, sidematerialshort,
    //        Float3(sidexsize, sideheight, sideysize),
    //        0.f, ArenaMatID, {
    //            Float3(-width/2.f+sidexsize*1.5f, sideheight/2.f, height/2.f-sideysize/2.f),
    //                Float4::IdentityQuaternion() });
    //
    //    ObjectLoader::LoadBrushToWorld(TargetWorld, sidematerialshort,
    //        Float3(sidexsize, sideheight, sideysize),
    //        0.f, ArenaMatID, {
    //            Float3(-width/2.f+sidexsize/2.f, sideheight/2.f, height/2.f-sideysize*1.5f),
    //                Float4::IdentityQuaternion() });
    //
    //    // bottom right //
    //    ObjectLoader::LoadBrushToWorld(TargetWorld, sidematerialtall,
    //        Float3(sidexsize, mheight, sideysize),
    //        0.f, ArenaMatID, {
    //            Float3(width/2.f-sidexsize/2.f, mheight/2.f, height/2.f-sideysize/2.f),
    //                Float4::IdentityQuaternion() });
    //
    //    ObjectLoader::LoadBrushToWorld(TargetWorld, sidematerialshort,
    //        Float3(sidexsize, sideheight, sideysize),
    //        0.f, ArenaMatID, {
    //            Float3(width/2.f-sidexsize*1.5f, sideheight/2.f, height/2.f-sideysize/2.f),
    //                Float4::IdentityQuaternion() });
    //
    //    ObjectLoader::LoadBrushToWorld(TargetWorld, sidematerialshort,
    //        Float3(sidexsize, sideheight, sideysize),
    //        0.f, ArenaMatID, {
    //            Float3(width/2.f-sidexsize/2.f, sideheight/2.f, height/2.f-sideysize*1.5f),
    //                Float4::IdentityQuaternion() });
    //
    //
    //    // fill empty paddle spaces //
    //    for(size_t i = 0; i < plyvec.size(); i++){
    //
    //        if(!plyvec[i]->IsSlotActive()){
    //            // The sub slot can save this //
    //            if(auto split = plyvec[i]->GetSplit())
    //                if(split->IsSlotActive())
    //                    continue;
    //            // Fill the empty slot //
    //            switch(i){
    //            case 0:
    //                {
    //                    // fill right with wall //
    //                    ObjectLoader::LoadBrushToWorld(TargetWorld,
    //                        sidematerialshort, Float3(sidexsize, sideheight/2,
    //                        sideysize*16.f), 0.f, ArenaMatID, {
    //                            Float3(width/2.f-sidexsize/2.f, sideheight/4.f, 0),
    //                                Float4::IdentityQuaternion() });
    //                }
    //                break;
    //            case 1:
    //                {
    //                    // fill bottom with wall //
    //                    ObjectLoader::LoadBrushToWorld(TargetWorld,
    //                        sidematerialshort, Float3(sidexsize*16.f, sideheight/2,
    //                        sideysize), 0.f, ArenaMatID, { Float3(0, sideheight/4.f,
    //                        height/2.f-sideysize/2.f),
    //                                Float4::IdentityQuaternion() });
    //                }
    //                break;
    //            case 2:
    //                {
    //                    // fill left with wall //
    //                    ObjectLoader::LoadBrushToWorld(TargetWorld,
    //                        sidematerialshort, Float3(sidexsize, sideheight/2,
    //                        sideysize*16.f), 0.f, ArenaMatID, {
    //                        Float3(-width/2.f+sidexsize/2.f, sideheight/4.f, 0),
    //                                Float4::IdentityQuaternion() });
    //                }
    //                break;
    //            case 3:
    //                {
    //                    // fill top with wall //
    //                    ObjectLoader::LoadBrushToWorld(TargetWorld,
    //                        sidematerialshort, Float3(sidexsize*16.f, sideheight/2,
    //                        sideysize), 0.f, ArenaMatID, { Float3(0, sideheight/4.f,
    //                        -height/2.f+sideysize/2.f),
    //                                Float4::IdentityQuaternion() });
    //                }
    //                break;
    //
    //            }
    //
    //        }
    //    }
    //
    //    Constraintable& castedbottombrush =
    //    TargetWorld->GetComponent<Constraintable>(BottomBrush);
    //
    //    // paddles and link slots to objects//
    //
    //    // loop through players and add paddles //
    //    for(size_t i = 0; i < plyvec.size(); i++){
    //        // skip empty slots //
    //        if(!plyvec[i]->IsSlotActive())
    //            continue;
    //        bool secondary = false;
    // addplayerpaddlelabel:
    //
    //        bool splitslotopen = false;
    //        if(plyvec[i]->GetSplit())
    //            splitslotopen = plyvec[i]->GetSplit()->IsSlotActive();
    //
    //        // Choose the thickness based on the split count of THIS slot //
    //        float paddlethickness = secondary || splitslotopen ? paddlethicknesssplit:
    //        paddlethicknesswhole;
    //
    //        // Get the colour for the paddle //
    //        Float4 colour = secondary ? plyvec[i]->GetSplit()->GetColour():
    //        plyvec[i]->GetColour();
    //
    //        // setup position //
    //        float horiadjust = 0;
    //        if(secondary || splitslotopen)
    //            horiadjust = secondary ? 0: paddlethickness;
    //
    //        ObjectID plypaddle = 0;
    //
    //        switch(i){
    //            case 0:
    //            {
    //                plypaddle = ObjectLoader::LoadBrushToWorld(TargetWorld,
    //                    GetMaterialNameForPlayerColour(colour), Float3((i == 0 || i == 2) ?
    //                        paddlethickness: paddlewidth, paddleheight, (i == 0 || i == 2) ?
    //                        paddlewidth: paddlethickness), paddlemass, PaddleID, {
    //                        Float3(width/2.f-paddlethickness/2.f-horiadjust,
    //                        paddleheight/2.f, 0),
    //                            Float4::IdentityQuaternion() });
    //            }
    //            break;
    //            case 1:
    //            {
    //                plypaddle = ObjectLoader::LoadBrushToWorld(TargetWorld,
    //                    GetMaterialNameForPlayerColour(colour), Float3((i == 0 || i == 2) ?
    //                        paddlethickness: paddlewidth, paddleheight, (i == 0 || i == 2) ?
    //                        paddlewidth: paddlethickness), paddlemass, PaddleID, {
    //                        Float3(0, paddleheight/2.f,
    //                        width/2.f-paddlethickness/2.f-horiadjust),
    //                            Float4::IdentityQuaternion() });
    //            }
    //            break;
    //            case 2:
    //            {
    //                plypaddle = ObjectLoader::LoadBrushToWorld(TargetWorld,
    //                    GetMaterialNameForPlayerColour(colour), Float3((i == 0 || i == 2) ?
    //                        paddlethickness: paddlewidth, paddleheight, (i == 0 || i == 2) ?
    //                        paddlewidth: paddlethickness), paddlemass, PaddleID, {
    //                        Float3(-width/2.f+paddlethickness/2.f+horiadjust,
    //                        paddleheight/2.f,
    //                            0), Float4::IdentityQuaternion() });
    //            }
    //            break;
    //            case 3:
    //            {
    //                plypaddle = ObjectLoader::LoadBrushToWorld(TargetWorld,
    //                    GetMaterialNameForPlayerColour(colour), Float3((i == 0 || i == 2) ?
    //                        paddlethickness: paddlewidth, paddleheight, (i == 0 || i == 2) ?
    //                        paddlewidth: paddlethickness), paddlemass, PaddleID, {
    //                        Float3(0, paddleheight/2.f,
    //                        -width/2.f+paddlethickness/2.f+horiadjust),
    //                            Float4::IdentityQuaternion() });
    //            }
    //            break;
    //        }
    //
    //        // setup joints //
    //        auto& constraintable = TargetWorld->GetComponent<Constraintable>(plypaddle);
    //
    //        if(!constraintable.CreateConstraintWith<Leviathan::SliderConstraint>(castedbottombrush)->
    //            SetParameters((i == 0 || i == 2) ? Float3(0.f, 0.f, -1.f):
    //                Float3(1.f, 0.f, 0.f))->Init())
    //        {
    //            Logger::Get()->Error("Arena: GenerateArena: failed to create slider for
    //            paddle "+
    //                Convert::ToString(i+1));
    //        }
    //
    //        // link //
    //        secondary ? plyvec[i]->GetSplit()->SetPaddleObject(plypaddle):
    //            plyvec[i]->SetPaddleObject(plypaddle);
    //
    //        // Create the track controller //
    //        std::vector<Leviathan::Position::PositionData> MovementPositions(2);
    //
    //        switch(i){
    //        case 0:
    //            {
    //                MovementPositions[0] = {
    //                    Float3(width/2.f-paddlethickness/2.f-horiadjust, paddleheight/2.f,
    //                        height/2.f-sideysize*2-paddlewidth/2.f),
    //                        Float4::IdentityQuaternion()};
    //
    //                MovementPositions[1] = {
    //                    Float3(width/2.f-paddlethickness/2.f-horiadjust, paddleheight/2.f,
    //                        -height/2.f+sideysize*2+paddlewidth/2.f),
    //                        Float4::IdentityQuaternion()};
    //            }
    //            break;
    //        case 1:
    //            {
    //                MovementPositions[0] = {
    //                    Float3(width/2.f-sidexsize*2-paddlewidth/2.f, paddleheight/2.f,
    //                        width/2.f-paddlethickness/2.f-horiadjust),
    //                        Float4::IdentityQuaternion()};
    //
    //                MovementPositions[1] = {
    //                    Float3(-width/2.f+sidexsize*2+paddlewidth/2.f, paddleheight/2.f,
    //                        width/2.f-paddlethickness/2.f-horiadjust),
    //                        Float4::IdentityQuaternion()};
    //            }
    //            break;
    //        case 2:
    //            {
    //                MovementPositions[0] = {
    //                    Float3(-width/2.f+paddlethickness/2.f+horiadjust, paddleheight/2.f,
    //                        height/2.f-sideysize*2-paddlewidth/2.f),
    //                        Float4::IdentityQuaternion()};
    //                MovementPositions[1] = {
    //                    Float3(-width/2.f+paddlethickness/2.f+horiadjust, paddleheight/2.f,
    //                        -height/2.f+sideysize*2+paddlewidth/2.f),
    //                        Float4::IdentityQuaternion()};
    //            }
    //            break;
    //        case 3:
    //            {
    //                MovementPositions[0] = {
    //                    Float3(width/2.f-sidexsize*2-paddlewidth/2.f, paddleheight/2.f,
    //                        -width/2.f+paddlethickness/2.f+horiadjust),
    //                        Float4::IdentityQuaternion()
    //                };
    //                MovementPositions[1] = {
    //                    Float3(-width/2.f+sidexsize*2+paddlewidth/2.f, paddleheight/2.f,
    //                        -width/2.f+paddlethickness/2.f+horiadjust),
    //                        Float4::IdentityQuaternion()};
    //            }
    //            break;
    //        }
    //
    //        auto track = ObjectLoader::LoadTrackControllerToWorld(TargetWorld,
    //                MovementPositions);
    //
    //        auto& controller = TargetWorld->GetComponent<TrackController>(track);
    //
    //        // Set //
    //        secondary ? plyvec[i]->GetSplit()->SetTrackObject(track):
    //            plyvec[i]->SetTrackObject(track);
    //
    //        // Paddle should be in the middle by default, so set progress to 50% //
    //        controller.NodeProgress = 0.5f;
    //
    //        // Connect the paddle to the track //
    //        auto& trackparent = TargetWorld->GetComponent<Parent>(track);
    //
    //        auto& plyparentable = TargetWorld->GetComponent<Parentable>(plypaddle);
    //
    //        trackparent.AddChild(plypaddle, plyparentable);
    //
    //        if(secondary)
    //            continue;
    //
    //        // Create goal area for this slot //
    //        ObjectID goalarea = 0;
    //
    //        switch(i){
    //            case 0:
    //            {
    //                goalarea = ObjectLoader::LoadBrushToWorld(TargetWorld,
    //                    materialclosedpaddlearea,
    //                    Float3((i == 0 || i == 2) ? paddlethickness: width, sideheight,
    //                        (i == 0 || i == 2) ? height: paddlethickness), 0.f,
    //                        GoalAreaMatID, { Float3(width/2.f+paddlethickness/2.f,
    //                        sideheight/2.f, 0),
    //                            Float4::IdentityQuaternion() });
    //            }
    //            break;
    //            case 1:
    //            {
    //                goalarea = ObjectLoader::LoadBrushToWorld(TargetWorld,
    //                    materialclosedpaddlearea,
    //                    Float3((i == 0 || i == 2) ? paddlethickness: width, sideheight,
    //                        (i == 0 || i == 2) ? height: paddlethickness), 0.f,
    //                        GoalAreaMatID, { Float3(0, sideheight/2.f,
    //                        width/2.f+paddlethickness/2.f),
    //                            Float4::IdentityQuaternion() });
    //            }
    //            break;
    //            case 2:
    //            {
    //                goalarea = ObjectLoader::LoadBrushToWorld(TargetWorld,
    //                    materialclosedpaddlearea,
    //                    Float3((i == 0 || i == 2) ? paddlethickness: width, sideheight,
    //                        (i == 0 || i == 2) ? height: paddlethickness), 0.f,
    //                        GoalAreaMatID, { Float3(-width/2.f-paddlethickness/2.f,
    //                        sideheight/2.f, 0),
    //                            Float4::IdentityQuaternion() });
    //            }
    //            break;
    //            case 3:
    //            {
    //                goalarea = ObjectLoader::LoadBrushToWorld(TargetWorld,
    //                    materialclosedpaddlearea,
    //                    Float3((i == 0 || i == 2) ? paddlethickness: width, sideheight,
    //                        (i == 0 || i == 2) ? height: paddlethickness), 0.f,
    //                        GoalAreaMatID, { Float3(0, sideheight/2.f,
    //                        -width/2.f-paddlethickness/2.f),
    //                            Float4::IdentityQuaternion() });
    //            }
    //            break;
    //        }
    //
    //        auto& node = TargetWorld->GetComponent<RenderNode>(goalarea);
    //        node.Marked = true;
    //        node.Hidden = true;
    //
    //        // Set to slot //
    //        plyvec[i]->SetGoalAreaObject(goalarea);
    //
    //        // loop again if has secondary //
    //        if(plyvec[i]->GetSplit() != NULL){
    //            secondary = true;
    //            goto addplayerpaddlelabel;
    //        }
    //    }

    // Notify how much stuff was created //
    Logger::Get()->Info("Arena: finished generating arena, total objects: " +
                        Convert::ToString(TargetWorld->GetEntityCount()));

    return true;
}
// ------------------------------------ //
void Pong::Arena::_ClearPointers(Lock& guard)
{

    BottomBrush = 0;
    TrailKeeper = 0;
    Ball = 0;
}

void Pong::Arena::ServeBall()
{

    LetGoOfBall();

    GUARD_LOCK();

    // we want to load our ball prop into the world //
    /*Ball = ObjectLoader::LoadPropToWorld(
        TargetWorld, "PongBall", TargetWorld->GetPhysicalMaterial("BallMaterial"), {
            Float3(0.f, 0.5f, 0.f), Float4::IdentityQuaternion() });
*/
    // Parent the trail to the ball //
    // TODO: make this send it to the client //
    // DirectTrail->AddNonPhysicsParent(prop);

    // Update trail colour //
    ColourTheBallTrail(guard, Float4(1));

    Float3 dir(0);

    // Add some randomness //
    int count = Leviathan::Random::Get()->GetNumber(0, 15);

    BasePongParts* game = BasePongParts::Get();

    // TODO: remove this debug code
    bool someactive = 0;
    for(size_t i = 0; i < game->_PlayerList.Size(); i++) {

        if(game->_PlayerList[i]->IsSlotActive()) {

            someactive = true;
        }
    }

    if(!someactive)
        Logger::Get()->Error("NO ACTIVE SLOTS; WILL DEADLOCK");
    //

    while(count > -1) {

        for(size_t i = 0; i < game->_PlayerList.Size(); i++) {

            if(game->_PlayerList[i]->IsSlotActive()) {
                count--;
                if(count < 0) {
                    // Set direction //

                    switch(i) {
                    case 0:
                        dir = Float3(25.f, 0.f, 0.f) +
                              (Leviathan::Random::Get()->GetNumber(0, 1) ?
                                      Float3(0.f, 0.f, 1.f) :
                                      Float3(0.f, 0.f, -1.f));
                        break;
                    case 1:
                        dir = Float3(0.f, 0.f, 25.f) +
                              (Leviathan::Random::Get()->GetNumber(0, 1) ?
                                      Float3(1.f, 0.f, 0.f) :
                                      Float3(-1.f, 0.f, 0.f));
                        break;
                    case 2:
                        dir = Float3(-25.f, 0.f, 0.f) +
                              (Leviathan::Random::Get()->GetNumber(0, 1) ?
                                      Float3(0.f, 0.f, 1.f) :
                                      Float3(0.f, 0.f, -1.f));
                        break;
                    case 3:
                        dir = Float3(0.f, 0.f, -25.f) +
                              (Leviathan::Random::Get()->GetNumber(0, 1) ?
                                      Float3(1.f, 0.f, 0.f) :
                                      Float3(-1.f, 0.f, 0.f));
                        break;
                    }

                    break;
                }
            }
        }
    }

    auto& physics = TargetWorld->GetComponent<Physics>(Ball);

    physics.GetBody()->GiveImpulse(dir);
}
// ------------------------------------ //
void Pong::Arena::VerifyTrail(Lock& guard)
{

    if(TrailKeeper != 0)
        return;

    std::cout << "TODO: trail" << std::endl;
    // DEBUG_BREAK;

    //// These settings are overwritten almost instantly //
    // Trail::Properties balltrailproperties(5, 10, 100, false);
    //
    //// Set up all elements //
    // balltrailproperties.Elements[0] = Trail::ElementProperties(
    //    Float4(0), Float4(0.5f, 0.5f, 0.5f, 0), 3.f, 0.3f);

    //// Create the trail //
    // TrailKeeper = ObjectLoader::LoadTrailToWorld(
    //        TargetWorld, "PongBallTrail", balltrailproperties, true);
}
// ------------------------------------ //
void Pong::Arena::ColourTheBallTrail(Lock& guard, const Float4& colour)
{

    DEBUG_BREAK;
    //// Adjust the trail parameters //
    // Trail::Properties balltrailproperties(5, 10, 100, false);
    //
    //// Set up all elements //
    // balltrailproperties.Elements[0] = Trail::ElementProperties(colour,
    //    Float4(0.7f, 0.7f, 0.7f, 0.3f), 5.f, 0.6f);

    // if(TrailKeeper != 0){

    //    auto& trail = TargetWorld->GetComponent<Trail>(TrailKeeper);

    //    trail.SetTrailProperties(balltrailproperties);
    //}
}
// ------------------------------------ //
void Pong::Arena::LetGoOfBall()
{
    GUARD_LOCK();

    // We should delete it (but after this physics update is done) //
    if(Ball && TargetWorld) {

        Logger::Get()->Info("Arena: destroying old ball");
        TargetWorld->QueueDestroyEntity(Ball);
        Ball = 0;
    }
}

bool Pong::Arena::IsBallInPaddleArea()
{

    GUARD_LOCK();
    // Cast //
    try {

        const auto pos =
            TargetWorld->GetComponent<Leviathan::Position>(Ball).Members._Position;

        if(std::abs(pos.X) >= 8.5f * BASE_ARENASCALE ||
            std::abs(pos.Z) >= 8.5f * BASE_ARENASCALE)
            return true;

    } catch(const NotFound&) {

        return false;
    }

    return false;
}
// ------------------------------------ //
std::string Pong::Arena::GetMaterialNameForPlayerColour(const Float4& colour)
{

    Lock lock(ColourMaterialNameMutex);

    auto iter = ColourMaterialName.find(colour);

    if(iter != ColourMaterialName.end()) {

        return iter->second;
    }

    // Create new //
    // The prefix in the name allows the clients to generate the texture when they need to
    // use it
    std::stringstream generatename;
    generatename << "/Generated/Colour_" << colour << "_PaddleMaterial";

    // Generate (only in graphical mode) //
    if(!Engine::Get()->GetNoGui()) {
        DEBUG_BREAK;
        // Leviathan::TextureGenerator::LoadSolidColourLightMaterialToMemory(
        //     generatename.str(), colour);
    }

    ColourMaterialName[colour] = generatename.str();
    return generatename.str();
}
