class CoolTimer : ScriptComponent{


    int TimeValue = 0;
};

class CoolSystemCached{

    CoolSystemCached(ObjectID id, CoolTimer@ first, Position@ second)
    {
        ID = id;
        @First = first;
        @Second = second;
    }

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
            cached.First.TimeValue *= cached.First.TimeValue;
        }
    }

    void Clear(){

        CachedComponents.resize(0);
    }

    void CreateAndDestroyNodes(){

        // Delegate to helper //
        ScriptSystemNodeHelper(World, @CachedComponents, SystemComponents);
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

    // ------------------------------------ //
    // Entity 1
    CoolTimer@ timer1 = cast<CoolTimer>(world.GetScriptComponentHolder("CoolTimer").Create(1));

    timer1.TimeValue = 1;
    
    asStandard.Create_Position(1, Float3(1, 0, 0), Float4::IdentityQuaternion);
    
    
    // ------------------------------------ //
    // Entity 2
    CoolTimer@ timer2 = cast<CoolTimer>(world.GetScriptComponentHolder("CoolTimer").Create(2));

    timer2.TimeValue = 2;

    asStandard.Create_Position(2, Float3(2, 0, 0), Float4::IdentityQuaternion);

    // ------------------------------------ //
    // Entity 3
    CoolTimer@ timer3 = cast<CoolTimer>(world.GetScriptComponentHolder("CoolTimer").Create(3));

    timer3.TimeValue = 7;

    asStandard.Create_Position(3, Float3(4, 0, 0), Float4::IdentityQuaternion);    
    
    return true;
}

int VerifyRunResult(GameWorld@ world){

    CoolTimer@ timer1 = cast<CoolTimer>(world.GetScriptComponentHolder("CoolTimer").Find(1));

    CoolTimer@ timer2 = cast<CoolTimer>(world.GetScriptComponentHolder("CoolTimer").Find(2));

    CoolTimer@ timer3 = cast<CoolTimer>(world.GetScriptComponentHolder("CoolTimer").Find(3));

    return timer1.TimeValue + timer2.TimeValue + timer3.TimeValue;
}
