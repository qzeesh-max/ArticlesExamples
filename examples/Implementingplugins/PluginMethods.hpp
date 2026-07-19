#ifndef PluginMethods_HPP
#define PluginMethods_HPP

class IPluginBase;

typedef IPluginBase* (*PluginCreateMethod_t)();
typedef void (*PluginDestroyMethod_t)(IPluginBase* pluginBase);

#endif
