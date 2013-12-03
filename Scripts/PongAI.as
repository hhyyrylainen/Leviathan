// ------------------ AI Variables ------------------ //


enum REALDIRECTION {REALDIRECTION_RIGHT, REALDIRECTION_LEFT};


CONTROLKEYACTION GetRealActionForSlot(PlayerSlot@ slot, REALDIRECTION absolutedir){
    // Translate based on verticalness of the slot //
    if(slot.IsVerticalSlot()){
        switch(absolutedir){
            case REALDIRECTION_RIGHT: return CONTROLKEYACTION_POWERUPUP;
            case REALDIRECTION_LEFT: return CONTROLKEYACTION_POWERUPDOWN;
        }
    } else {
        switch(absolutedir){
            case REALDIRECTION_RIGHT: return CONTROLKEYACTION_RIGHT;
            case REALDIRECTION_LEFT: return CONTROLKEYACTION_LEFT;
        }
    }
    Print("Don't do this");
    return CONTROLKEYACTION_RIGHT;
}
// ------------------ AI data structures ------------------ //
class AIDataCache{

    AIDataCache(PlayerSlot@ slot){
        @AISlot = @slot;
        NotMovedToBall = 0;
        TargetPercentage = 0.5;
    }
    ~AIDataCache(){
        Print("Unloading AI cache");
    }
    // ------------------ The main AI think functions ------------------ //  
    void RunAINormal(int mspassed){
        
        NotMovedToBall += mspassed;
        
        Prop@ ballptr = GetPongGame().GetBall();
        Float3 ballvelocity = ballptr.GetVelocity();
        Float3 ballpos = ballptr.GetPosition()+(ballvelocity.Normalize()*2);
        Float3 endpos = ballpos+(ballvelocity*10.f);
        
        // User raycasting to detect what the ball is going to hit //
        RayCastHitEntity@ hit = GetPongGame().GetGameWorld().CastRayGetFirstHit(ballpos, endpos);
        
        Float3 hitpos = hit.GetPosition();
        
        // Stop moving if the ball is about to hit the paddle //
        NewtonBody@ paddlebody = AISlot.GetPaddle().CustomMessageGetNewtonBody();
        NewtonBody@ goalbody = AISlot.GetGoalArea().CustomMessageGetNewtonBody();
        
        float curprogress = AISlot.GetTrackProgress();
        
        if(hit.DoesBodyMatchThisHit(paddlebody)){
            NotMovedToBall = 0;
            TargetPercentage = AISlot.GetTrackProgress();
        
        } else if(hit.DoesBodyMatchThisHit(goalbody)){
            
            NotMovedToBall = 0;
            // We need to try to block the ball //
            // Get track controller positions //
            TrackEntityController@ track = AISlot.GetTrackController();
            Float3 startpos = track.GetCurrentNodePosition();
            Float3 endpos = track.GetNextNodePosition();
            
            Print("Hit: "+hitpos.GetX()+" "+hitpos.GetY()+" "+hitpos.GetZ());
            
            float distance = 0;
            
            Float3 activeaxis = endpos-startpos;
            Float3 tmp = hitpos-startpos;
            // Figure out the distance //
            if(activeaxis.GetX() != 0.f)
                distance += abs(tmp.GetX());
            if(activeaxis.GetZ() != 0.f)
                distance += abs(tmp.GetZ());
                
            Print("Distance: "+distance);
            
            float tmptarget = distance/activeaxis.HAddAbs();
            TargetPercentage = /*1.f-*/tmptarget;
            
            Print("Target: "+TargetPercentage);
        }
        
        if(NotMovedToBall > 1000){
            // Move to the center //
            TargetPercentage = 0.5;
            // Or maybe a random spot... //
        }
        if(TargetPercentage > 0.99f)
            TargetPercentage = 0.99f;
        AISlot.GetTrackController().SetProgressTowardsNextNode(TargetPercentage);
        return;
        
        // Move towards the percentage //

        
        // Set right direction based on the target progress //
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
        // Now we're happy //
    }
    


    // How long in milliseconds last time ball was coming towards our goal //
    int NotMovedToBall;
    // Target percentage for the paddle //
    float TargetPercentage;
    
    PlayerSlot@ AISlot;
};
// ------------------ Utility functions ------------------ //
// Stores all existing AIs //
array<AIDataCache@> ExistingAIs;

AIDataCache@ GetAIForSlot(PlayerSlot@ slot){
    // Loop through all and add new if not found //
    for(uint i = 0; i < ExistingAIs.size(); i++){
        if(ExistingAIs[i].AISlot is slot)
            return ExistingAIs[i];
    }
    // Not found, add new //
    ExistingAIs.push_back(AIDataCache(slot));
    return ExistingAIs[(ExistingAIs.size()-1)];
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
