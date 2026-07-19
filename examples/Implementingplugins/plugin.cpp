#include <iostream>
#include "PluginBase.hpp"
#include "PluginManager.hpp"
#include "PluginInfo.hpp"
#include "DynamicLibrary.hpp"

int main()
{
#ifndef _WIN32
	auto w = LoadPlugin("./libTestPluginOne.so");
#else
	auto w = LoadPlugin("libTestPluginOne.dll");
#endif
	auto p = PluginManager::Instance().Create("TestPluginOne");

	std::cout << " name = " << p->GetPluginClassName() << std::endl;
	std::cout << " address = " << p << std::endl;


	PluginInfo** plugins;
	uint32_t pluginCount = PluginManager::Instance().GetKnownPluginsList(plugins);

	for (int i = 0; i < pluginCount; ++i)
	{
		std::cout << "[" << i << "]: PluginClassName= " << plugins[i]->GetPluginClassName() << std::endl;
	}

	PluginManager::Instance().FreeKnownPluginsList(plugins, pluginCount);

	p->InitializePlugin();

	p->TerminatePlugin();

	PluginManager::Instance().Destroy(p);

	UnloadPlugin(w);

	return 0;
}

