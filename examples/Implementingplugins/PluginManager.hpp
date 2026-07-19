#ifndef PluginManager_H
#define PluginManager_H

#include <stdint.h>
#include "PluginMethods.hpp"

class PluginManagerImplementation;
class PluginInfo;

#ifdef _WIN32
#ifdef PLUGINDLL_EXPORT
#define PLUGINDLLAPI __declspec(dllexport)
#else
#define PLUGINDLLAPI __declspec(dllimport)
#endif
#else
#define PLUGINDLLAPI
extern "C"
{
#endif

class PLUGINDLLAPI PluginManager
{
	private:
		PluginManagerImplementation * Implementation;

	private:
		PluginManager();
		~PluginManager();

	public:
		static PluginManager& Instance();


		void Register(const char *className, const char *pluginName, uint16_t MajorVersion, uint16_t MinorVersion, uint32_t Build, 
				PluginCreateMethod_t createMethod, PluginDestroyMethod_t destroyMethod);

		void Unregister(const char *className);

		PluginInfo* GetPluginInfo(const char* className);

		IPluginBase* Create(const char* className);
		void Destroy(IPluginBase* plugin);

		uint32_t GetKnownPluginsList(PluginInfo**& plugins);
		void FreeKnownPluginsList(PluginInfo** plugins, uint32_t size);
};

#ifndef _WIN32
}
#endif
#endif
