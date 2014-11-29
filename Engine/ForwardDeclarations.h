// ------------------ This file contains forward declarations for all possible things that need to be included
// (or forward declared) ------------------
#pragma once

// Newton things
struct NewtonJoint;

namespace Leviathan{
	class Logger;
	class FileSystem;
	class Engine;
	class Graphics;

	namespace Gui{
		
		class GuiManager;
	}

	namespace Entity{

		class Prop;
		class Brush;
		struct TrackControllerPosition;
		class TrackEntityController;
		struct TrailProperties;
		class TrailEmitter;

        class BaseConstraint;
        class BaseConstraintable;

        enum ENTITY_CONSTRAINT_TYPE : int32_t;
	}

	// Entity bases //
	class BasePositionable;
    class BaseObject;

    // Exceptions //
	class ExceptionInvalidArgument;


	class ObjectFileTemplateInstance;
	class ObjectFileTemplateDefinition;

    class BaseEntitySerializer;
    class EntitySerializerManager;

    // Network Response //
    class NetworkResponseDataForInitialEntity;
    class NetworkResponseDataForEntityConstraint;
    class NetworkResponseDataForWorldFrozen;

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
    
	class ApplyForceInfo;

	class NetworkedInput;
	class NetworkedInputHandler;
	class ConnectedPlayer;
	class CommandSender;
	class CommandHandler;
	class ScriptArgumentsProviderBridge;
	class ResourceFolderListener;
	class ResourceRefreshHandler;
	class UTF8DataIterator;
	template<class DTypeName> class SyncedPrimitive;
	class SyncedResource;
	class ScriptInterface;
	class ViewerCameraPos;
	class NewtonManager;
	class SoundDevice;
	class PhysicsMaterialManager;
	class NewtonManager;
	class ScriptConsole;
	class LeapManager;
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
	struct SentNetworkThing;
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
	class NetworkHandler;
	class ScriptExecutor;
	class GameWorld;
	class AppDef;
	class Convert;
	class EventHandler;
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
	class ThreadSafe;
    class DataStore;
    class OutOfMemoryHandler;
    class RenderingStatistics;
	struct MasterServerInformation;
}


