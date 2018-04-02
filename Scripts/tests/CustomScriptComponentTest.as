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

    CoolSystem@ system = cast<CoolSystem>(world.GetScriptSystem("CoolSystem"));

    if(system is null)
        return false;
    

    StandardWorld@ asStandard = cast<StandardWorld>(world);

    // ------------------------------------ //
    // Entity 1
    ObjectID id1 = world.CreateEntity();
    CoolTimer@ timer1 = cast<CoolTimer>(
        world.GetScriptComponentHolder("CoolTimer").Create(id1));

    timer1.TimeValue = 1;
    
    asStandard.Create_Position(id1, Float3(1, 0, 0), Float4::IdentityQuaternion);
    
    
    // ------------------------------------ //
    // Entity 2
    ObjectID id2 = world.CreateEntity();
    CoolTimer@ timer2 = cast<CoolTimer>(
        world.GetScriptComponentHolder("CoolTimer").Create(id2));

    timer2.TimeValue = 2;

    asStandard.Create_Position(id2, Float3(2, 0, 0), Float4::IdentityQuaternion);

    // ------------------------------------ //
    // Entity 3
    ObjectID id3 = world.CreateEntity();
    CoolTimer@ timer3 = cast<CoolTimer>(
        world.GetScriptComponentHolder("CoolTimer").Create(id3));

    timer3.TimeValue = 7;

    asStandard.Create_Position(id3, Float3(4, 0, 0), Float4::IdentityQuaternion);

    return true;
}

int VerifyRunResult(GameWorld@ world){

    auto index = world.GetScriptComponentHolder("CoolTimer").GetIndex();

    if(index.length() != 3)
        return -1;

    int value = 0;
    
    for(uint i = 0; i < index.length(); ++i){

        CoolTimer@ timer = cast<CoolTimer>(
            world.GetScriptComponentHolder("CoolTimer").Find(index[i]));

        value += timer.TimeValue;
    }

    return value;
}

array<ObjectID>@ BeforeRemoveIndex;

bool RemoveSomeComponents(GameWorld@ world){

    // Verify that the cached count is correct
    CoolSystem@ system = cast<CoolSystem>(world.GetScriptSystem("CoolSystem"));

    if(system.CachedComponents.length() != 3)
        return false;

    @BeforeRemoveIndex = world.GetScriptComponentHolder("CoolTimer").GetIndex();

    if(BeforeRemoveIndex.length() != 3)
        return false;

    world.DestroyEntity(BeforeRemoveIndex[0]);
    world.GetScriptComponentHolder("CoolTimer").Destroy(BeforeRemoveIndex[1]);

    if(world.GetScriptComponentHolder("CoolTimer").Find(BeforeRemoveIndex[1]) !is null)
        return false;
    
    return true;
}

bool VerifyRemoved(GameWorld@ world){

    CoolSystem@ system = cast<CoolSystem>(world.GetScriptSystem("CoolSystem"));

    if(system.CachedComponents.length() != 1)
        return false;

    if(system.CachedComponents[0].ID != BeforeRemoveIndex[2])
        return false;

    return true;
}


class SecondTimer : ScriptComponent{


    int TimeValue = 0;
};

class CombinedCached{

    CombinedCached(ObjectID id, SecondTimer@ first, CoolTimer@ second, Position@ third)
    {
        ID = id;
        @First = first;
        @Second = second;
        @Third = third;
    }

    ObjectID ID;
    SecondTimer@ First;
    CoolTimer@ Second;
    Position@ Third;
};

class CombinedSystem : ScriptSystem{

    void Init(GameWorld@ world){

        @World = cast<StandardWorld@>(world);
    }

    void Release(){

    }

    void Run(){

        for(uint i = 0; i < CachedComponents.length(); ++i){

            CombinedCached@ cached = CachedComponents[i];

            cached.Second.TimeValue += int(cached.Third._Position.X) + cached.First.TimeValue;
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
        ScriptSystemUses("SecondTimer"),
        ScriptSystemUses("CoolTimer"),
        ScriptSystemUses(Position::TYPE)
    };

    array<CombinedCached@> CachedComponents;
};


ScriptComponent@ SecondFactory(GameWorld@ world){

    return SecondTimer();
}


bool SetupCustomComponentsMultiple(GameWorld@ world){

    world.RegisterScriptComponentType("CoolTimer", @CoolFactory);
    world.RegisterScriptComponentType("SecondTimer", @SecondFactory);
    world.RegisterScriptSystem("CoolSystem", CoolSystem());
    world.RegisterScriptSystem("CombinedSystem", CombinedSystem());
    
    return true;
}
