

class TestCallable : public Leviathan::CallableObject {
public:
	TestCallable() : IsEvented(false){};

	void OnEvent(Event** pEvent) {
		IsEvented = true;
	}

	bool IsEvented;
};