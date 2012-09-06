//
//  main.cpp
//  Joint Blender
//
//  Created by Samuel Williams on 9/06/12.
//  Copyright (c) 2012 Orion Transfer Ltd. All rights reserved.
//

#include <Dream/Client/Client.h>
#include <Dream/Client/Display/Application.h>

#include "MazeSolverScene.h"

int main (int argc, const char * argv[])
{
	using namespace Dream;
	using namespace Dream::Core;
	using namespace Dream::Client::Display;
	
	Ref<Dictionary> config = new Dictionary;
	IApplication::run_scene(new MazeSolver::MazeSolverScene, config);
	
	return 0;
}
