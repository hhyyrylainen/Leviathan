
#include "Events/CallableObject.h"

class TestCallable : public Leviathan::CallableObject {
public:
	TestCallable() : IsEvented(false){};

	int OnEvent(Event** pEvent) {
		IsEvented = true;
		return 1;
	}

	DLLEXPORT virtual int OnGenericEvent(GenericEvent** pevent){
		throw std::exception();
	}

	bool IsEvented;
};
