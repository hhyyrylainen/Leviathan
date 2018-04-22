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
    native function LOnEvent(eventstypestr, callbackfunction);
    Leviathan.OnEvent = LOnEvent;
    
    //! Allows JavaScript to receive GenericEvents
    //! The callback will be called with (eventname, namedvars)
    native function LOnGeneric(genericname, callbackfunction);
    Leviathan.OnGeneric = LOnGeneric;

    //! Plays a 2D sound effect. Requires VIEW_SECURITYLEVEL_NORMAL
    native function Play2DSoundEffect(filename);
    Leviathan.Play2DSoundEffect = Play2DSoundEffect;

    //! Plays a 2D sound (either of the boolean parameters must be
    //! true. use Play2DSoundEffect instead if both are false)
    //! oncreated a callback to be called when the object is created. Passes the object
    //! as the first parameter to the callback
    native function Play2DSound(filename, looping, startpaused, oncreated);
    Leviathan.Play2DSound = Play2DSound;
    
}());
