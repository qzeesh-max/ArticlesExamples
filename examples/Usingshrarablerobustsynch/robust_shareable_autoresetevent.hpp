#ifndef __ROBUST_SHAREABLE_AUTORESETEVENT__H
#define __ROBUST_SHAREABLE_AUTORESETEVENT__H

#include <boost/utility.hpp>

#ifdef _WIN32

#include <winbase.h>
#include "shareable_object_slot.hpp"

#else

#include <sstream>
#include <pthread.h>

#endif

class robust_shareable_autoresetevent : boost::noncopyable
{
protected:
#ifdef _WIN32
	shareable_object_slot<HANDLE> event;
#else
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	volatile bool signalled;
#endif


#ifdef _WIN32
	HANDLE create_event()
	{
		std::stringstream stm;

		auto uid = event.get_uid();

		stm << "Global\\" << uid[0] << "." << uid[1] << "." << uid[2] << "." << uid[3];

		HANDLE handle = CreateEvent(NULL, false, false, stm.str().c_str());	

		event = handle;

		return handle;
	}
#endif

public:
	robust_shareable_autoresetevent()
	{
#ifdef _WIN32
		create_event();
#else
		signalled = false;
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);
		pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
		pthread_mutex_init(&mutex, &attr);
		pthread_mutexattr_destroy(&attr);

		pthread_condattr_t condattr;
		pthread_condattr_init(&condattr);
		pthread_condattr_setpshared(&condattr, PTHREAD_PROCESS_SHARED);
		pthread_cond_init(&cond, &condattr);
		pthread_condattr_destroy(&condattr);
#endif		
	}


	~robust_shareable_autoresetevent()
	{
#ifdef _WIN32
		CloseHandle(*event);
#else
		pthread_mutex_destroy(&mutex);
		pthread_cond_destroy(&cond);	
#endif		
	}

	void signal()
	{
#ifdef _WIN32
		HANDLE handle = *event;

		if (handle==NULL)
		{
			handle = create_event();		
		}

		SetEvent(handle);
#else
		signalled = true;
	
		pthread_cond_signal(&cond);
		
#endif
	}


	void wait()
	{
#ifdef _WIN32
		HANDLE handle = *event;

		if (handle==NULL)
		{
			handle = create_event();
		} 

		WaitForSingleObject(handle, INFINITE);

#else
		if ((pthread_mutex_lock(&mutex)==EOWNERDEAD))
			pthread_mutex_consistent(&mutex);

		if ((pthread_cond_wait(&cond, &mutex)==EOWNERDEAD))
			pthread_mutex_consistent(&mutex);

		pthread_mutex_unlock(&mutex);
#endif
	}
};

#endif

