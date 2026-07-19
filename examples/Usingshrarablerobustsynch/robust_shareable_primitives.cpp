#include "shareable_object_slot.hpp"
#include "robust_shareable_mutex.hpp"
#include "robust_shareable_recursivemutex.hpp"
#include "robust_shareable_autoresetevent.hpp"
#include <string>
#include <string.h>
#include <iostream>
#include <boost/interprocess/containers/deque.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>

using namespace boost::interprocess;

/*
	Provide a class that basically contains a family of mutexs to provide to the MemoryAlgorithm used by the basic_managed_shared_memory below.
*/
struct robust_shareable_mutex_family
{
public:
	typedef robust_shareable_mutex mutex_type;
	typedef robust_shareable_recursivemutex recursive_mutex_type;
};

/*
	Our specialization of basic_managed_shared_memory that is essentially the same except it uses our robust_shareable_mutex and robust_shareable_recursivemutex.
*/
typedef basic_managed_shared_memory
   <char
   ,rbtree_best_fit<robust_shareable_mutex_family>
   ,iset_index>
robust_managed_shared_memory;

/*
	Allocators and containers required for our example.
*/
typedef allocator<char, robust_managed_shared_memory::segment_manager> SharedCharAllocator;
typedef basic_string<char, std::char_traits<char>, SharedCharAllocator> SharedString;
typedef allocator<SharedString, robust_managed_shared_memory::segment_manager> SharedStringAllocator;
typedef deque<SharedString, SharedStringAllocator> SharedStringDeque;


/*
	This program expects one parameter which may either be server or client.

	server -- The program will listen for data on the queue. You can have multiple servers listening to the queues.
	client -- The program will post data on the queue. You can have multiple clients posting to the queues.

	Important: Ensure that you start the servers first, so the queue does not overflow.
*/
int main(int argc, char ** argv)
{
	bool server;

	if ((argc==2) && ((server=(strcmp(argv[1],"server")==0)) || (strcmp(argv[1],"client"))==0))
	{		
		robust_managed_shared_memory Shared(open_or_create, "myshare", 1024*4096);

		robust_shareable_autoresetevent* haveMessages = Shared.find_or_construct<robust_shareable_autoresetevent>("autoResetEvent")();
 
		robust_shareable_mutex * queueMutex = Shared.find_or_construct<robust_shareable_mutex>("queueMutex")();

		SharedCharAllocator charAllocator(Shared.get_segment_manager());
		SharedStringAllocator stringAllocator(Shared.get_segment_manager());

		SharedStringDeque * deque = Shared.find_or_construct<SharedStringDeque>("deque")(stringAllocator);

		
		if (server)
		{
			while (true)
			{
				haveMessages->wait();

				queueMutex->lock();
				while (!deque->empty())
				{
					std::cout << deque->front().c_str() << std::endl;

					deque->pop_front();					
				}
				queueMutex->unlock();
			}			
		} else {			
			for (int i = 0; ; i++)
			{
				std::stringstream stm;
	
				stm << "string:" << i;


				SharedString s(charAllocator);

				s = stm.str().c_str();

				queueMutex->lock();
				deque->push_back(s);

				if (i==5000)
					exit(1);

				queueMutex->unlock();

				haveMessages->signal();


			}
		}
	} else {
		std::cout << argv[0] << " [server|client]" << std::endl;
	}
	return 0;
}
