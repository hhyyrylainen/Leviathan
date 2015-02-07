#pragma once
#ifndef LEVIATHAN_MAIN
#define LEVIATHAN_MAIN
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
// ---- includes ---- //
#include "Application/Application.h"

// Some core classes that most likely won't conflict are brought to the global namespace here with 'using' statements //

using Leviathan::Logger;
using Leviathan::FileSystem;
using Leviathan::Engine;
using Leviathan::Gui::GuiManager;
using Leviathan::GameWorld;
using Leviathan::AppDef;
using Leviathan::MasterServerInformation;
using Leviathan::Convert;
using Leviathan::EventHandler;
using Leviathan::Event;
using Leviathan::GenericEvent;
using Leviathan::Float1;
using Leviathan::Float2;
using Leviathan::Float3;
using Leviathan::Float4;
using Leviathan::Int1;
using Leviathan::Int2;
using Leviathan::Int3;
using Leviathan::LeviathanApplication;
using Leviathan::StringIterator;
using Leviathan::NetworkRequest;
using Leviathan::NetworkResponse;
using Leviathan::ConnectionInfo;
using Leviathan::Window;
using Leviathan::VariableBlock;
using Leviathan::TimingMonitor;
using Leviathan::ScopeTimer;
using Leviathan::GameModule;
using Leviathan::ObjectLock;
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
using Leviathan::NetworkedInput;
using Leviathan::ThreadSafe;
using Leviathan::ApplyForceInfo;


using Leviathan::ThreadingManager;
using Leviathan::QueuedTask;
using Leviathan::DelayedTask;
using Leviathan::ConditionalTask;
using Leviathan::ConditionalDelayedTask;
using Leviathan::RepeatingDelayedTask;
using Leviathan::RepeatCountedDelayedTask;


using Leviathan::BaseObject;
using Leviathan::GameWorld;
using Leviathan::ObjectPtr;

using Leviathan::ExceptionInvalidArgument;


#endif
