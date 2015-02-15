// ------------------ AI Variables ------------------ //


enum REALDIRECTION {
    REALDIRECTION_RIGHT,
    REALDIRECTION_LEFT
};
enum AISTATE {
    AISTATE_IDLING,
    AISTATE_BLOCKINGBALL,
    AISTATE_TRYINGTOINTERCEPT
};

CONTROLKEYACTION GetRealActionForSlot(PlayerSlot@ slot, REALDIRECTION absolutedir){
    // Translate based on verticalness of the slot //
    if(slot.IsVerticalSlot()){
        switch(absolutedir){
            case REALDIRECTION_RIGHT: return CONTROLKEYACTION_POWERUPUP;
            case REALDIRECTION_LEFT: return CONTROLKEYACTION_POWERUPDOWN;
        }
    } else {
        switch(absolutedir){
            case REALDIRECTION_RIGHT: return CONTROLKEYACTION_LEFT;
            case REALDIRECTION_LEFT: return CONTROLKEYACTION_RIGHT;
        }
    }
    
    Print("Don't do this");
    return CONTROLKEYACTION_RIGHT;
}

//! Moves slots paddle towards TargetPercentage
void MoveTowardsProgress(PlayerSlot@ AISlot, float TargetPercentage){
    // Set right direction based on the target progress //
    float curprogress = AISlot.GetTrackProgress();
    if(curprogress > TargetPercentage+0.01){
            
        AISlot.PassInputAction(GetRealActionForSlot(AISlot, REALDIRECTION_RIGHT), false);    
        AISlot.PassInputAction(GetRealActionForSlot(AISlot, REALDIRECTION_LEFT), true);
            
    } else if(curprogress < TargetPercentage-0.01){
        
        AISlot.PassInputAction(GetRealActionForSlot(AISlot, REALDIRECTION_LEFT), false);
        AISlot.PassInputAction(GetRealActionForSlot(AISlot, REALDIRECTION_RIGHT), true);
            
    } else {
        // Reset move //
        AISlot.PassInputAction(GetRealActionForSlot(AISlot, REALDIRECTION_RIGHT), false);
        AISlot.PassInputAction(GetRealActionForSlot(AISlot, REALDIRECTION_LEFT), false);
    }
}
    

// ------------------ AI data structures ------------------ //
class AIDataCache{

    AIDataCache(PlayerSlot@ slot){
        @AISlot = @slot;
        NotMovedToBall = 0;
        TargetPercentage = 0.5;
        AiState = AISTATE_IDLING;
    }
    ~AIDataCache(){
    }


    //! Moves towards the set target and updates the cache variable
    void CommonAIEnd(){

        // Set cache variable //
        // Should be unique enough //
        string targetname = "Paddle_target_"+AISlot.GetPlayerNumber();

        GetAINetworkCache().SetVariable(ScriptSafeVariableBlock(targetname, TargetPercentage));

        MoveTowardsProgress(AISlot, TargetPercentage);
    }
    
