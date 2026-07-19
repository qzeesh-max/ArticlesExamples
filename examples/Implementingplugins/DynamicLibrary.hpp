#ifndef DYNAMICLIBRARY_HPP
#define DYNAMICLIBRARY_HPP


#ifdef _WIN32

#include <windows.h>

typedef HMODULE PLUGINHANDLE;

#else

#include <dlfcn.h>

typedef void* PLUGINHANDLE;

#endif


inline PLUGINHANDLE LoadPlugin(const char * library)
{
#ifndef _WIN32
	auto w = dlopen(library, RTLD_NOW);
#else
	auto w = LoadLibrary(library);
#endif

	return w;
}


inline void UnloadPlugin(PLUGINHANDLE library)
{
#ifndef _WIN32
	dlclose(library);
#else
	FreeLibrary(library);
#endif
}

#endif
