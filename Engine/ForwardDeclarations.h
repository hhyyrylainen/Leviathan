// ------------------ This file contains forward declarations for all possible things
// that need to be included (or forward declared)
#pragma once

#include <cstdint>


namespace Leviathan {

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

namespace GUI {

class GuiManager;
class View;
class LeviathanJavaScriptAsync;
class JSAsyncCustom;
class CefApplication;
} // namespace GUI

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

class ResourceFolderListener;
class ResourceRefreshHandler;

class UTF8DataIterator;
template<class DTypeName>
class SyncedPrimitive;
class SyncedResource;

class ViewerCameraPos;
class SoundDevice;
class PhysicsMaterialManager;
class ScriptConsole;

class LeapManager;
class LeapListener;

class GameModule;

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
template<class ParentType, class ChildType>
class BaseNotifier;
template<class ParentType, class ChildType>
class BaseNotifiable;
class GameConfiguration;
class KeyConfiguration;
class Engine;
class LeviathanApplication;
struct RayCastData;
class ScriptModule;
class ScriptScript;
class Event;
class GenericEvent;
struct Int2;
struct Int3;
class LeviathanApplication;
class StringIterator;
class Window;
class Random;
class VariableBlock;
class TimingMonitor;
class ScopeTimer;
struct MasterServerInformation;
class GlobalCEFHandler;
} // namespace Leviathan

#ifdef LEAK_INTO_GLOBAL
using Leviathan::Locker;

using Leviathan::AppDef;
using Leviathan::BaseNotifiableAll;
using Leviathan::BaseNotifierAll;
using Leviathan::CommandHandler;
using Leviathan::CommandSender;
using Leviathan::Connection;
using Leviathan::Convert;
using Leviathan::Engine;
using Leviathan::Event;
using Leviathan::EventHandler;
using Leviathan::FileSystem;
using Leviathan::GameConfiguration;
using Leviathan::GameModule;
using Leviathan::GameSpecificPacketData;
using Leviathan::GameWorld;
using Leviathan::GenericEvent;
using Leviathan::KeyConfiguration;
using Leviathan::LeviathanApplication;
using Leviathan::Logger;
using Leviathan::MasterServerInformation;
using Leviathan::NamedVariableList;
using Leviathan::NamedVars;
using Leviathan::NetworkClientInterface;
using Leviathan::NetworkRequest;
using Leviathan::NetworkResponse;
using Leviathan::QueuedTask;
using Leviathan::ScopeTimer;
using Leviathan::SentNetworkThing;
using Leviathan::StringIterator;
using Leviathan::SyncedPrimitive;
using Leviathan::SyncedResource;
using Leviathan::SyncedValue;
using Leviathan::TimingMonitor;
using Leviathan::UTF8DataIterator;
using Leviathan::VariableBlock;
using Leviathan::GUI::GuiManager;


using Leviathan::ConditionalDelayedTask;
using Leviathan::ConditionalTask;
using Leviathan::DelayedTask;
using Leviathan::QueuedTask;
using Leviathan::RepeatCountedDelayedTask;
using Leviathan::RepeatingDelayedTask;
using Leviathan::ThreadingManager;


using Leviathan::GameWorld;

using Leviathan::Exception;
using Leviathan::InvalidAccess;
using Leviathan::InvalidArgument;
using Leviathan::InvalidState;
using Leviathan::InvalidType;
#endif