    // ------------------ The main AI think functions ------------------ //  
    void RunAINormal(int mspassed){
        
        Prop@ ballptr = GetPongBase().GetBall();
        Float3 ballvelocity = ballptr.GetVelocity();
        Float3 ballpos = ballptr.GetPosition()+(ballvelocity.Normalize()*1.1f);
        Float3 endpos = ballpos+(ballvelocity*10.f);
        
        // User raycasting to detect what the ball is going to hit //
        RayCastHitEntity@ hit = GetPongBase().GetGameWorld().CastRayGetFirstHit(ballpos, endpos);
        
        Float3 hitpos = hit.GetPosition();
        
        // Stop moving if the ball is about to hit the paddle //
        NewtonBody@ paddlebody = AISlot.GetPaddle().CustomMessageGetNewtonBody();
        NewtonBody@ goalbody = AISlot.GetGoalArea().CustomMessageGetNewtonBody();
        
        float curprogress = AISlot.GetTrackProgress();
        
        if(hit.DoesBodyMatchThisHit(paddlebody) && AiState == AISTATE_TRYINGTOINTERCEPT){
            NotMovedToBall = 0;
            TargetPercentage = AISlot.GetTrackProgress();
            // Set state //
            AiState = AISTATE_BLOCKINGBALL;
            
        
        } else if(hit.DoesBodyMatchThisHit(goalbody) && AiState != AISTATE_BLOCKINGBALL){
            
            NotMovedToBall = 0;
            // We need to try to block the ball //
            // Get track controller positions //
            TrackEntityController@ track = AISlot.GetTrackController();
            Float3 startpos = track.GetCurrentNodePosition();
            endpos = track.GetNextNodePosition();
            
            // Change state //
            AiState = AISTATE_TRYINGTOINTERCEPT;
            
            float disttostart;
            float disttoend;
            
            if(abs(startpos.GetZ()-endpos.GetZ()) > abs(startpos.GetX()-endpos.GetX())){
                // Only Z distance //
                disttostart = abs((startpos-hitpos).GetZ());
                disttoend = abs((endpos-hitpos).GetZ());
            } else {
                // Only X distance //
                disttostart = abs((startpos-hitpos).GetX());
                disttoend = abs((endpos-hitpos).GetX());
            }
            
            TargetPercentage = disttostart/(disttostart+disttoend);
        } else {
            // Set to idle if we have hit the ball (or no one else has) //
            int lasthitid = GetPongBase().GetLastHitPlayer();
            
            if(lasthitid == -1 || AISlot.DoesPlayerNumberMatchThisOrParent(lasthitid)){
                AiState = AISTATE_IDLING;
            }
        }
        
        if(AiState == AISTATE_IDLING){
            NotMovedToBall += mspassed;
            if(NotMovedToBall >= 1500){
                NotMovedToBall = 0;
                TargetPercentage = 0.5;
            }
        }

        CommonAIEnd();
    }
    
    void RunAITracker(int mspassed){
        
        // Get the ball location along our axis and set our progress //
        
        Prop@ ballptr = GetPongBase().GetBall();
        Float3 ballpos = ballptr.GetPosition();
        
        float curprogress = AISlot.GetTrackProgress();
        
        // Get track controller positions //
        TrackEntityController@ track = AISlot.GetTrackController();
        Float3 startpos = track.GetCurrentNodePosition();
        Float3 endpos = track.GetNextNodePosition();
            
        float disttostart;
        float disttoend;
            
        if(abs(startpos.GetZ()-endpos.GetZ()) > abs(startpos.GetX()-endpos.GetX())){
            // Only Z distance //
            disttostart = abs((startpos-ballpos).GetZ());
            disttoend = abs((endpos-ballpos).GetZ());
        } else {
            // Only X distance //
            disttostart = abs((startpos-ballpos).GetX());
            disttoend = abs((endpos-ballpos).GetX());
        }
            
        TargetPercentage = disttostart/(disttostart+disttoend);

        CommonAIEnd();
    }
    
