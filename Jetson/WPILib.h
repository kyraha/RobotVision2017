class Preferences {
private:
	static Preferences* instance;
public:
	static Preferences* GetInstance() { return instance; };
	float GetFloat(const char *name, float value) {return value;};

};

