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

        // delegate to helper //
        ScriptSystemNodeHelper(World, CachedComponents, SystemComponents);
    }

    private StandardWorld@ World;
    private array<ScriptSystemUses> SystemComponents = {
        ScriptSystemUses("CoolTimer"), ScriptSystemUses(Position::TYPE)
    };

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
