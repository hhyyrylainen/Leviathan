
#include "Events/AutoUpdateable.h"

class TestAutoUpdateable : public Leviathan::AutoUpdateableObject{
public:
	bool HasUpdated(){
		return ValuesUpdated;
	}





};
