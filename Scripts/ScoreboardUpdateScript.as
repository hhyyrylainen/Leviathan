// common function for updating the scoreboard //
void UpdateScoreboard(BaseGuiObject@ element){

    string text = "";

    // player count is always four (closed slots exist) //
    for(int i = 0; i < 4; i++){
        PlayerSlot@ slot = GetPongGame().GetSlot(i);
        bool teamprinted = false;
        // for looping through split slots //
        while(true){
            
            // skip empty slots //
            if(!slot.IsActive())
                break;
                
            if(!teamprinted){
                teamprinted = true;
                text += "<p>Team "+i+": ";
            }
            
            // get score //
            int score = slot.GetScore();
            int number = slot.GetPlayerNumber();
            text += "<p style='padding-left: 10px;'>Player "+number+": "+score+"</p>";
            
            @slot = slot.GetSplit();
            if(slot is null)
                break;
        }
        if(teamprinted){
            // End the team paragraph //
            text += "</p>";
        }
    }

    element.SetInternalElementRML(text);
}