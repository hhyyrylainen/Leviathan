// ------------------ This file contains forward declarations for all possible things
// that need to be included (or forward declared)
#pragma once

#include "Include.h"

#include <cstdint>

// Newton things
class NewtonJoint;

namespace Ogre{

class SceneManager;
class SceneNode;
}

namespace CEGUI{

class InputAggregator;
class Window;
class Animation;
class GUIContext;
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
class ConsoleInput;

class Locker;

class Visitor;
class Visitable;


class InputReceiver;
class InputController;

namespace GUI{
        
class GuiManager;
}

// Entities //

// Component types //
class Sendable;
class Position;
class Constraintable;
class Parentable;
class Parent;
class Physics;
class RenderNode;
class Received;
class BoxGeometry;
class Model;
class ManualObject;



class EntitySerializer;


// Networking //
class RemoteConsole;
class NetworkRequest;
class NetworkResponse;
class Connection;
class NetworkCache;

// Constraints //
class BaseConstraint;
    

// DeltaStates //

    

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


// Network Response //
class ResponseCacheUpdated;
class ResponseEntityCreation;
class ResponseWorldFrozen;
class ResponseCacheRemoved;
class ResponseIdentification;

// Network Request //
class RequestWorldClockSync;

// Threading //
class ThreadingManager;
class QueuedTask;
class DelayedTask;
class ConditionalTask;
class ConditionalDelayedTask;
class RepeatingDelayedTask;
class RepeatCountedDelayedTask;
class TaskThread;

class NetworkCache;

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
class GameConfiguration;
class KeyConfiguration;
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

#ifdef LEAK_INTO_GLOBAL
using Leviathan::Locker;

using Leviathan::Logger;
using Leviathan::FileSystem;
using Leviathan::Engine;
using Leviathan::GUI::GuiManager;
using Leviathan::GameWorld;
using Leviathan::AppDef;
using Leviathan::MasterServerInformation;
using Leviathan::Convert;
using Leviathan::EventHandler;
using Leviathan::Event;
using Leviathan::GenericEvent;
using Leviathan::LeviathanApplication;
using Leviathan::StringIterator;
using Leviathan::NetworkRequest;
using Leviathan::NetworkResponse;
using Leviathan::Connection;
using Leviathan::VariableBlock;
using Leviathan::TimingMonitor;
using Leviathan::ScopeTimer;
using Leviathan::GameModule;
using Leviathan::GameConfiguration;
using Leviathan::KeyConfiguration;
using Leviathan::NamedVars;
using Leviathan::NamedVariableList;
using Leviathan::NetworkClientInterface;
using Leviathan::SyncedValue;
using Leviathan::GameSpecificPacketData;
using Leviathan::SentNetworkThing;
using Leviathan::QueuedTask;
using Leviathan::SyncedResource;
using Leviathan::SyncedPrimitive;
using Leviathan::UTF8DataIterator;
using Leviathan::BaseNotifiableAll;
using Leviathan::BaseNotifierAll;
using Leviathan::CommandSender;
using Leviathan::CommandHandler;


using Leviathan::ThreadingManager;
using Leviathan::QueuedTask;
using Leviathan::DelayedTask;
using Leviathan::ConditionalTask;
using Leviathan::ConditionalDelayedTask;
using Leviathan::RepeatingDelayedTask;
using Leviathan::RepeatCountedDelayedTask;


using Leviathan::GameWorld;

using Leviathan::Exception;
using Leviathan::InvalidArgument;
using Leviathan::InvalidType;
using Leviathan::InvalidState;
using Leviathan::InvalidAccess;
#endif

