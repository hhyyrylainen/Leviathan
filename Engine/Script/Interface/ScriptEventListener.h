#pragma once
#include "Common/ReferenceCounted.h"
#include "Events/CallableObject.h"
#include "Exceptions/ExceptionInvalidArgument.h"
#include "Common/ThreadSafe.h"

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
                    throw ExceptionInvalidArgument(L"At least on event or on generic listeners need to be passed", 0,
                        __WFUNCTION__, L"onevent ongeneric", L"both are NULL");
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

                    // Execute the script function //
                    DEBUG_BREAK;
                }

                return -1;
            }

            int OnGenericEvent(GenericEvent** event) override{

                GUARD_LOCK_THIS_OBJECT();
                
                if(OnGenericScript){

                    // Execute the script function //
                    DEBUG_BREAK;
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
                
            } catch(const ExceptionInvalidArgument &e){

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

