#ifndef IPLUGINBASE_HPP
#define IPLUGINBASE_HPP

#include <stdint.h>

#ifdef _WIN32
#ifdef PLUGINDLL_EXPORT
#define PLUGINDLLAPI __declspec(dllexport)
#else
#define PLUGINDLLAPI __declspec(dllimport)
#endif
#else
extern "C" 
{
#define PLUGINDLLAPI
#endif

struct PLUGINDLLAPI PLUGIN_VERSION_INFO
{
public:
	constexpr PLUGIN_VERSION_INFO(uint16_t MajorVersion, uint16_t MinorVersion, uint32_t Build) : 
				MajorVersion(MajorVersion), MinorVersion(MinorVersion), Build(Build)
	{
	}

	uint16_t GetMajorVersion() const
	{
		return MajorVersion;
	}

	uint16_t GetMinorVersion() const
	{
		return MinorVersion;
	}

	uint32_t GetBuild() const
	{
		return Build;
	}

private:
	uint16_t MajorVersion;
	uint16_t MinorVersion;
	uint32_t Build;
};

class PLUGINDLLAPI PluginImplementation
{
};

class PLUGINDLLAPI IPluginBase
{
public: 
	/*
		Friendly name of the plugin. String returned by the plugin must be valid until the plugin object is destroyed.
	*/
	virtual const char * GetPluginName()=0;
	
	/*
		Version information for the plugin.
	*/
	virtual void GetPluginVersion(PLUGIN_VERSION_INFO& outPluginVersionInfo)=0;
	
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
    	virtual const char* GetPluginClassName()=0;

	virtual ~IPluginBase() 
	{
	}

protected:
	PluginImplementation * Implementation;
    
};
#ifndef _WIN32
}
#endif

#endif
