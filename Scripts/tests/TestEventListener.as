
EventListener@ listener;

int value = 0;

int OnGeneric(GenericEvent@ event){

    NamedVars@ vars = event.GetNamedVars();

    value += int(vars.GetSingleValueByName("val"));
    return 1;
}

class ListeningClass{
    
    ListeningClass(string event){

        @listener = EventListener(null, OnGenericEventCallback(this.OnGeneric));
        assert(listener.RegisterForEvent(event));
    }

    int OnGeneric(GenericEvent@ event){

        NamedVars@ vars = event.GetNamedVars();

        value += int(vars.GetSingleValueByName("val"));
        return 1;
    }

    private EventListener@ listener;
}

ListeningClass@ obj;

bool StartListen(string event){

    @listener = EventListener(null, @OnGeneric);
    if(!listener.RegisterForEvent(event)){
        return false;
    }

    @obj = ListeningClass(event);
    return true;
}

int CheckListen(){
    return value * 2;
}

