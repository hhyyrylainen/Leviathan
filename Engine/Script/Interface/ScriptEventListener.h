#pragma once
#include "Common/ReferenceCounted.h"
#include "Events/CallableObject.h"
#include "Exceptions.h"
#include "Common/ThreadSafe.h"
#include "Script/ScriptInterface.h"

namespace Leviathan{ namespace Script{

        //! \brief Script class 
        class EventListener : public ReferenceCounted, public CallableObject, public ThreadSafe{
        public:
            EventListener(asIScriptFunction* onevent, asIScriptFunction* ongeneric){

                if(onevent){
                    
                    OnEventScript = onevent;
                    
                } else {
                    
                    OnEventScript = NULL;
                }

                if(ongeneric){
                    
                    OnGenericScript = ongeneric;
                    
                } else {
                    
                    OnGenericScript = NULL;
                }

                // Fail if neither is set //
                if(!OnGenericScript && !OnEventScript)
                    throw InvalidArgument("At least on event or on generic listeners need to be "
                        "provided, both are NULL");
            }
            
            ~EventListener(){

                UnRegisterAllEvents();

                GUARD_LOCK_THIS_OBJECT();

                SAFE_RELEASE(OnEventScript);
                SAFE_RELEASE(OnGenericScript);
            }

            int OnEvent(Event** event) override{

                GUARD_LOCK_THIS_OBJECT();
                
                if(OnEventScript){

                    // Setup the parameters //
                    vector<shared_ptr<NamedVariableBlock>> Args = boost::assign::list_of(new NamedVariableBlock(
                            new VoidPtrBlock(*event), L"Event"));

                    (*event)->AddRef();

                    ScriptRunningSetup sargs;
                    sargs.SetArguments(Args);


                    // Run the script //
                    shared_ptr<VariableBlock> result = ScriptInterface::Get()->ExecuteScript(OnGenericScript, &sargs);

                    if(!result || !result->IsConversionAllowedNonPtr<int>()){

                        return 0;
                    }

                    // Return the value returned by the script //
                    return result->ConvertAndReturnVariable<int>();
                }

                return -1;
            }

            int OnGenericEvent(GenericEvent** event) override{

                GUARD_LOCK_THIS_OBJECT();
                
                if(OnGenericScript){

                    // Setup the parameters //
                    vector<shared_ptr<NamedVariableBlock>> Args = boost::assign::list_of(new NamedVariableBlock(
                            new VoidPtrBlock(*event), L"GenericEvent"));

                    (*event)->AddRef();

                    ScriptRunningSetup sargs;
                    sargs.SetArguments(Args);


                    // Run the script //
                    shared_ptr<VariableBlock> result = ScriptInterface::Get()->ExecuteScript(OnGenericScript, &sargs);

                    if(!result || !result->IsConversionAllowedNonPtr<int>()){

                        return 0;
                    }

                    // Return the value returned by the script //
                    return result->ConvertAndReturnVariable<int>();
                }

                return -1;
            }

            //! \brief Registers for a predefined event type if OnEvent is not NULL
            bool RegisterForEventType(EVENT_TYPE type){
                
                {
                    GUARD_LOCK_THIS_OBJECT();
                
                    if(!OnEventScript)
                        return false;
                }

                EventHandler::Get()->RegisterForEvent(this, type);
                return true;
            }

            //! \brief Registers for a generic event if OnGeneric is not NULL
            bool RegisterForEventGeneric(const string &name){
                
                {
                    GUARD_LOCK_THIS_OBJECT();
                
                    if(!OnGenericScript)
                        return false;
                }
                
                EventHandler::Get()->RegisterForEvent(this, Convert::StringToWstring(name));
                return true;
            }

            REFERENCECOUNTED_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(EventListener);

        protected:

            // The AngelScript functions to be called //
            asIScriptFunction* OnEventScript;
            asIScriptFunction* OnGenericScript;
        };
        
        EventListener* EventListenerFactory(asIScriptFunction* onevent, asIScriptFunction* ongeneric){

            EventListener* listener = NULL;
            
            try{
                
                listener = new EventListener(onevent, ongeneric);
                
            } catch(const InvalidArgument &e){

                Logger::Get()->Error("Failed to construct Script::EventListener, exception: ");
                e.PrintToLog();
            }

            if(!listener){
                
                // The object won't release the parameter references //
                if(onevent)
                    onevent->Release();
                
                if(ongeneric)
                    ongeneric->Release();
            }
            
            return listener;
        }


    }
}

