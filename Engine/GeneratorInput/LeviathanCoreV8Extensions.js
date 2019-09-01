// The global Leviathan object
var Leviathan = {};


// All library functions defined in one function block //
(function(){
    "use strict";

    Leviathan._ = {};

    //! This is needed to properly allow focusing input elements while playing
    //! And scrollable things
    Leviathan.SetupInputDetection = function () {
        native function NotifyViewInputStatus(focused);
        native function NotifyViewScrollableStatus(scrollable);

        Leviathan._.LastInputStatus = false;
        Leviathan._.LastScrollStatus = false;
        
        let detectChange = function() {
            let status;
            if (document.activeElement instanceof HTMLInputElement) {
                status = true;
            } else {
                status = false;
            }
            if(status === Leviathan._.LastInputStatus)
                return;
            
            Leviathan._.LastInputStatus = status;
            NotifyViewInputStatus(Leviathan._.LastInputStatus);
        };


        let detectScroll = function(event) {

            let scrollable = false;
            let element = document.elementFromPoint(event.clientX, event.clientY);

            // Grab this target (as the mouse can be over a scroll bar where there's
            // nothing under it
            if(element == null)
                element = event.target;

            while(element != null){
                
                // Check is it scrollable
                let overflowY = window.getComputedStyle(element)["overflow-y"];
                // let overflowY = element.style.overflowY;
                // TODO: x-way scrolling
                // let overflowX = window.getComputedStyle(element)["overflow-x"];

                // TODO: should this be used instead?
                // element.offsetHeight
                
                if((overflowY === "scroll" || overflowY === "auto" ||
                    // This detects if the whole page is scrollable
                    (element instanceof HTMLHtmlElement && overflowY === "visible"))
                    && element.scrollHeight > element.clientHeight
                   // || (overflowX === 'scroll' || overflowX === 'auto') &&
                   // element.scrollWidth > element.clientWidth
                  ) {
                    // console.log("this is scrollable: " + overflowY + " scroll: " +
                    //             element.scrollHeight + " clientHeight: " +
                    //             element.clientHeight);
                    scrollable = true;
                    break;
                }

                // Check parent
                element = element.parentElement;
            }

            if(scrollable === Leviathan._.LastScrollStatus)
                return;
            
            Leviathan._.LastScrollStatus = scrollable;
            NotifyViewScrollableStatus(Leviathan._.LastScrollStatus);
        };

        window.addEventListener('focus', detectChange, true);
        window.addEventListener('blur', detectChange, true);
        window.addEventListener('mouseover', detectScroll, false);

        // Detect initial focus
        detectChange();
    };
    
    //! Closes the game, requires VIEW_SECURITYLEVEL_ACCESS_ALL
    Leviathan.Quit = function(){
        window.cefQuery({request: 'Quit', persistent: false,
            onSuccess: function(){}, onFailure: function(){}});
    };
    
    //! Gets the current Leviathan version, requires VIEW_SECURITYLEVEL_MINIMAL
    //!
    //! The result is returned to the successcallback in string form if successful
    Leviathan.GetVersion = function(successcallback, failurecallback){
        window.cefQuery({request: 'LeviathanVersion', persistent: false,
            onSuccess: successcallback,
            onFailure: failurecallback});
    };
    
    // Register the native functions //
    //! Allows JavaScript to receive Engine events
    native function LOnEvent(eventstypestr, callbackfunction);
    Leviathan.OnEvent = LOnEvent;
    
    //! Allows JavaScript to receive GenericEvents
    //! The callback will be called with (eventname, namedvars)
    native function LOnGeneric(genericname, callbackfunction);
    Leviathan.OnGeneric = LOnGeneric;

    //! Fires an event from js
    native function CallGenericEvent(genericname, values);
    Leviathan.CallGenericEvent = CallGenericEvent;

    //! Plays a 2D sound effect. Requires VIEW_SECURITYLEVEL_NORMAL
    native function Play2DSoundEffect(filename);
    Leviathan.Play2DSoundEffect = Play2DSoundEffect;

    //! Plays a 2D sound (either of the boolean parameters must be
    //! true. use Play2DSoundEffect instead if both are false)
    //! oncreated a callback to be called when the object is created. Passes the object
    //! as the first parameter to the callback
    native function Play2DSound(filename, looping, oncreated);
    Leviathan.Play2DSound = Play2DSound;

    //! Creates a new GUI layer and plays a video in it. Calls the callback when done.
    //! On error onerror callback is called. Only one of the callbacks is called
    native function PlayCutscene(filename, onended, onerror);
    Leviathan.PlayCutscene = PlayCutscene;

    //! Destroys a playing cutscene player if there is one active
    native function CancelCutscene();
    Leviathan.CancelCutscene = CancelCutscene;
    
}());
