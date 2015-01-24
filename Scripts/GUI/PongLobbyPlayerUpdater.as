// The object that gets notified when the values change //
ScriptNotifier@ UpdateListener = null;

// Determines if any data is stored, on first update will default to false
bool OldDataCaptured = false;

// Contains the stored data which is used to determine what has changed //
PlayerDataHolder@ StoredData = @PlayerDataHolder();

// This is for accessing the GUI windows for changing their data //
GuiObject@ Us;

array<ControlScheme@> ControlList;

class ControlScheme{

    // controlnumber is used for controllers
    ControlScheme(const string &in showtext, PLAYERCONTROLS controltype, int controlnumber = 0){

        Text = showtext;
        Controls = controltype;
        ControlNumber = controlnumber;
    }
    
    string Text;
    PLAYERCONTROLS Controls;
    int ControlNumber;
}

    
[@Listener="OnInit"]
int SetupData(GuiObject@ instance, Event@ event){
    // Create a listener for getting notified when the player list changes //
    @UpdateListener = @ScriptNotifier(CheckIsSomethingActuallyUpdated);
    
    // Connect it //
    GetPongBase().GetPlayerChanges().ConnectToNotifier(UpdateListener);
    
    // We can store reference to us for the rest of eternity //
    @Us = @instance;

    // Build controls list //
    ControlList.insertLast(ControlScheme("Arrow keys", PLAYERCONTROLS_ARROWS));


    // Add them to all of the controls //
    for(int i = 0; i < 4; i++){

        const string target1 = "LobbyScreen/LobbyTabs/__auto_TabPane__/Team"+i+"/Player"+i+"0TypeSelect";

        const string target2 = "LobbyScreen/LobbyTabs/__auto_TabPane__/Team"+i+"/Player"+i+"1TypeSelect";
        
        AddControlsToBox(target1, false);
        AddControlsToBox(target2, false);
    }
    
    return 1;
}

[@Listener="OnRelease"]
int ReleaseData(GuiObject@ instance, Event@ event){
    
    // Let the listener be released //
    @UpdateListener = null;
    
    return 1;
}

void AddControlsToBox(const string &in element, bool clearfirst = true){

    // TODO: find a more efficient way to do this
    
    if(clearfirst){

        Us.GetOwningManager().GetWindowByName(element).ClearItems();
    }

    for(uint i = 0; i < ControlList.length(); i++){
        
        Us.GetOwningManager().GetWindowByName(element).AddItem(ControlList[i].Text);
    }
}

void CheckIsSomethingActuallyUpdated(){
    // Go through all the current data and check if any of it has changed //
    if(!OldDataCaptured){
    
        OldDataCaptured = true;
        
        // Update everything //
        StoredData.UpdateAllData();
        return;
    }
    
    // Check individual things whether they have been updated //
    StoredData.UpdateDataIfChanged();
}

// Converts a player type to a string //
string PlayerTypeToString(PLAYERTYPE type, int humanid){
    if(type == PLAYERTYPE_EMPTY)
        return "Empty";
    if(type == PLAYERTYPE_HUMAN)
        return "Human, PlayerID: "+humanid;
    if(type == PLAYERTYPE_COMPUTER)
        return "Computer";
    if(type == PLAYERTYPE_CLOSED)
        return "Closed";
    return "Unknown";
}


// This keeps a single player's data safe //
class PlayerData{
    
    PlayerData(int number, bool splittedthing = false){
    
        SlotNumber = number;
        IsSplitSlot = splittedthing;
    }
    
    ~PlayerData(){
        // If this is a split, reset the buttons etc. to disabled //
        
    }
    