    void RunAICombined(int mspassed){
        
        Prop@ ballptr = GetPongBase().GetBall();
        Float3 ballvelocity = ballptr.GetVelocity();
        Float3 ballpos = ballptr.GetPosition()+(ballvelocity.Normalize()*1.1f);
        Float3 endpos = ballpos+(ballvelocity*10.f);
        
        // User raycasting to detect what the ball is going to hit //
        RayCastHitEntity@ hit = GetPongBase().GetGameWorld().CastRayGetFirstHit(ballpos, endpos);
        
        Float3 hitpos = hit.GetPosition();
        
        // Stop moving if the ball is about to hit the paddle //
        NewtonBody@ paddlebody = AISlot.GetPaddle().CustomMessageGetNewtonBody();
        NewtonBody@ goalbody = AISlot.GetGoalArea().CustomMessageGetNewtonBody();
        
        float curprogress = AISlot.GetTrackProgress();
        
        if(hit.DoesBodyMatchThisHit(paddlebody)){
            NotMovedToBall = 0;
            TargetPercentage = AISlot.GetTrackProgress();
            // Set state //
            AiState = AISTATE_BLOCKINGBALL;
            
        
        } else if(hit.DoesBodyMatchThisHit(goalbody) && AiState != AISTATE_BLOCKINGBALL){
            
            NotMovedToBall = 0;
            // We need to try to block the ball //
            // Get track controller positions //
            TrackEntityController@ track = AISlot.GetTrackController();
            Float3 startpos = track.GetCurrentNodePosition();
            endpos = track.GetNextNodePosition();
            
            // Change state //
            AiState = AISTATE_BLOCKINGBALL;
            
            float disttostart;
            float disttoend;
            
            if(abs(startpos.GetZ()-endpos.GetZ()) > abs(startpos.GetX()-endpos.GetX())){
                // Only Z distance //
                disttostart = abs((startpos-hitpos).GetZ());
                disttoend = abs((endpos-hitpos).GetZ());
            } else {
                // Only X distance //
                disttostart = abs((startpos-hitpos).GetX());
                disttoend = abs((endpos-hitpos).GetX());
            }
            
            TargetPercentage = disttostart/(disttostart+disttoend);
        }
        
        if(AiState != AISTATE_IDLING){
            NotMovedToBall += mspassed;
            
            if(NotMovedToBall >= 1500){
                NotMovedToBall = 0;
                AiState = AISTATE_IDLING;
                TargetPercentage = 0.5;
            }
        }
        
        if(AiState == AISTATE_IDLING){
            // Get the ball location along our axis and set our progress //
            
            // Get track controller positions //
            TrackEntityController@ track = AISlot.GetTrackController();
            Float3 startpos = track.GetCurrentNodePosition();
            endpos = track.GetNextNodePosition();
                
            float disttostart;
            float disttoend;
                
            if(abs(startpos.GetZ()-endpos.GetZ()) > abs(startpos.GetX()-endpos.GetX())){
                // Only Z distance //
                disttostart = abs((startpos-ballpos).GetZ());
                disttoend = abs((endpos-ballpos).GetZ());
            } else {
                // Only X distance //
                disttostart = abs((startpos-ballpos).GetX());
                disttoend = abs((endpos-ballpos).GetX());
            }
                
            TargetPercentage = disttostart/(disttostart+disttoend);
        }
        
        
        CommonAIEnd();
    }


    // How long in milliseconds last time ball was coming towards our goal //
    int NotMovedToBall;
    
    // Target percentage for the paddle //
    float TargetPercentage;
    AISTATE AiState;
    PlayerSlot@ AISlot;
};
// ------------------ Utility functions ------------------ //
// Stores all existing AIs //
array<AIDataCache@> ExistingAIs;

AIDataCache@ GetAIForSlot(PlayerSlot@ slot){
    // Loop through all and add new if not found //
    for(uint i = 0; i < ExistingAIs.length(); i++){
        if(ExistingAIs[i].AISlot is slot)
            return ExistingAIs[i];
    }
    // Not found, add new //
    ExistingAIs.insertLast(AIDataCache(slot));
    return ExistingAIs[(ExistingAIs.length()-1)];
}

void ClearCache(){
    ExistingAIs.resize(0);
}

// ------------------ Game call entry points ------------------ //  
// Stupid AI that just wiggles //
void AIThinkDummy(GameModule@ mod, PlayerSlot@ slot, int mspassed){
    // TODO: reimplement dummy
}

// More advanced AI that actually plays the game //
void SimpleAI(GameModule@ mod, PlayerSlot@ slot, int mspassed){
    
    GetAIForSlot(slot).RunAINormal(mspassed);
}
// Quite difficult "hax" AI //
void BallTrackerAI(GameModule@ mod, PlayerSlot@ slot, int mspassed){
    
    GetAIForSlot(slot).RunAITracker(mspassed);   
}
// Combined hax and raytrace AI //
void CombinedAI(GameModule@ mod, PlayerSlot@ slot, int mspassed){

    GetAIForSlot(slot).RunAICombined(mspassed); 
}

// AI client prediction
void AIClientSide(GameModule@ mod, PlayerSlot@ slot, int mspassed){

    string targetname = "Paddle_target_"+AISlot.GetPlayerNumber();

    auto variable = GetAINetworkCache().GetVariable(targetname);

    if(variable is null)
        return;

    float target = float(variable);
    
    MoveTowardsProgress(slot, target);
}

// ------------------ Listener Functions ------------------ //
[@Listener="OnInit"]
void OnLoad(GameModule@ module, Event@ event){
    // We pretty much have nothing to load here... //
    Print("AI loaded and working");
}

[@Listener="OnRelease"]
void OnRelease(GameModule@ module, Event@ event){
    // Nothing to release
    Print("Unloading AI");
}

[@Listener="Generic", @Type="MatchEnded"]
void OnMatchEnd(GameModule@ module, Event@ event){
    // Clear AI cache //
    ClearCache();
}
