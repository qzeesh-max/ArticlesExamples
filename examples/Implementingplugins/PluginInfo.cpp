#include "PluginInfo.hpp"
#include "PluginBase.hpp"
#include "PluginRegistration.hpp"
#include <string>

class PluginInfoImpl
{
	private:
		std::string className, pluginName;
		PLUGIN_VERSION_INFO pluginVersionInfo;
		PluginCreateMethod_t pluginCreate;
		PluginDestroyMethod_t pluginDestroy;

	public:
		PluginInfoImpl(const std::string& className, const std::string& pluginName, const PLUGIN_VERSION_INFO& pluginVersionInfo, 
				PluginCreateMethod_t pluginCreate, PluginDestroyMethod_t pluginDestroy):
			className(className), pluginName(pluginName), pluginVersionInfo(pluginVersionInfo), pluginCreate(pluginCreate), pluginDestroy(pluginDestroy)
		{
		}


		const char * GetPluginClassName() const
		{
			return className.c_str();
		}

		const char * GetPluginName() const
		{
			return pluginName.c_str();
		}

		const PLUGIN_VERSION_INFO & GetPluginVersion() const
		{
			return pluginVersionInfo;
		}

		PluginCreateMethod_t  GetPluginCreate() const
		{
			return pluginCreate;
		}

		PluginDestroyMethod_t GetPluginDestroy() const
		{
			return pluginDestroy;
		}
	
};





PluginInfo::PluginInfo(const char * className, const char * pluginName, const PLUGIN_VERSION_INFO& pluginVersionInfo, PluginCreateMethod_t pluginCreate, PluginDestroyMethod_t pluginDestroy):
	Implementation(new PluginInfoImpl(className, pluginName, pluginVersionInfo, pluginCreate, pluginDestroy))
{
}

PluginInfo::~PluginInfo()
{
	delete Implementation;
}


const char * PluginInfo::GetPluginClassName() const
{
	return Implementation->GetPluginClassName();
}

const char * PluginInfo::GetPluginName() const
{
	return Implementation->GetPluginName();
}

const PLUGIN_VERSION_INFO& PluginInfo::GetPluginVersion() const
{
	return Implementation->GetPluginVersion();
}

PluginCreateMethod_t PluginInfo::GetPluginCreate() const
{
	return Implementation->GetPluginCreate();
}

PluginDestroyMethod_t PluginInfo::GetPluginDestroy() const
{
	return Implementation->GetPluginDestroy();
}


