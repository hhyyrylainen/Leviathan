// ------------------ This file contains forward declarations for all possible things
// that need to be included (or forward declared)
#pragma once

#include <cstdint>

// Newton things
class NewtonJoint;

namespace Ogre{

    class SceneManager;
    class SceneNode;
}

namespace Leviathan{

    // Core systems //
	class Logger;
	class FileSystem;
	class Engine;
	class Graphics;
    class ScriptExecutor;
    class NetworkHandler;
	class ScriptExecutor;
	class GameWorld;
	class AppDef;
	class Convert;
	class EventHandler;
    class DataStore;
    class OutOfMemoryHandler;
    class RenderingStatistics;
    class IDFactory;
    class Time;

    class Locker;

    class Visitor;
    class Visitable;


    class InputReceiver;
    class InputController;

	namespace Gui{
		
		class GuiManager;
	}

    // Entities //
    class Sendable;
    class Position;
    class ObjectDeltaStateData;
    class Constraintable;
    class Parentable;
    class Parent;

    class SendableEntitySerializer;

    class Physics;


    // Constraints //
    class BaseConstraint;
    enum ENTITY_CONSTRAINT_TYPE : int32_t;
    
    

    // DeltaStates //
    class PositionDeltaState;
    class PhysicalDeltaState;
    class ObjectDeltaStateData;
    class TrackControllerState;
    

    // Exceptions //
    class Exception;
    class InvalidAccess;
    class InvalidArgument;
    class InvalidState;
    class InvalidType;
    class NotFound;
    class NULLPtr;


	class ObjectFileTemplateInstance;
	class ObjectFileTemplateDefinition;

    class ObjectFileList;

    class BaseEntitySerializer;
    class EntitySerializerManager;

    // Network Response //
    class NetworkResponseDataForInitialEntity;
    class NetworkResponseDataForEntityConstraint;
    class NetworkResponseDataForWorldFrozen;
    class NetworkResponseDataForEntityUpdate;
    class NetworkResponseDataForAICacheUpdated;

    // Network Request //
    class RequestWorldClockSyncData;

    // Threading //
	class ThreadingManager;
	class QueuedTask;
	class DelayedTask;
	class ConditionalTask;
	class ConditionalDelayedTask;
	class RepeatingDelayedTask;
	class RepeatCountedDelayedTask;

    class ConstraintSerializerManager;
    
    class AINetworkCache;

	class NetworkedInput;
	class NetworkedInputHandler;
	class ConnectedPlayer;
	class CommandSender;
	class CommandHandler;
    
	class ScriptArgumentsProviderBridge;
    struct FunctionParameterInfo;
    
	class ResourceFolderListener;
	class ResourceRefreshHandler;
    
	class UTF8DataIterator;
	template<class DTypeName> class SyncedPrimitive;
	class SyncedResource;
    
	class ViewerCameraPos;
	class NewtonManager;
	class SoundDevice;
	class PhysicsMaterialManager;
	class NewtonManager;
	class ScriptConsole;
    
	class LeapManager;
    class LeapListener;

	class ObjectLoader;
	class GameSpecificPacketHandler;
	class BaseGameSpecificRequestPacket;
	class BaseGameSpecificResponsePacket;
	class GameSpecificPacketData;
	class NamedVariableBlock;
	class SyncedVariables;
	class SyncedValue;
	class NetworkInterface;
	class NamedVars;
	class NamedVariableList;
	class NetworkServerInterface;
	class NetworkClientInterface;
	class SentNetworkThing;
	class BaseEntityController;
	class BaseNotifiableAll;
	class BaseNotifierAll;
	class BaseNotifierEntity;
	class BaseNotifiableEntity;
	template<class ParentType, class ChildType> class BaseNotifier;
	template<class ParentType, class ChildType> class BaseNotifiable;
	class RemoteConsole;
	class NetworkRequest;
	class NetworkResponse;
	class ConnectionInfo;
	class GameConfiguration;
	class KeyConfiguration;
	class ThreadingManager;
	class TaskThread;
	class Engine;
	class LeviathanApplication;
	struct RayCastData;
	class ScriptModule;
	class ScriptScript;
	class Event;
	class GenericEvent;
	struct Int1;
	struct Int2;
	struct Int3;
	class LeviathanApplication;
	class GraphicalInputEntity;
	class StringIterator;
	class Window;
    class Random;
	class VariableBlock;
	class TimingMonitor;
	class ScopeTimer;
	class GameModule;
	struct MasterServerInformation;
}