    void CheckUpdates(PlayerSlot@ slot, bool forceupdate = false){
        
        PLAYERTYPE newtype = slot.GetPlayerType();
        int newid = slot.GetPlayerID();
        
        // Check has it updated //
        if(forceupdate || PlayerType != newtype || HumanPlayerID != newid){
            
            // It has! store the new value and notify about this //
            PlayerType = newtype;
            HumanPlayerID = newid;
            PlayerTypeUpdated();
        }
        
        int newscore = slot.GetScore();
        if(forceupdate || Score != newscore){
            
            Score = newscore;
            ScoreUpdated();
        }
        
        PLAYERCONTROLS newcontrols = slot.GetControlType();
        if(forceupdate || newcontrols != ControlType){
            
            ControlType = newcontrols;
            ControlsUpdated();
        }

        string othertarget = "LobbyScreen/LobbyTabs/__auto_TabPane__/Team"+SlotNumber+"/Player"+
            SlotNumber+(IsSplitSlot ? "1": "0")+"TypeSelect";
        
        // Enable controls button if possible //
        if(HumanPlayerID == GetPongGame().GetOurPlayerID()){

            // Enable control selection //
            Us.GetOwningManager().GetWindowByName(othertarget).SetDisabledState(false);
                
        } else {

            // Disable control selection //
            Us.GetOwningManager().GetWindowByName(othertarget).SetDisabledState(true);
        }
        
        
        // Handle split //
        PlayerSlot@ newsplitdata = slot.GetSplit();
        
        if(newsplitdata !is null && SplitData is null){
            
            // New split slot //
            @SplitData = @PlayerData(SlotNumber, true);
            SplitData.CheckUpdates(newsplitdata, true);
        } else if(newsplitdata !is null && SplitData !is null){
            
            SplitData.CheckUpdates(newsplitdata, forceupdate);
        }
    }
    
    // Controls split //
    bool IsSplitSlot;
    
    
    // The split handle //
    PlayerData@ SplitData;
    
    int HumanPlayerID = -1;
    
    // The number of the slot 0-3
    int SlotNumber;
    PLAYERTYPE PlayerType;
    PLAYERCONTROLS ControlType;
    
    // Might as well handle the score here //
    int Score = 0;
    
    
    // GUI Updating functions //
    void PlayerTypeUpdated(){
        
        // Set the proper texts //
        Print("Updating player type, for: " +SlotNumber+", split: "+IsSplitSlot);
        string targetthing = "LobbyScreen/LobbyTabs/__auto_TabPane__/Team"+SlotNumber+"/Player"+
            SlotNumber+(IsSplitSlot ? "1": "0");
        
        Us.GetOwningManager().GetWindowByName(targetthing).SetText(PlayerTypeToString(PlayerType, HumanPlayerID));
        
        string othertarget = "LobbyScreen/LobbyTabs/__auto_TabPane__/Team"+SlotNumber+"/Player"+
            SlotNumber+(IsSplitSlot ? "1": "0")+"JoinOpen";
            
        if(PlayerType == PLAYERTYPE_HUMAN || PlayerType == PLAYERTYPE_COMPUTER){
        
            Us.GetOwningManager().GetWindowByName(othertarget).SetText("Kick");
            
        } else if(PlayerType == PLAYERTYPE_EMPTY){
            
            Us.GetOwningManager().GetWindowByName(othertarget).SetText("Join");
            
        } else {
            
            Us.GetOwningManager().GetWindowByName(othertarget).SetText("Open");
        }
    }
    
    void ScoreUpdated(){
        
        Print("Updating player score, for: " +SlotNumber+", split: "+IsSplitSlot);
    }
    
    void ControlsUpdated(){
    
        Print("Updating player controls, for: " +SlotNumber+", split: "+IsSplitSlot);
    }
}


// This keeps all the old values in the same place, this is needed to know which parts are updated //
class PlayerDataHolder{
    
    PlayerDataHolder(){
        
        ThePlayers = array<PlayerData@>(4);
        
        for(uint i = 0; i < ThePlayers.length(); i++){
            // Create an empty object //
            @ThePlayers[i] = @PlayerData(i);
        }
    }
    
    // Updates all data ignoring all data change checks; everything will be updated //
    void UpdateAllData(){
        
        // Avoids the code duplication...
        UpdateDataIfChanged(true);
    }
    
    // Updates only the data that has been changed since last call //
    void UpdateDataIfChanged(bool ignorechanges = false){
        
        PongBase@ game = GetPongBase();
        
        for(int i = 0; i < 4; i++){
            
            PlayerSlot@ currentplayer = game.GetSlot(i);
            
            ThePlayers[i].CheckUpdates(currentplayer, ignorechanges);
        }
    }
    
    
    private array<PlayerData@> ThePlayers;
}




