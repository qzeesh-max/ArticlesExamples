#ifndef __ROBUST_SHAREABLE_RECURSIVEMUTEX__H
#define __ROBUST_SHAREABLE_RECURSIVEMUTEX__H

#include <boost/utility.hpp>

#ifdef _WIN32

#include <winbase.h>
#include "shareable_object_slot.hpp"

#else

#include <sstream>
#include <pthread.h>

#endif

class robust_shareable_recursivemutex : boost::noncopyable
{
protected:
#ifdef _WIN32
	shareable_object_slot<HANDLE> mutex;
#else
	pthread_mutex_t mutex;
#endif


#ifdef _WIN32
	HANDLE create_mutex()
	{
		std::stringstream stm;

		auto uid = mutex.get_uid();

		stm << "Global\\" << uid[0] << "." << uid[1] << "." << uid[2] << "." << uid[3];

		HANDLE handle = CreateMutex(NULL, false, stm.str().c_str());	

		mutex = handle;

		return handle;
	}
#endif

public:
	robust_shareable_recursivemutex()
	{
#ifdef _WIN32
		create_mutex();
#else
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);
		pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);	
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);	
		pthread_mutex_init(&mutex, &attr);
		pthread_mutexattr_destroy(&attr);
#endif		
	}


	~robust_shareable_recursivemutex()
	{
#ifdef _WIN32
		CloseHandle(*mutex);
#else
		pthread_mutex_destroy(&mutex);
#endif		
	}

	void lock()
	{
#ifdef _WIN32
		HANDLE handle = *mutex;

		if (handle==NULL)
		{
			handle = create_mutex();		
		}

		WaitForSingleObject(handle, INFINITE);
#else
		if (pthread_mutex_lock(&mutex)==EOWNERDEAD)
			pthread_mutex_consistent(&mutex);
#endif
	}

	void unlock()
	{	
#ifdef _WIN32
		HANDLE handle = *mutex;

		if (handle==NULL)
		{
			handle = create_mutex();
		}

		ReleaseMutex(handle);
#else
		pthread_mutex_unlock(&mutex);
#endif
	}

	bool try_lock()
	{
#ifdef _WIN32
		HANDLE handle = *mutex;

		if (handle==NULL)
		{
			handle = create_mutex();
		} 

		DWORD wait = WaitForSingleObject(handle, INFINITE);

		return (wait == WAIT_OBJECT_0) || (wait==WAIT_ABANDONED);
#else
		int lockstate = pthread_mutex_trylock(&mutex);
		
		if (lockstate==EOWNERDEAD)
			pthread_mutex_consistent(&mutex);

		return (lockstate==0) || (lockstate==EOWNERDEAD);
#endif
	}
};

#endif

