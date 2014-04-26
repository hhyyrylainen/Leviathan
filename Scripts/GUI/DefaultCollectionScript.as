// default function implementations for GuiCollection //

// Set's the visibility of the Gui Window //
void HandleSet(GuiCollection@ Instance, bool isvisible){
    // fetch pointers //
    GuiManager@ tmpmanager = Instance.GetOwningGuiManager();
    
    // Get the actual target object //
    GuiWindow@ ourwind = tmpmanager.GetGuiWindowByName(Instance.GetWindowName());
    
    // Set it's visibility state //
    ourwind.SetVisibility(isvisible);
}

[@Listener="OnHide"]
int OnHide(GuiCollection@ Instance, Event@ event){
    // We need to hide our elements //
    HandleSet(Instance, false);
    return 1;
}

[@Listener="OnShow"]
int OnShow(GuiCollection@ Instance, Event@ event){
    // We need to show our elements //
    HandleSet(Instance, true);
    return 1;
}