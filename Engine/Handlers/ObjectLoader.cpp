// ------------------------------------ //
#include "ObjectLoader.h"

#include "../Common/StringOperations.h"
#include "../Entities/GameWorld.h"
#include "Exceptions.h"
#include "FileSystem.h"
#include "ObjectFiles/ObjectFileProcessor.h"
using namespace Leviathan;
// ------------------------------------ //



// void ObjectLoader::_CreatePropCommon(GameWorld* world, Lock &worldlock,
//    ObjectID prop, const std::string &ogrefile, Model &model, bool hidden)
//{
//
//    world->CreateConstraintable(prop, prop, world);
//
//    // Load the Ogre entity if in graphical mode //
//    auto scene = world->GetScene();
//
//    // Render node is required on the server for hiding things on clients
//    auto& rendernode = world->CreateRenderNode(prop);
//
//    if(hidden){
//
//        rendernode.Marked = true;
//        rendernode.Hidden = true;
//    }
//
//    if(scene && !ogrefile.empty()){
//
//        model.GraphicalObject = scene->createEntity(ogrefile);
//
//        // Create scene node for positioning //
//        rendernode.Node = scene->getRootSceneNode()->createChildSceneNode();
//
//        // attach for deletion and valid display //
//        rendernode.Node->attachObject(model.GraphicalObject);
//    }
//}
//
// void ObjectLoader::_CreatePropPhysics(GameWorld* world, Lock &worldlock, Model &model,
//    Physics &physics, Position &position, ObjectFileList* proplist, const std::string &path,
//    int materialid)
//{
//    // Skip creating if no physical world
//    if(!world->GetPhysicalWorld())
//        return;
//
//    NewtonWorld* tmpworld = world->GetPhysicalWorld()->GetNewtonWorld();
//
//    // get offset //
//    string offsettype;
//    Ogre::Matrix4 offset = Ogre::Matrix4::IDENTITY;
//    // this is useful when the origin is at the bottom of the model and you don't take
//    // this into account in newton primitive (using Convex hull should not require this)
//    if(ObjectFileProcessor::LoadValueFromNamedVars<string>(proplist->GetVariables(),
//            "Offset", offsettype, "", Logger::Get(), "Prop: Init: CreatePhysicsModel"))
//    {
//
//        if(offsettype == "None"){
//            // nothing needs to set //
//
//
//        } else if(offsettype == "BoundingBoxCenter"){
//
//            // There needs to be a workaround in non-graphical mode //
//            if(model.GraphicalObject){
//
//                Ogre::Aabb bbox = model.GraphicalObject->getLocalAabb();
//                offset.setTrans(bbox.mCenter);
//
//            } else {
//
//                Logger::Get()->Error("Prop: Init: trying to use BoundingBoxCenter as "
//                    "offset in non-graphical mode and there is no workaround to "
//                    "calculate that...yet");
//            }
//
//        } else {
//
//            Logger::Get()->Error("Prop: Init: invalid offset type, use None or "
//                "BoundingBoxCenter for most common cases, file: "+path);
//        }
//    }
//
//    // Newton uses different handed matrices...
//    Ogre::Matrix4 toffset = offset.transpose();
//
//    string ptype;
//
//    ObjectFileProcessor::LoadValueFromNamedVars<string>(proplist->GetVariables(),
//        "PrimitiveType", ptype, "Convex");
//
//    // Process first the most complicated one, Convex hull which is basically a prop that
//    // we need to load
//    if(ptype == "Convex"){
//
//        Logger::Get()->Error("Prop: Init: physical object Convex hull loading is not "
//            "implemented!");
//        return;
//
//    } else if(ptype == "Sphere" || ptype == "Ball"){
//
//        // Sphere primitive type, now we just need to load the size parameter or
//        // count it from the object bounding box
//        float radius = 0.f;
//
//        if(!ObjectFileProcessor::LoadValueFromNamedVars<float>(proplist->GetVariables(),
//                "Size", radius, 0.f))
//        {
//            // it should be string type //
//            string sizesourcename;
//
//            if(!ObjectFileProcessor::LoadValueFromNamedVars<string>(
//                    proplist->GetVariables(), "Size", sizesourcename, "", Logger::Get(),
//                    "Prop: Init: CreatePhysicsModel:"))
//            {
//                // Process based on the source name //
//                if(sizesourcename == "GraphicalModel"){
//
//                    // There needs to be a workaround in non-graphical mode //
//                    if(model.GraphicalObject){
//
//                        // Calculate the radius from bounding box size //
//                        Ogre::Aabb bbox = model.GraphicalObject->getLocalAabb();
//                        Ogre::Vector3 sizes = bbox.getSize();
//
//                        // A little sanity check //
//                        if(sizes.x != sizes.y || sizes.x != sizes.z){
//                            // it's not cube //
//                            Logger::Get()->Warning("Prop: Init: physical model sphere, "
//                                "the bounding box of graphical model is not a cube, "
//                                "continuing anyways");
//                        }
//
//                        radius = sizes.x;
//
//                    } else {
//
//                        Logger::Get()->Error("Prop: Init: trying to use GraphicalModel "
//                            "as size specification  but in non-gui mode graphical object "
//                            "isn't loaded and there is no workaround to calculate "
//                            "that...yet");
//                    }
//
//                } else {
//                    Logger::Get()->Error("Prop: Init: physical model has no size! "
//                        "unknown source: "+sizesourcename+
//                        " (\"Size = GraphicalModel;\"), file: "+path);
//                }
//
//            } else {
//
//                Logger::Get()->Error("Prop: Init: physical model has no size! "
//                    "at least specify \"Size = GraphicalModel;\", file: "+path);
//            }
//
//        }
//
//        // Radius should be fine now //
//        if(radius > 0){
//
//            // Create the sphere now //
//            physics.Collision = NewtonCreateSphere(tmpworld, radius, 0, &toffset[0][0]);
//        }
//
//    } else if(ptype == "Box"){
//
//        DEBUG_BREAK;
//        //Collision = NewtonCreateBox(tmpworld, sizes.x, sizes.y, sizes.z, NULL,
//        // &toffset[0][0]);
//    } else {
//        Logger::Get()->Error("Prop: Init: physical model has no type! "
//            "unknown typename: "+ptype+", \"PrimitiveType = Convex;\" for mesh "
//            "collisions, file: "+path);
//    }
//
//    if(physics.Collision){
//
//        GUARD_LOCK_OTHER((&physics));
//
//        // Create the body //
//        Ogre::Matrix4 locrot = Ogre::Matrix4::IDENTITY;
//        locrot.makeTransform(position._Position, Float3(1, 1, 1), position._Orientation);
//
//        Ogre::Matrix4 tlocrot = locrot.transpose();
//
//        physics.Body = NewtonCreateDynamicBody(tmpworld, physics.Collision,
//            &tlocrot[0][0]);
//
//        // Add this as user data //
//        NewtonBodySetUserData(physics.Body, &physics);
//
//        // Get mass //
//        physics.Mass = 0;
//        ObjectFileProcessor::LoadValueFromNamedVars<float>(proplist->GetVariables(),
//            "Mass", physics.Mass, 0.f, Logger::Get(), "Prop: Init: CreatePhysicsModel:");
//
//        // First calculate inertia and center of mass points //
//        Float3 inertia;
//        Float3 centerofmass;
//
//        NewtonConvexCollisionCalculateInertialMatrix(physics.Collision, &inertia.X,
//            &centerofmass.X);
//
//        // Apply mass to inertia
//        inertia *= physics.Mass;
//
//        NewtonBodySetMassMatrix(physics.Body, physics.Mass, inertia.X, inertia.Y,
//            inertia.Z);
//        NewtonBodySetCentreOfMass(physics.Body, &centerofmass.X);
//
//
//        // Callbacks //
//        NewtonBodySetTransformCallback(physics.Body,
//            Physics::PhysicsMovedEvent);
//
//        NewtonBodySetForceAndTorqueCallback(physics.Body,
//            Physics::ApplyForceAndTorgueEvent);
//
//        NewtonBodySetDestructorCallback(physics.Body, Physics::DestroyBodyCallback);
//
//        // TODO: create test that verifies that default material id = 0
//        if(materialid > 0)
//            physics.SetPhysicalMaterialID(guard, materialid);
//    }
//}
//
// DLLEXPORT bool ObjectLoader::LoadNetworkProp(GameWorld* world, Lock &worldlock,
//    ObjectID id, const std::string &modelfile, int materialid,
//    const Position::PositionData &pos, bool hidden)
//{
//    // Get relative path //
//	std::string path = FileSystem::Get()->SearchForFile(FILEGROUP_MODEL,
//        StringOperations::RemoveExtensionString(modelfile, true), "levmd", true);
//
//	// Parse the file //
//	auto file = ObjectFileProcessor::ProcessObjectFile(path, Logger::Get());
//
//	if(!file){
//
//        Logger::Get()->Error("LoadNetworkProp: failed to parse prop file "+path);
//		return false;
//	}
//
//    ObjectID prop = id;
//
//    string ogrefile;
//
//	ObjectFileProcessor::LoadValueFromNamedVars<string>(file->GetVariables(),
//"Model-Graphical",
//        ogrefile, "", Logger::Get(), "Prop: Init: no model file");
//
//
//    // Setup the model //
//    world->CreateReceived(prop, SENDABLE_TYPE_PROP);
//
//    auto& position = world->CreatePosition(prop, pos._Position, pos._Orientation);
//
//    auto& model = world->CreateModel(prop, path);
//
//    world->CreateParentable(prop);
//
//    _CreatePropCommon(world, worldlock, prop, ogrefile, model, hidden);
//
//	// Find the physics object definition //
//	auto phyobj = file->GetObjectWithType("PhysicalModel");
//
//	if(phyobj){
//
//        auto proplist = phyobj->GetListWithName("properties");
//
//        if(proplist){
//
//            auto& physics = world->CreatePhysics(prop, prop, world, position, nullptr);
//
//            _CreatePropPhysics(world, worldlock, model, physics, position, proplist, path,
//                materialid);
//
//
//        } else {
//
//            Logger::Get()->Warning("Model file "+path+" has PhysicalModel but no properties
//            list");
//        }
//	}
//
//    // Notify that it has been created //
//    world->NotifyEntityCreate(worldlock, prop);
//
//    return true;
//}
//
// DLLEXPORT ObjectID Leviathan::ObjectLoader::LoadPropToWorld(GameWorld* world, Lock
// &worldlock,
//    const std::string &name, int materialid, const Position::PositionData &pos)
//{
//    // Get relative path //
//	std::string path = FileSystem::Get()->SearchForFile(FILEGROUP_MODEL, name, "levmd", false);
//
//	// Parse the file //
//	auto file = ObjectFileProcessor::ProcessObjectFile(path, Logger::Get());
//
//	if(!file){
//
//        Logger::Get()->Error("LoadProp: failed to parse prop file "+path);
//		return 0;
//	}
//
//    ObjectID prop = world->CreateEntity(worldlock);
//
//    string ogrefile;
//
//	ObjectFileProcessor::LoadValueFromNamedVars<string>(file->GetVariables(),
//"Model-Graphical",
//        ogrefile, "", Logger::Get(), "Prop: Init: no model file");
//
//
//	// Setup the model //
//    auto& sendable = world->CreateSendable(prop, SENDABLE_TYPE_PROP);
//
//    auto& position = world->CreatePosition(prop, pos._Position, pos._Orientation);
//
//    auto& model = world->CreateModel(prop, path);
//
//    world->CreateParentable(prop);
//
//    _CreatePropCommon(world, worldlock, prop, ogrefile, model, false);
//
//	// Find the physics object definition //
//	auto phyobj = file->GetObjectWithType("PhysicalModel");
//
//	if(phyobj){
//
//        auto proplist = phyobj->GetListWithName("properties");
//
//        if(proplist){
//
//            auto& physics = world->CreatePhysics(prop, prop, world, position, &sendable);
//
//            _CreatePropPhysics(world, worldlock, model, physics, position, proplist, path,
//                materialid);
//
//        } else {
//
//            Logger::Get()->Warning("Model file "+path+" has PhysicalModel but no properties
//            list");
//        }
//	}
//
//    // Notify that it has been created //
//    world->NotifyEntityCreate(worldlock, prop);
//
//	return prop;
//}
//// ------------------------------------ //
// void ObjectLoader::_CreateBrushModel(GameWorld* world, Lock &worldlock, ObjectID brush,
//    Physics &physics, BoxGeometry &box, Position &position, float mass, const Float3 &size,
//    bool hidden)
//{
//
//	// Load the Ogre entity if in graphical mode //
//    auto scene = world->GetScene();
//
//    // Server might want to hide this on the client
//    auto& rendernode = world->CreateRenderNode(brush);
//
//    if(hidden){
//
//        rendernode.Marked = true;
//        rendernode.Hidden = true;
//    }
//
//    if(scene){
//
//        // Use an unique name for the mesh //
//        auto& manual = world->CreateManualObject(brush);
//
//        // Create the graphical box //
//
//        // We do not want to update this later //
//        manual.Object->setDynamic(false);
//        manual.Object->estimateVertexCount(24);
//        manual.Object->estimateIndexCount(24);
//
//        // Create mesh name //
//        manual.CreatedMesh = "_BoxGeometry_manual_"+Convert::ToString(brush);
//
//        manual.Object->begin(box.Material, Ogre::RenderOperation::OT_TRIANGLE_LIST);
//
//        // Bottom left //
//        // Bottom //
//        manual.Object->position(size.X/-2.f, size.Y/-2.f, size.Z/-2.f);
//        manual.Object->textureCoord(Ogre::Vector2(0.f, 0.f));
//        manual.Object->normal(Ogre::Vector3(0.f, -1.f, 0.f));
//
//        // second is the face that is on the right when looking from the
//        // corner to the center of the cube
//        manual.Object->position(size.X/-2.f, size.Y/-2.f, size.Z/-2.f);
//        manual.Object->textureCoord(Ogre::Vector2(0.f, 1.f));
//        manual.Object->normal(Ogre::Vector3(0.f, 0.f, -1.f));
//
//        // and third is on the left when looking to the center //
//        manual.Object->position(size.X/-2.f, size.Y/-2.f, size.Z/-2.f);
//        manual.Object->textureCoord(Ogre::Vector2(1.f, 1.f));
//        manual.Object->normal(Ogre::Vector3(-1.f, 0.f, 0.f));
//
//
//        // right point //
//        // first is bottom or top face //
//        manual.Object->position(size.X/2.f, size.Y/-2.f, size.Z/-2.f);
//        manual.Object->textureCoord(Ogre::Vector2(1.f, 0.f));
//        manual.Object->normal(Ogre::Vector3(0.f, -1.f, 0.f));
//
//        // second is the face that is on the right when looking from the
//        // corner to the center of the cube
//        manual.Object->position(size.X/2.f, size.Y/-2.f, size.Z/-2.f);
//        manual.Object->textureCoord(Ogre::Vector2(0.f, 1.f));
//        manual.Object->normal(Ogre::Vector3(1.f, 0.f, 0.f));
//
//        // and third is on the left when looking to the center //
//        manual.Object->position(size.X/2.f, size.Y/-2.f, size.Z/-2.f);
//        manual.Object->textureCoord(Ogre::Vector2(1.f, 1.f));
//        manual.Object->normal(Ogre::Vector3(0.f, 0.f, -1.f));
//
//
//        // right up //
//        // first is bottom or top face //
//        manual.Object->position(size.X/2.f, size.Y/-2.f, size.Z/2.f);
//        manual.Object->textureCoord(Ogre::Vector2(1.f, 1.f));
//        manual.Object->normal(Ogre::Vector3(0.f, -1.f, 0.f));
//
//        // second is the face that is on the right when looking from the corner
//        // to the center of the cube
//        manual.Object->position(size.X/2.f, size.Y/-2.f, size.Z/2.f);
//        manual.Object->textureCoord(Ogre::Vector2(0.f, 1.f));
//        manual.Object->normal(Ogre::Vector3(0.f, 0.f, 1.f));
//
//        // and third is on the left when looking to the center //
//        manual.Object->position(size.X/2.f, size.Y/-2.f, size.Z/2.f);
//        manual.Object->textureCoord(Ogre::Vector2(1.f, 1.f));
//        manual.Object->normal(Ogre::Vector3(1.f, 0.f, 0.f));
//
//
//        // left up //
//        // first is bottom or top face //
//        manual.Object->position(size.X/-2.f, size.Y/-2.f, size.Z/2.f);
//        manual.Object->textureCoord(Ogre::Vector2(0.f, 1.f));
//        manual.Object->normal(Ogre::Vector3(0.f, -1.f, 0.f));
//
//        // second is the face that is on the right when looking from the corner to
//        // the center of the cube
//        manual.Object->position(size.X/-2.f, size.Y/-2.f, size.Z/2.f);
//        manual.Object->textureCoord(Ogre::Vector2(0.f, 1.f));
//        manual.Object->normal(Ogre::Vector3(-1.f, 0.f, 0.f));
//
//        // and third is on the left when looking to the center //
//        manual.Object->position(size.X/-2.f, size.Y/-2.f, size.Z/2.f);
//        manual.Object->textureCoord(Ogre::Vector2(1.f, 1.f));
//        manual.Object->normal(Ogre::Vector3(0.f, 0.f, 1.f));
//
//        /// The other half ///
//
//        // Bottom left //
//        // Bottom //
//        manual.Object->position(size.X/-2.f, size.Y/2.f, size.Z/-2.f);
//        manual.Object->textureCoord(Ogre::Vector2(0.f, 0.f));
//        manual.Object->normal(Ogre::Vector3(0.f, 1.f, 0.f));
//
//        // second is the face that is on the right when looking from the
//        // corner to the center of the cube
//        manual.Object->position(size.X/-2.f, size.Y/2.f, size.Z/-2.f);
//        manual.Object->textureCoord(Ogre::Vector2(0.f, 0.f));
//        manual.Object->normal(Ogre::Vector3(0.f, 0.f, -1.f));
//
//        // and third is on the left when looking to the center //
//        manual.Object->position(size.X/-2.f, size.Y/2.f, size.Z/-2.f);
//        manual.Object->textureCoord(Ogre::Vector2(1.f, 0.f));
//        manual.Object->normal(Ogre::Vector3(-1.f, 0.f, 0.f));
//
//
//        // right point //
//        // first is bottom or top face //
//        manual.Object->position(size.X/2.f, size.Y/2.f, size.Z/-2.f);
//        manual.Object->textureCoord(Ogre::Vector2(1.f, 0.f));
//        manual.Object->normal(Ogre::Vector3(0.f, 1.f, 0.f));
//
//        // second is the face that is on the right when looking from the
//        // corner to the center of the cube
//        manual.Object->position(size.X/2.f, size.Y/2.f, size.Z/-2.f);
//        manual.Object->textureCoord(Ogre::Vector2(0.f, 0.f));
//        manual.Object->normal(Ogre::Vector3(1.f, 0.f, 0.f));
//
//        // and third is on the left when looking to the center //
//        manual.Object->position(size.X/2.f, size.Y/2.f, size.Z/-2.f);
//        manual.Object->textureCoord(Ogre::Vector2(1.f, 0.f));
//        manual.Object->normal(Ogre::Vector3(0.f, 0.f, -1.f));
//
//
//        // right up //
//        // first is bottom or top face //
//        manual.Object->position(size.X/2.f, size.Y/2.f, size.Z/2.f);
//        manual.Object->textureCoord(Ogre::Vector2(1.f, 1.f));
//        manual.Object->normal(Ogre::Vector3(0.f, 1.f, 0.f));
//
//        // second is the face that is on the right when looking from the corner
//        // to the center of the cube
//        manual.Object->position(size.X/2.f, size.Y/2.f, size.Z/2.f);
//        manual.Object->textureCoord(Ogre::Vector2(0.f, 0.f));
//        manual.Object->normal(Ogre::Vector3(0.f, 0.f, 1.f));
//
//        // and third is on the left when looking to the center //
//        manual.Object->position(size.X/2.f, size.Y/2.f, size.Z/2.f);
//        manual.Object->textureCoord(Ogre::Vector2(1.f, 0.f));
//        manual.Object->normal(Ogre::Vector3(1.f, 0.f, 0.f));
//
//
//        // left up //
//        // first is bottom or top face //
//        manual.Object->position(size.X/-2.f, size.Y/2.f, size.Z/2.f);
//        manual.Object->textureCoord(Ogre::Vector2(0.f, 1.f));
//        manual.Object->normal(Ogre::Vector3(0.f, 1.f, 0.f));
//
//        // second is the face that is on the right when looking from the corner to
//        // the center of the cube
//        manual.Object->position(size.X/-2.f, size.Y/2.f, size.Z/2.f);
//        manual.Object->textureCoord(Ogre::Vector2(0.f, 0.f));
//        manual.Object->normal(Ogre::Vector3(-1.f, 0.f, 0.f));
//
//        // and third is on the left when looking to the center //
//        manual.Object->position(size.X/-2.f, size.Y/2.f, size.Z/2.f);
//        manual.Object->textureCoord(Ogre::Vector2(1.f, 0.f));
//        manual.Object->normal(Ogre::Vector3(0.f, 0.f, 1.f));
//
//
//        // quads are both same //
//        // base //
//        manual.Object->quad(0, 3, 6, 9);
//
//        // front side //
//        manual.Object->quad(13, 17, 5, 1);
//
//        // right side //
//        manual.Object->quad(16, 20, 8, 4);
//
//        // left side //
//        manual.Object->quad(2, 10, 22, 14);
//
//        // back side //
//        manual.Object->quad(19, 23, 11, 7);
//
//        // top //
//        manual.Object->quad(21, 18, 15, 12);
//
//        // end and turn into a mesh //
//        manual.Object->end();
//        manual.Object->convertToMesh(manual.CreatedMesh,
//            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
//
//        // create instance //
//        // load the Ogre entity //
//        box.GraphicalObject = scene->createEntity(manual.CreatedMesh);
//
//        // create scene node for positioning //
//        rendernode.Node = scene->getRootSceneNode()->createChildSceneNode();
//
//        // attach for deletion and valid display //
//        rendernode.Node->attachObject(box.GraphicalObject);
//    }
//
//    // Create physical box //
//
//    physics.Mass = mass;
//
//    // Skip using physics if not available //
//    if(world->GetPhysicalWorld()){
//
//        // Create a newton object which is always a box //
//        NewtonWorld* tmpworld = world->GetPhysicalWorld()->GetNewtonWorld();
//
//        // Possible offset //
//        Ogre::Matrix4 offset = Ogre::Matrix4::IDENTITY;
//        Ogre::Matrix4 toffset = offset.transpose();
//
//        physics.Collision = NewtonCreateBox(tmpworld, size.X, size.Y, size.Z, 0,
//        &toffset[0][0]);
//
//        Ogre::Matrix4 matrix;
//        matrix.makeTransform(position._Position, Float3(1, 1, 1), position._Orientation);
//
//        Ogre::Matrix4 tmatrix = matrix.transpose();
//
//        physics.Body = NewtonCreateDynamicBody(tmpworld, physics.Collision, &tmatrix[0][0]);
//
//        // Add this as user data //
//        NewtonBodySetUserData(physics.Body, &physics);
//
//        // Set as movable if has mass //
//        if(mass != 0){
//
//            // First calculate inertia and center of mass points //
//            Float3 inertia;
//            Float3 centerofmass;
//
//            NewtonConvexCollisionCalculateInertialMatrix(physics.Collision, &inertia.X,
//                &centerofmass.X);
//
//            // Apply mass to inertia
//            inertia *= mass;
//
//            NewtonBodySetMassMatrix(physics.Body, mass, inertia.X, inertia.Y, inertia.Z);
//            NewtonBodySetCentreOfMass(physics.Body, &centerofmass.X);
//
//            NewtonBodySetForceAndTorqueCallback(physics.Body,
//            Physics::ApplyForceAndTorgueEvent);
//
//        }
//
//        // Callbacks //
//        NewtonBodySetTransformCallback(physics.Body, Physics::PhysicsMovedEvent);
//        NewtonBodySetDestructorCallback(physics.Body, Physics::DestroyBodyCallback);
//    }
//}
//
// DLLEXPORT bool ObjectLoader::LoadNetworkBrush(GameWorld* world, Lock &worldlock,
//    ObjectID id, const std::string &material, const Float3 &size, const float &mass,
//    int materialid, const Position::PositionData &pos, bool hidden)
//{
//    ObjectID brush = id;
//
//    auto& position = world->CreatePosition(brush, pos._Position, pos._Orientation);
//
//    auto& box = world->CreateBoxGeometry(brush, size, material);
//
//    world->CreateReceived(brush, SENDABLE_TYPE_BRUSH);
//
//    world->CreateConstraintable(brush, brush, world);
//
//    world->CreateParentable(brush);
//
//    auto& physics = world->CreatePhysics(brush, brush, world, position, nullptr);
//
//    _CreateBrushModel(world, worldlock, brush, physics, box, position, mass, size, hidden);
//
//    // Notify that it has been created //
//    world->NotifyEntityCreate(worldlock, brush);
//
//    return true;
//}
//
// DLLEXPORT ObjectID Leviathan::ObjectLoader::LoadBrushToWorld(GameWorld* world, Lock
// &worldlock,
//    const std::string &material, const Float3 &size, const float &mass, int materialid,
//    const Position::PositionData &pos)
//{
//
//    ObjectID brush = world->CreateEntity(worldlock);
//
//	// Setup the model //
//
//    auto& position = world->CreatePosition(brush, pos._Position, pos._Orientation);
//
//    auto& box = world->CreateBoxGeometry(brush, size, material.size() ? material:
//    "BaseWhiteNoLighting");
//
//    auto& sendable = world->CreateSendable(brush, SENDABLE_TYPE_BRUSH);
//
//    world->CreateParentable(brush);
//
//    world->CreateConstraintable(brush, brush, world);
//
//    auto& physics = world->CreatePhysics(brush, brush, world, position, &sendable);
//
//    _CreateBrushModel(world, worldlock, brush, physics, box, position, mass, size, false);
//
//    // Notify that it has been created //
//    world->NotifyEntityCreate(worldlock, brush);
//
//	return brush;
//}
//// ------------------------------------ //
// DLLEXPORT ObjectID Leviathan::ObjectLoader::LoadTrackControllerToWorld(GameWorld* world,
//    Lock &worldlock, std::vector<Position::PositionData> &initialtrack)
//{
//
//    ObjectID controller = world->CreateEntity(worldlock);
//
//    auto& owner = world->CreatePositionMarkerOwner(controller);
//    world->CreateParent(controller);
//    auto& sendable = world->CreateSendable(controller, SENDABLE_TYPE_TRACKCONTROLLER);
//    world->CreateTrackController(controller, owner, &sendable);
//
//	// Set nodes //
//	for(auto iter = initialtrack.begin(); iter != initialtrack.end(); ++iter){
//        // Create entity //
//        // TODO: make sure that these are cleaned up once no longer needed
//        auto created = world->CreateEntity(worldlock);
//
//        auto& pos = world->CreatePosition(created, iter->_Position, iter->_Orientation);
//
//		// Add position //
//		owner.Add(created, pos);
//	}
//
//
//    // Notify that it has been created //
//    world->NotifyEntityCreate(worldlock, controller);
//
//	return controller;
//}
//
// DLLEXPORT  bool Leviathan::ObjectLoader::LoadNetworkTrackController(GameWorld* world, Lock
// &worldlock,
//    ObjectID id, size_t reachednode, float nodeprogress, float changespeed, float applyforce,
//    const Parent::Data &childrendata, const PositionMarkerOwner::Data &positions)
//{
//
//    ObjectID controller = id;
//
//    auto& owner = world->CreatePositionMarkerOwner(controller, positions, world, worldlock);
//    world->CreateParent(controller, childrendata, world, worldlock);
//    auto& sendable = world->CreateSendable(controller, SENDABLE_TYPE_TRACKCONTROLLER);
//    world->CreateTrackController(controller, owner, &sendable, reachednode, nodeprogress,
//        changespeed, applyforce);
//
//    // Notify that it has been created //
//    world->NotifyEntityCreate(worldlock, controller);
//
//	return true;
//}
//// ------------------------------------ //
// DLLEXPORT ObjectID Leviathan::ObjectLoader::LoadTrailToWorld(GameWorld* world, Lock
// &worldlock,
//    const std::string &material, const Trail::Properties &properties, bool allowupdatelater,
//    const Position::PositionData &pos)
//{
//
//    ObjectID entity = world->CreateEntity(worldlock);
//
//    // TODO: add sendable here
//    world->CreatePosition(entity, pos._Position, pos._Orientation);
//
//    // TODO: sendable
//
//    auto scene = world->GetScene();
//
//    if(scene){
//
//        auto& rendernode = world->CreateRenderNode(entity);
//
//        auto& trail = world->CreateTrail(entity, &rendernode, material, properties);
//
//        rendernode.Node = scene->getRootSceneNode()->createChildSceneNode();
//
//        // Trail entity //
//        trail.TrailEntity = scene->createRibbonTrail();
//        trail.TrailEntity->setName("TrailEmitter_"+Convert::ToString(entity));
//        trail.TrailEntity->setMaterialName(material);
//
//        // Set dynamic update if wanted //
//        if(allowupdatelater){
//            trail.TrailEntity->setDynamic(true);
//        }
//
//        // Add to root node to include in the scene //
//        scene->getRootSceneNode()->attachObject(trail.TrailEntity);
//
//        // Apply the settings, this also adds the node to the trail //
//        trail.SetTrailProperties(properties, true);
//
//    } else {
//
//        world->CreateTrail(entity, nullptr, material, properties);
//    }
//
//    // Notify that it has been created //
//    world->NotifyEntityCreate(worldlock, entity);
//
//	return entity;
//}
// ------------------------------------ //
