class CoolTimer : ScriptComponent{


    int TimeValue = 0;
};

class CoolSystemCached{

    ObjectID ID;
    CoolTimer@ First;
    Position@ Second;
};

class CoolSystem : ScriptSystem{

    void Init(GameWorld@ world){

        @World = cast<StandardWorld@>(world);
    }

    void Release(){

    }

    void Run(){

        for(uint i = 0; i < CachedComponents.length(); ++i){

            CoolSystemCached@ cached = CachedComponents[i];

            cached.First.TimeValue += int(cached.Second._Position.X);
        }
    }

    void Clear(){

        CachedComponents.resize(0);
    }

    void CreateAndDestroyNodes(){

        // Creation: delegate to helper //
        

        _RemoveByList(World.GetRemovedIDsForComponents({Position::TYPE}));
        _RemoveByList(World.GetRemovedIDsForScriptComponents({"CoolTimer"}));
    }

    // This should really be a C++ helper...
    private void _RemoveByList(const array<ObjectID> &in todelete){

        for(uint i = 0; i < todelete.length(); ++i){

            const auto lookfor = todelete[i];

            for(uint a = 0; a < CachedComponents.length(); ++a){

                if(CachedComponents[a].ID == lookfor){

                    CachedComponents.removeAt(a);
                    break;
                }
            }
        }
    }

    private StandardWorld@ World;

    array<CoolSystemCached@> CachedComponents;
};


ScriptComponent@ CoolFactory(GameWorld@ world){

    return CoolTimer();
}


bool SetupCustomComponents(GameWorld@ world){

    world.RegisterScriptComponentType("CoolTimer", @CoolFactory);
    world.RegisterScriptSystem("CoolSystem", CoolSystem());
    

    StandardWorld@ asStandard = cast<StandardWorld>(world);

    
    

    return false;
}

int VerifyRunResult(GameWorld@ world){

    return 0;
}
