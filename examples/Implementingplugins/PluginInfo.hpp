#ifndef PLUGININFO_HPP
#define PLUGININFO_HPP

#include "IPluginBase.hpp"
#include "PluginRegistration.hpp"
#include <string>

class PluginInfoImpl;
class IPluginBase;

#ifdef _WIN32
#ifdef PLUGINDLL_EXPORT
#define PLUGINDLLAPI __declspec(dllexport)
#else
#define PLUGINDLLAPI __declspec(dllimport)
#endif
#else
extern "C" {
#endif

class PLUGINDLLAPI PluginInfo
{
private:
	PluginInfoImpl * Implementation;


public:
	PluginInfo(const char * className, const char * pluginName, const PLUGIN_VERSION_INFO& pluginVersionInfo, 
			PluginCreateMethod_t pluginCreate, PluginDestroyMethod_t pluginDestroy);

	~PluginInfo();


	const char *GetPluginClassName() const;
	const char *GetPluginName() const;
	const PLUGIN_VERSION_INFO& GetPluginVersion() const;
	PluginCreateMethod_t GetPluginCreate() const;
	PluginDestroyMethod_t GetPluginDestroy() const;


};

#ifndef _WIN32
}
#endif

#endif

