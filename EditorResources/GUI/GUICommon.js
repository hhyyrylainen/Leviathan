// Common stuff for GUI code
"use strict";

//! Returns true if Leviathan is available false if inside a desktop browser
function IsInEngine()
{
    return typeof Leviathan === 'object' && Leviathan !== null;
}

//! Shows an alert if isInEngine is false
function RequireEngine(msg)
{
    if(!IsInEngine()){

        alert("This method only works inside a Leviathan application, msg: " + msg);
    }
}
