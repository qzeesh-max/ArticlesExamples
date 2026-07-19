#ifndef PluginRegistration_H
#define PluginRegistration_H

#include "PluginManager.hpp"
#include "PluginMethods.hpp"

class IPluginBase;

template <typename T, const char** className, const char ** pluginName, uint16_t MajorVersion = 0, uint16_t MinorVersion = 0, uint32_t Build = 0>
class PluginRegistration
{
	public:
		PluginRegistration(PluginCreateMethod_t pluginCreateMethod, PluginDestroyMethod_t pluginDestroyMethod)
		{
			PluginManager::Instance().Register(*className, *pluginName, MajorVersion, MinorVersion, Build, pluginCreateMethod, pluginDestroyMethod);
							  
		}

		~PluginRegistration()
		{
			PluginManager::Instance().Unregister(*className);
		}
};

#endif
