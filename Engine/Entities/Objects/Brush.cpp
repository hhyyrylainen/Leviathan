// ------------------------------------ //
#ifndef LEVIATHAN_ENTITY_BRUSH
#include "Brush.h"
#endif
#include "OgreManualObject.h"
#include "OgreMeshManager.h"
#include "FileSystem.h"
#include "OgreSceneManager.h"
#include "OgreSceneNode.h"
#include "OgreEntity.h"
#include "Entities/CommonStateObjects.h"
using namespace Leviathan;
using namespace Entity;
// ------------------------------------ //
//#define BRUSH_CALCULATENORMALS		1



DLLEXPORT Leviathan::Entity::Brush::Brush(bool hidden, GameWorld* world) :
    BaseRenderable(hidden), BaseObject(IDFactory::GetID(), world), BaseSendableEntity(BASESENDABLE_ACTUAL_TYPE_BRUSH),
    Sizes(0), BrushModel(NULL), Mass(0.f)
{

}

DLLEXPORT Leviathan::Entity::Brush::Brush(bool hidden, GameWorld* world, int netid) :
    BaseRenderable(hidden), BaseObject(netid, world), BaseSendableEntity(BASESENDABLE_ACTUAL_TYPE_BRUSH),
    Sizes(0), BrushModel(NULL), Mass(0.f)
{

}


DLLEXPORT Leviathan::Entity::Brush::~Brush(){

    ReleaseParentHooks();
}

