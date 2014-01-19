// default function implementations for GuiCollection //
// define string ROCKETOBJECTID("something") before including this file //

// set's the visibility of the object //
void HandleSet(GuiCollection@ Instance, bool hidden){
    // fetch pointers
    GuiLoadedSheet@ tmpsheet = Instance.GetOwningSheet();
    RocketElement@ maincontainer = tmpsheet.GetElementByID(ROCKETOBJECTID);
    
    if(hidden){
        maincontainer.SetProperty("visibility", "hidden");
        maincontainer.SetProperty("display", "none");
        Print("Collection "+Instance.GetName()+" set hidden");
    } else {
        maincontainer.SetProperty("visibility", "visible");
        maincontainer.SetProperty("display", "block");
        Print("Collection "+Instance.GetName()+" set visible");
    }
}

[@Listener="OnHide"]
int OnHide(GuiCollection@ Instance, Event@ event){
    // we need to hide our elements //
    HandleSet(Instance, true);
    return 1;
}

[@Listener="OnShow"]
int OnShow(GuiCollection@ Instance, Event@ event){
    // we need to show our elements //
    HandleSet(Instance, false);
    return 1;
}