//! \file Test for leaking memory due to delegates and anoynmous functions

class Organelle{

    Organelle(const string &in name, int cost)
    {
        this.cost = cost;
        this.name = name;
    }
    
    int cost;
    string name;    
}

class PlacedOrganelle{

    // Default constructor doesn't help
    // PlacedOrganelle(){}

    PlacedOrganelle(Organelle@ organelle, int q, int r, int rotation)
    {
        @this._organelle = organelle;
        this.q = q;
        this.r = r;
        this.rotation = rotation;
    }

    PlacedOrganelle(PlacedOrganelle@ typefromother, int q, int r, int rotation)
    {
        @this._organelle = typefromother._organelle;
        this.q = q;
        this.r = r;
        this.rotation = rotation;
    }

    PlacedOrganelle(PlacedOrganelle@ other)
    {
        @this._organelle = other._organelle;
        this.q = other.q;
        this.r = other.r;
        this.rotation = other.rotation;

        // _commonConstructor();
    }

    ~PlacedOrganelle()
    {
    }

    int q;
    int r;
    int rotation;

    const Organelle@ organelle {
        get const{
            return _organelle;
        }
    }

    private Organelle@ _organelle;
}

void RunTest()
{
    PlacedOrganelle@ organelle = PlacedOrganelle(Organelle("cytoplasm", 10), 1, 1, 0);

    dictionary data;

    @data["organelle"] = organelle;

    PlacedOrganelle@ organelle2 = cast<PlacedOrganelle>(data["organelle"]);
}