DLLEXPORT void Leviathan::Entity::Brush::ReleaseData(){

    StopInterpolating();

    GUARD_LOCK_THIS_OBJECT();
    
	// Release Ogre entity //
    if(ObjectsNode)
        OwnedByWorld->GetScene()->destroySceneNode(ObjectsNode);
    if(GraphicalObject)
        OwnedByWorld->GetScene()->destroyEntity(GraphicalObject);

	ObjectsNode = NULL;
	GraphicalObject = NULL;

	// The model won't be used anymore //
	if(MeshName.size() && OwnedByWorld->GetScene()){
		OwnedByWorld->GetScene()->destroyManualObject(BrushModel);
	}

	OwnedByWorld = NULL;

	// Physical entity //
	AggressiveConstraintUnlink();
	_DestroyPhysicalBody();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Entity::Brush::Init(const Float3 &dimensions, const string &material, 
	bool createphysics /*= true*/)
{
	Sizes = dimensions;

    // This is needed later by the network sending functionality //
    Material = material;

    // Force material to be default if not specified //
    if(Material.empty())
        Material = "BaseWhiteNoLighting";
    
	// Create an unique name for mesh //
	MeshName = "Brush_"+Convert::ToString(ID);

    // Opt out of graphics if in non-gui mode //
    if(!OwnedByWorld->GetScene()){

        goto brushpostgraphicalobjectcreation;
    }
    
    {
        // create the graphical box //
        BrushModel = OwnedByWorld->GetScene()->createManualObject();
        BrushModel->setName(MeshName+"_manual");

        // we do not want to update this later //
        BrushModel->setDynamic(false);
        BrushModel->estimateVertexCount(24);
        BrushModel->estimateIndexCount(24);
        
#ifdef BRUSH_CALCULATENORMALS
        std::vector<Float3> tmpvertices;
        tmpvertices.reserve(24);
#endif // BRUSH_CALCULATENORMALS


        BrushModel->begin(Material, Ogre::RenderOperation::OT_TRIANGLE_LIST);


        // loops to avoid redundant code //
        float yval = dimensions.Y/-2.f;
        bool up = false;

        for(int i = 0; i < 2; i++){
            // loop three times on all points for each side that has that point //

            // bottom left //
            for(int a = 0; a < 3; a++){

                BrushModel->position(dimensions.X/-2.f, yval, dimensions.Z/-2.f);
#ifdef BRUSH_CALCULATENORMALS
                tmpvertices.push_back(Float3(dimensions.X/-2.f, yval, dimensions.Z/-2.f));
#endif // BRUSH_CALCULATENORMALS
					
                // bottom left //
                if(a == 0){

                    BrushModel->textureCoord(Ogre::Vector2(0.f, 0.f));
                    BrushModel->normal(Float3(0.f, -1.f*(i != 0 ? -1.f: 1.f), 0.f));
						
                } else if (a == 1){
                    // second is the face that is on the right when looking from the corner to the center of the cube //
                    if(!up)
                        BrushModel->textureCoord(Ogre::Vector2(0.f, 1.f));
                    else
                        BrushModel->textureCoord(Ogre::Vector2(0.f, 0.f));

                    BrushModel->normal(Float3(0.f, 0.f, -1.f));

                } else {
                    // and third is on the left when looking to the center //
                    if(!up)
                        BrushModel->textureCoord(Ogre::Vector2(1.f, 1.f));
                    else
                        BrushModel->textureCoord(Ogre::Vector2(1.f, 0.f));

                    BrushModel->normal(Float3(-1.f, 0.f, 0.f));
                }
            }

            // right point //
            for(int a = 0; a < 3; a++){

                BrushModel->position(dimensions.X/2.f, yval, dimensions.Z/-2.f);
#ifdef BRUSH_CALCULATENORMALS
                tmpvertices.push_back(Float3(dimensions.X/2.f, yval, dimensions.Z/-2.f));
#endif // BRUSH_CALCULATENORMALS


                // first is bottom or top face //
                if(a == 0){

                    BrushModel->textureCoord(Ogre::Vector2(1.f, 0.f));
                    BrushModel->normal(Float3(0.f, -1.f*(i != 0 ? -1.f: 1.f), 0.f));

                } else if (a == 1){
                    // second is the face that is on the right when looking from the corner to the center of the cube //
                    if(!up)
                        BrushModel->textureCoord(Ogre::Vector2(0.f, 1.f));
                    else
                        BrushModel->textureCoord(Ogre::Vector2(0.f, 0.f));

                    BrushModel->normal(Float3(1.f, 0.f, 0.f));

                } else {
                    // and third is on the left when looking to the center //
                    if(!up)
                        BrushModel->textureCoord(Ogre::Vector2(1.f, 1.f));
                    else
                        BrushModel->textureCoord(Ogre::Vector2(1.f, 0.f));

                    BrushModel->normal(Float3(0.f, 0.f, -1.f));
                }
            }

            // right up //
            for(int a = 0; a < 3; a++){

                BrushModel->position(dimensions.X/2.f, yval, dimensions.Z/2.f);
#ifdef BRUSH_CALCULATENORMALS
                tmpvertices.push_back(Float3(dimensions.X/2.f, yval, dimensions.Z/2.f));
#endif // BRUSH_CALCULATENORMALS


                // first is bottom or top face //
                if(a == 0){

                    BrushModel->textureCoord(Ogre::Vector2(1.f, 1.f));
                    BrushModel->normal(Float3(0.f, -1.f*(i != 0 ? -1.f: 1.f), 0.f));

                } else if (a == 1){
                    // second is the face that is on the right when looking from the corner to the center of the cube //
                    if(!up)
                        BrushModel->textureCoord(Ogre::Vector2(0.f, 1.f));
                    else
                        BrushModel->textureCoord(Ogre::Vector2(0.f, 0.f));

                    BrushModel->normal(Float3(0.f, 0.f, 1.f));

                } else {
                    // and third is on the left when looking to the center //
                    if(!up)
                        BrushModel->textureCoord(Ogre::Vector2(1.f, 1.f));
                    else
                        BrushModel->textureCoord(Ogre::Vector2(1.f, 0.f));

                    BrushModel->normal(Float3(1.f, 0.f, 0.f));

                }
            }
            // left up //
            for(int a = 0; a < 3; a++){

                BrushModel->position(dimensions.X/-2.f, yval, dimensions.Z/2.f);
#ifdef BRUSH_CALCULATENORMALS
                tmpvertices.push_back(Float3(dimensions.X/-2.f, yval, dimensions.Z/2.f));
#endif // BRUSH_CALCULATENORMALS


                // first is bottom or top face //
                if(a == 0){

                    BrushModel->textureCoord(Ogre::Vector2(0.f, 1.f));
                    BrushModel->normal(Float3(0.f, -1.f*(i != 0 ? -1.f: 1.f), 0.f));

                } else if (a == 1){
                    // second is the face that is on the right when looking from the corner to the center of the cube //
                    if(!up)
                        BrushModel->textureCoord(Ogre::Vector2(0.f, 1.f));
                    else
                        BrushModel->textureCoord(Ogre::Vector2(0.f, 0.f));

                    BrushModel->normal(Float3(-1.f, 0.f, 0.f));

                } else {
                    // and third is on the left when looking to the center //
                    if(!up)
                        BrushModel->textureCoord(Ogre::Vector2(1.f, 1.f));
                    else
                        BrushModel->textureCoord(Ogre::Vector2(1.f, 0.f));

                    BrushModel->normal(Float3(0.f, 0.f, 1.f));
                }
            }
				
            // move to next layer for next loop //
            yval = dimensions.Y/2.f;
            up = true;
        }

        // quads are both same //
        // base //
        BrushModel->quad(0, 3, 6, 9);

        // front side //
        BrushModel->quad(13, 17, 5, 1);
        //TestModel->quad(1, 13, 17, 5);

        // right side //
        BrushModel->quad(16, 20, 8, 4);
        //TestModel->quad(4, 16, 20, 8);

        // left side //
        BrushModel->quad(2, 10, 22, 14);
        //TestModel->quad(10, 22, 14, 2);

        // back side //
        BrushModel->quad(19, 23, 11, 7);
        //TestModel->quad(7, 19, 23, 11);

        // top //
        BrushModel->quad(21, 18, 15, 12);
        //TestModel->quad(12, 21, 18, 15);


#ifdef BRUSH_CALCULATENORMALS
        // calculate normals and save //


        wstring filetext = L"";

        std::vector<Float3> normals(6);


        // calculate normals and store results according to indexes in above quads //
        normals[0] = MMath::CalculateNormal(tmpvertices[0], tmpvertices[3], tmpvertices[6]);
        normals[1] = MMath::CalculateNormal(tmpvertices[13], tmpvertices[17], tmpvertices[5]);
        normals[2] = MMath::CalculateNormal(tmpvertices[16], tmpvertices[20], tmpvertices[8]);
        normals[3] = MMath::CalculateNormal(tmpvertices[2], tmpvertices[10], tmpvertices[22]);
        normals[4] = MMath::CalculateNormal(tmpvertices[19], tmpvertices[23], tmpvertices[11]);
        normals[5] = MMath::CalculateNormal(tmpvertices[21], tmpvertices[18], tmpvertices[15]);


        for(size_t i = 0; i < normals.size(); i++){

            // space after a face //
            filetext += L"\nstarting face number: "+Convert::ToWstring(i)+L"\n";
		

            filetext += L"normal: "+Convert::ToWstring(normals[i].X)+L", "+Convert::ToWstring(normals[i].Y)+L", "
                +Convert::ToWstring(normals[i].Z)+L"\n";

        }


	FileSystem::WriteToFile(filetext, L"Brush_Normals_"+Convert::ToWstring(ID)+L".txt");

#endif // BRUSH_CALCULATENORMALS

	// end and turn into a mesh //
	BrushModel->end();
	BrushModel->convertToMesh(MeshName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	// create instance //
	// load the Ogre entity //
	GraphicalObject = OwnedByWorld->GetScene()->createEntity(MeshName);

	// create scene node for positioning //
	ObjectsNode = OwnedByWorld->GetScene()->getRootSceneNode()->createChildSceneNode();

	// attach for deletion and valid display //
	ObjectsNode->attachObject(GraphicalObject);
}

brushpostgraphicalobjectcreation:
    

	// create physical box if wanted //
	if(createphysics)
		AddPhysicalObject(0.f);

	return true;
}

DLLEXPORT void Leviathan::Entity::Brush::AddPhysicalObject(const float &mass /*= 0.f*/){
	// destroy old first //
	AggressiveConstraintUnlink();
	_DestroyPhysicalBody();

    GUARD_LOCK_THIS_OBJECT();
    
    // Store the mass //
    Mass = mass;

	// create a newton object which is always a box //

	NewtonWorld* tmpworld = OwnedByWorld->GetPhysicalWorld()->GetNewtonWorld();

	// possible offset //
	Ogre::Matrix4 offset = Ogre::Matrix4::IDENTITY;
	Ogre::Matrix4 toffset = offset.transpose();

	Collision = NewtonCreateBox(tmpworld, Sizes.X, Sizes.Y, Sizes.Z, 0, &toffset[0][0]);

	Ogre::Matrix4 matrix;
	matrix.makeTransform(Position, Float3(1, 1, 1), QuatRotation);

	Ogre::Matrix4 tmatrix = matrix.transpose();

	Body = NewtonCreateDynamicBody(tmpworld, Collision, &tmatrix[0][0]);
	// set location //
	_UpdatePhysicsObjectLocation(guard);

	// add this as user data //
	NewtonBodySetUserData(Body, static_cast<BasePhysicsObject*>(this));

	// set as movable if has mass //
	if(mass != 0){

		// first calculate inertia and center of mass points //
		Float3 inertia;
		Float3 centerofmass;

		NewtonConvexCollisionCalculateInertialMatrix(Collision, &inertia.X, &centerofmass.X);
		// set mass //
		// apply mass to inertia 
		inertia *= mass;

		Immovable = false;

		NewtonBodySetMassMatrix(Body, mass, inertia.X, inertia.Y, inertia.Z);
		NewtonBodySetCentreOfMass(Body, &centerofmass.X);

		// gravity callback //
		NewtonBodySetForceAndTorqueCallback(Body, BasePhysicsObject::ApplyForceAndTorgueEvent);
        
	} else {
        
		Immovable = true;
	}

	// callbacks //
	NewtonBodySetTransformCallback(Body, Brush::BrushPhysicsMovedEvent);
	NewtonBodySetDestructorCallback(Body, BasePhysicsObject::DestroyBodyCallback);
}
// ------------------------------------ //
void Leviathan::Entity::Brush::_UpdatePhysicsObjectLocation(ObjectLock &guard){
	// Update physics object location which will in turn change graphical object location //

	Ogre::Matrix4 matrix;
	matrix.makeTransform(Position, Float3(1, 1, 1), QuatRotation);

	Ogre::Matrix4 tmatrix = matrix.transpose();

	// Update body //
	NewtonBodySetMatrix(Body, &tmatrix[0][0]);
    
    if(ObjectsNode){
        // Update graphical object location to have it always match up //
        ObjectsNode->setOrientation(QuatRotation);
        ObjectsNode->setPosition(Position);
    }

    
	// Update potential children //
	_ParentableNotifyLocationDataUpdated();

    // Notify network of new position //
    _MarkDataUpdated(guard);
}
// ------------------------------------ //
void Leviathan::Entity::Brush::BrushPhysicsMovedEvent(const NewtonBody* const body, const dFloat* const matrix,
    int threadIndex)
{
	// First create Ogre 4x4 matrix from the matrix //
	Ogre::Matrix4 mat(matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5], matrix[6], matrix[7], matrix[8],
        matrix[9], matrix[10], matrix[11], matrix[12], matrix[13], matrix[14], matrix[15]);

	// Needs to convert from d3d style matrix to OpenGL style matrix //
	Ogre::Matrix4 tmat = mat.transpose();
	Ogre::Vector3 vec = tmat.getTrans();

	Float3 position = vec;

	// Rotation //
	Float4 quat(tmat.extractQuaternion());

	// Apply to graphical object //
	Brush* tmp = static_cast<Brush*>(reinterpret_cast<BasePhysicsObject*>(NewtonBodyGetUserData(body)));
    
    // The object needs to be locked here //
    GUARD_LOCK_OTHER_OBJECT(tmp);
    
    if(tmp->ObjectsNode){
        
        tmp->ObjectsNode->setOrientation(quat);
        tmp->ObjectsNode->setPosition(position);
    }
    
	// Also update these so if only one is updated it doesn't force last value to rotation or location //
	tmp->Position = position;
	tmp->QuatRotation = quat;
    
	// Update potential children //
	tmp->_ParentableNotifyLocationDataUpdated();

    tmp->_MarkDataUpdated(guard);
}
// ------------------------------------ //
bool Leviathan::Entity::Brush::_LoadOwnDataFromPacket(sf::Packet &packet){

    // First get the base class data //
    BasePositionData pdata;

    if(!LoadPositionFromPacketToHolder(packet, pdata)){

        // It failed (the packet was invalid) //
        Logger::Get()->Error("Brush: packet has invalid format");
        return false;
    }

    float x, y, z;
    bool physics;
    float mass;
    string matname;
    int physid;

    // Get all the things at once and then check for invalid state //
    packet >> x >> y >> z;
    packet >> physics;
    
    if(physics){
        
        packet >> mass >> physid;
    }

    packet >> matname;

    // Don't apply data (Init) if the packet was invalid //
    if(!packet){

        Logger::Get()->Error("Brush: packet has invalid format");
        return false;
    }
        
    // We always create the physical object ourselves if wanted
    if(!Init(Float3(x, y, z), matname, false)){

        // This shouldn't happen //
        Logger::Get()->Error("Brush: failed to create from packet, Init failed");
        return false;
    }

    if(physics){

        AddPhysicalObject(mass);
        
        if(physid >= 0)
            SetPhysicalMaterialID(physid);
    }
    
    
    // Then set the position //
    ApplyPositionDataObject(pdata);

    // Apply hidden state //
    _OnHiddenStateUpdated();
    
    return true;
}

void Leviathan::Entity::Brush::_SaveOwnDataToPacket(sf::Packet &packet){

    GUARD_LOCK_THIS_OBJECT();

    // The hidden state needs to be the first thing
    packet << Hidden;
    
    // Before adding our data make base classes add stuff //
    AddPositionAndRotationToPacket(packet);

    // First add the size //
    packet << Sizes.X << Sizes.Y << Sizes.Z;

    // Then whether we have a physical object or not //
    packet << (GetPhysicsBody() ? true: false);

    // Add the mass if it is applicable //
    if(GetPhysicsBody()){

        packet << Mass << AppliedPhysicalMaterial;
    }

    // And finally our material //
    packet << Material;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Entity::Brush::SendCustomMessage(int entitycustommessagetype, void* dataptr){
	// First check if it is a request //
	if(entitycustommessagetype == ENTITYCUSTOMMESSAGETYPE_DATAREQUEST){
		// Check through components //
		ObjectDataRequest* tmprequest = reinterpret_cast<ObjectDataRequest*>(dataptr);

		BASEPOSITIONAL_CUSTOMMESSAGE_GET_CHECK;
		BASEPHYSICS_CUSTOMMESSAGE_GET_CHECK;
		BASEPARENTABLE_CUSTOMMESSAGE_GET_CHECK;

		return false;
	}

	// Check through components //
	BASEPOSITIONAL_CUSTOMMESSAGE_DATA_CHECK;
	BASEPHYSICS_CUSTOMMESSAGE_DATA_CHECK;
	BASEPARENTABLE_CUSTOMMESSAGE_DATA_CHECK;

	// This specific //

	return false;
}
// ------------------------------------ //
void Leviathan::Entity::Brush::_GetCurrentActualPosition(Float3 &pos){

    GetPos(pos);
}
        
void Leviathan::Entity::Brush::_GetCurrentActualRotation(Float4 &rot){

    GetOrientation(rot);
}
// ------------------------------------ //
void Leviathan::Entity::Brush::_SendCreatedConstraint(BaseConstraintable* other, Entity::BaseConstraint* ptr){
    _SendNewConstraint(static_cast<BaseConstraintable*>(this), other, ptr);
}
// ------------------------------------ //
BaseConstraintable* Leviathan::Entity::Brush::BasePhysicsGetConstraintable(){
    return static_cast<BaseConstraintable*>(this);
}
// ------------------------------------ //
DLLEXPORT shared_ptr<ObjectDeltaStateData> Leviathan::Entity::Brush::CaptureState(){
    
    return shared_ptr<ObjectDeltaStateData>(
        PositionablePhysicalDeltaState::CaptureState(*this).release());
}

DLLEXPORT void Leviathan::Entity::Brush::VerifyOldState(ObjectDeltaStateData* serversold, ObjectDeltaStateData* ourold,
    int tick)
{
    CheckOldPhysicalState(static_cast<PositionablePhysicalDeltaState*>(serversold),
        static_cast<PositionablePhysicalDeltaState*>(ourold), tick, this);
}

void Leviathan::Entity::Brush::OnBeforeResimulateStateChanged(){
    StartInterpolating(GetPos(), GetOrientation());
}

DLLEXPORT shared_ptr<ObjectDeltaStateData> Leviathan::Entity::Brush::CreateStateFromPacket(sf::Packet &packet) const{
    
    try{
        
        return make_shared<PositionablePhysicalDeltaState>(packet);
        
    } catch(ExceptionInvalidArgument &e){

        Logger::Get()->Warning("Brush: failed to CreateStateFromPacket, exception:");
        e.PrintToLog();
        return nullptr;
    }
    
}


