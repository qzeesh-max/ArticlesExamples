#include "PluginBase.hpp"
#include <iostream>

DEFINE_PLUGIN(TestPluginOne, "TestPluginOne", 4, 0, 0)
{
	/*
		Initialize the plugin. 
	
		@Return = 0 for success, other = errno
	*/
	virtual uint32_t InitializePlugin()
	{
		std::cout << "Hello from InitializePlugin" << std::endl;
		return 0;
	}
	
	/*
		Terminates the plugin.

		@Return = 0 for success, other = errno
	*/
	virtual uint32_t TerminatePlugin()
	{
		std::cout << "Bye from InitializePlugin" << std::endl;
		return 0;
	}
};

IMPLEMENT_PLUGIN(TestPluginOne, "TestPluginOne", 4, 0, 0);
