#pragma once
#include "Module.h"
#include "Globals.h"

class b2World;
class ModulePhysics : public Module
{
public:
	ModulePhysics(Application* app, bool start_enabled = true);
	~ModulePhysics();

	bool Start();
	update_status PreUpdate();
	update_status PostUpdate();
	bool CleanUp();
	
	b2World* world = nullptr;
	
	
	

	void Shape(float, float, float, bool);

	

private:

	bool debug;
};