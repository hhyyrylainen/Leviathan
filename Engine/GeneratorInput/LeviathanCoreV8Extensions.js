// The global Leviathan object
var Leviathan = {};


// All library functions defined in one function block //
(function(){
    "use strict";

    //! This is needed to properly allow focusing input elements while playing
    Leviathan.SetupInputDetection = function () {
        native function NotifyViewInputStatus();
        let detectChange = function() {
            if (document.activeElement instanceof HTMLInputElement) {
                NotifyViewInputStatus(true);
            } else {
                NotifyViewInputStatus(false);
            }
        }

        window.addEventListener('focus', detectChange, true);
        window.addEventListener('blur', detectChange, true);

        // Detect initial focus
        detectChange();
    }
    
    //! Closes the game, requires VIEW_SECURITYLEVEL_ACCESS_ALL
    Leviathan.Quit = function(){
        window.cefQuery({request: 'Quit', persistent: false,
            onSuccess: function(){}, onFailure: function(){}});
    }
    
    //! Gets the current Leviathan version, requires VIEW_SECURITYLEVEL_MINIMAL
    //!
    //! The result is returned to the successcallback in string form if successful
    Leviathan.GetVersion = function(successcallback, failurecallback){
        window.cefQuery({request: 'LeviathanVersion', persistent: false,
            onSuccess: successcallback,
            onFailure: failurecallback});
    }
    
    // Register the native functions //
    //! Allows JavaScript to receive Engine events
    Leviathan.OnEvent = function(eventstypestr, callbackfunction){
        native function LOnEvent();
        return LOnEvent(eventstypestr, callbackfunction);
    }
    
    //! Allows JavaScript to receive GenericEvents
    Leviathan.OnGeneric = function(genericname, callbackfunction){
        native function LOnGeneric();
        return LOnGeneric(genericname, callbackfunction);
    }

    
}());
