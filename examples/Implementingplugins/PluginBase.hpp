#ifndef PluginBase_HPP
#define PluginBase_HPP

#include "IPluginBase.hpp"
#include "PluginRegistration.hpp"




template <typename T, const char** className, const char ** pluginName, uint16_t MajorVersion = 0, uint16_t MinorVersion = 0, uint32_t Build = 0>
class PluginBase : public IPluginBase
{
	public:
		/*
			Friendly name of the plugin. String returned by the plugin must be valid until the plugin object is destroyed.
		*/
		virtual const char * GetPluginName()
		{
			return *pluginName;
		}
	
		/*
			Version information for the plugin.
		*/
		virtual void GetPluginVersion(PLUGIN_VERSION_INFO& outPluginVersionInfo)
		{
			outPluginVersionInfo = PLUGIN_VERSION_INFO(MajorVersion, MinorVersion, Build);
		}
	
		/*
			Initialize the plugin. 
	
			@Return = 0 for success, other = errno
		*/
		virtual uint32_t InitializePlugin()=0;
	
		/*
			Terminates the plugin.

			@Return = 0 for success, other = errno
		*/
		virtual uint32_t TerminatePlugin()=0;

		/*
			Name of the Plugin Class used to create a plugin. String returned by the plugin must be valid until the plugin object is destroyed.
		*/
    		virtual const char* GetPluginClassName()
		{
			return *className;
		}
	
		static IPluginBase* Create()
		{
			return new T();
		}

		static void Destroy(IPluginBase* plugin)
		{
			delete plugin;
		}
};


#define DEFINE_PLUGIN(type,pluginName,...) extern const char* PLUGIN_INFO_ ## type ## _CLASSNAME = #type;\
	extern const char* PLUGIN_INFO_ ## type ## _PLUGINNAME = pluginName;\
	class type : public PluginBase<type, &PLUGIN_INFO_ ## type ## _CLASSNAME, &PLUGIN_INFO_ ## type ## _PLUGINNAME, __VA_ARGS__> 


#define IMPLEMENT_PLUGIN(type,pluginName,...) \
	static PluginRegistration<type, &PLUGIN_INFO_ ## type ## _CLASSNAME, &PLUGIN_INFO_ ## type ## _PLUGINNAME, __VA_ARGS__> PLUGIN_INFO_## type ## _REGISTRATION\
		(&PluginBase<type, &PLUGIN_INFO_ ## type ## _CLASSNAME, &PLUGIN_INFO_ ## type ## _PLUGINNAME, __VA_ARGS__>::Create,\
		&PluginBase<type, &PLUGIN_INFO_ ## type ## _CLASSNAME, &PLUGIN_INFO_ ## type ## _PLUGINNAME, __VA_ARGS__>::Destroy);\


#endif
