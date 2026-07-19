#include "PluginManager.hpp"
#include "PluginInfo.hpp"
#include <map>
#include <string>

#ifndef USE_BOOST
#include <mutex>
#else
#include <boost/thread.hpp>
#include <atomic>

using namespace boost;
#endif

using namespace std;


class PluginManagerImplementation
{
	protected:
		map<string, PluginInfo*> Plugins;
		recursive_mutex		PluginSync;

	friend class PluginManager;
};

PluginManager::PluginManager() : Implementation(new PluginManagerImplementation())
{ 
}

PluginManager::~PluginManager()
{
	for (auto plugin :  Implementation->Plugins)
		delete plugin.second;

	delete Implementation;
}

PluginManager& PluginManager::Instance()
{
#ifdef _WIN32
	static std::atomic_flag lock = ATOMIC_FLAG_INIT;
	while (lock.test_and_set(std::memory_order_acquire));
#endif
	static PluginManager instance;
#ifdef _WIN32
	lock.clear(std::memory_order_release);
#endif

	return instance;	
}


void PluginManager::Register(const char *className, const char *pluginName, uint16_t MajorVersion, uint16_t MinorVersion, uint32_t Build, 
				PluginCreateMethod_t createMethod, PluginDestroyMethod_t destroyMethod)
{
	unique_lock<recursive_mutex> lock(Implementation->PluginSync);

	auto it = Implementation->Plugins.find(className);

	if (it == Implementation->Plugins.end())
	{
		/* because of our design, we only care about adds one time, duplicates mean nothing and happen just because of the static definition */
		Implementation->Plugins.insert(make_pair(className, new PluginInfo(className, pluginName, PLUGIN_VERSION_INFO(MajorVersion, MinorVersion, Build), 							createMethod, destroyMethod))); 			
	}
}

void PluginManager::Unregister(const char *className)
{
	unique_lock<recursive_mutex> lock(Implementation->PluginSync);

	auto it = Implementation->Plugins.find(className);

	if (it != Implementation->Plugins.end())
	{
		delete it->second;

		Implementation->Plugins.erase(it);
	}

}

PluginInfo* PluginManager::GetPluginInfo(const char* className)
{
	unique_lock<recursive_mutex> lock(Implementation->PluginSync);

	auto it = Implementation->Plugins.find(className);


	if (it != Implementation->Plugins.end())
	{
		return it->second;
	}
	
	return nullptr;
}

IPluginBase* PluginManager::Create(const char* className)
{
	unique_lock<recursive_mutex> lock(Implementation->PluginSync);

	auto it = Implementation->Plugins.find(className);


	if (it != Implementation->Plugins.end())
	{
		return it->second->GetPluginCreate()();
	}


	return nullptr;
}

void PluginManager::Destroy(IPluginBase* plugin)
{
	unique_lock<recursive_mutex> lock(Implementation->PluginSync);

	auto it = Implementation->Plugins.find(plugin->GetPluginClassName());


	if (it != Implementation->Plugins.end())
	{
		it->second->GetPluginDestroy()(plugin);
	}


}


uint32_t PluginManager::GetKnownPluginsList(PluginInfo**& plugins)
{
	unique_lock<recursive_mutex> lock(Implementation->PluginSync);

	uint32_t i = 0;
	plugins = new PluginInfo*[Implementation->Plugins.size()];

	for (auto plugin : Implementation->Plugins)
	{
		plugins[i] = new PluginInfo(*plugin.second);		
		++i;
	}

	return i;
}

void PluginManager::FreeKnownPluginsList(PluginInfo** plugins, uint32_t size)
{
	for (uint32_t i = 0; i < size; ++i)
		delete plugins[i];
	delete[] plugins;
}

