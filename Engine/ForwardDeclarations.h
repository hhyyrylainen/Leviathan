// ------------------ This file contains forward declarations for all possible things that need to be included (or forward declared) ------------------ //
#pragma once



namespace Leviathan{
	class Logger;
	class FileSystem;
	class Engine;

	namespace Gui{


		class GuiManager;
	}

	namespace Entity{

		class BaseNotifierEntity;
		class BaseNotifiableEntity;
		class Prop;
		class Brush;
		struct TrackControllerPosition;
		class TrackEntityController;
		struct TrailProperties;
		class TrailEmitter;
	}


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
	class WstringIterator;
	class Window;
	class VariableBlock;
	class TimingMonitor;
	class ScopeTimer;
	class GameModule;
	class ThreadSafe;
	struct MasterServerInformation;

}


