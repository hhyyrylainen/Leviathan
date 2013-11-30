


[@Listener="Init"]
void OnLoad(GameModule@ module, Event@ event){
    // We pretty much have nothing to load here... //
    Print("AI loaded and working");
}

[@Listener="Release"]
void OnRelease(GameModule@ module, Event@ event){
    // Nothing to release
    Print("Unloading AI");
}

