// default function implementations for GuiCollection //
// define string GUIOBJECTID("something") before including this file //

// set's the visibility of the object //
void ToggleElement(GuiCollection@ Instance){
    // fetch pointers //
    GuiView@ tmpsheet = Instance.GetContainingView();
    
    tmpsheet.ToggleElement(GUIOBJECTID);
}

[@Listener="OnHide"]
int OnHide(GuiCollection@ Instance, Event@ event){
    // we need to hide our elements //
    HandleSet(Instance);
    return 1;
}

[@Listener="OnShow"]
int OnShow(GuiCollection@ Instance, Event@ event){
    // we need to show our elements //
    HandleSet(Instance);
    return 1;
}