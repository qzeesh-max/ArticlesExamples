#include <time.h>
#include <string.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <math.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <sstream>
#include <unistd.h>

using namespace std;

// This number should be reasonably sized to the through-put requirement of the application
#define MAX_RCU 16384

// This number should ideally be set to the number of threads using RCUs in the application
#define MAX_RCU_BUFFER_ZONE 256


// This is base of the holder for the RCU class to allow collection of derived template class with
// proper invokation of the destructor
class IRcuCollectable
{
    public:
        virtual ~IRcuCollectable() { }
};

// This is class is specialized to hold the object that is being used with the RCU
template<typename T>
class RcuCollectable : public IRcuCollectable
{
    protected:
        T* Object;
        
    public:
        RcuCollectable(T* object): Object(object)
        {
        }
        
        ~RcuCollectable()
        {
            delete Object;
        }
};

typedef unsigned long int counter_t;

// This is a slot on the RCU Queue
struct RcuCollectableRef
{
    volatile IRcuCollectable* Object;
    volatile counter_t CycleCount;
};

// Forward reference for RcuReader<T>
template<typename T>
class RcuReader;

// Forward reference for RcuSlot<T>
template<typename T>
class RcuSlot;

class RcuManager
{
    protected:
        volatile int RcuSectionHolderCount;
        volatile int RcuDeletionQueueBack;
        volatile RcuCollectableRef* RcuDeletionQueue;
        volatile counter_t RcuSectionCycleCount;
        volatile counter_t RcuSectionZeroCycleCount;
        volatile int MaxRcuDeletionQueueBack;
        static RcuManager instance;
        mutex ThreadExitedConditionMutex;
        condition_variable ThreadExitedConditionVariable;
        volatile bool   ExitThread;
        volatile bool   ThreadExited;
        


    protected:
    
        // Initialize the object, and the RcuDeletionQueue. 
        RcuManager() : RcuSectionHolderCount(0),
                   RcuDeletionQueueBack(0),
                   // RcuSectionCycleCount and RcuSectionZeroCycleCount are both one because 
                   // we use zero to mean value has not been placed yet on the Queue.
                   RcuSectionCycleCount(1),
                   RcuSectionZeroCycleCount(1),
                   ExitThread(false),
                   ThreadExited(false),
                   // MaxRcuDeletionQueueBack field is solely for diagnostic purposes
                   MaxRcuDeletionQueueBack(0)
        {
            RcuDeletionQueue = new RcuCollectableRef[MAX_RCU];

            // RcuDeletionQueue should be filled with zeroes to allow
            // it to be usable sans locking
            memset((void*)RcuDeletionQueue, 0, MAX_RCU*sizeof(RcuCollectableRef));
            
            // create a thread to monitor the deletion Queue to eliminate any objects that need
            // to be deleted
            thread(&RcuManager::MonitorThread, this).detach();
        }

        ~RcuManager()
        {       
            ExitThread = true;
            
            unique_lock<mutex> lock(ThreadExitedConditionMutex);
            
            while (!ThreadExited)
                ThreadExitedConditionVariable.wait(lock);
                
            delete[] RcuDeletionQueue;
        }
        
        void MonitorThread()
        {
            counter_t RcuLastSectionZeroCycleCount = 0;
            
            while ((!ExitThread) || (RcuDeletionQueueBack!=0))
            {
                if (RcuLastSectionZeroCycleCount==0)
                    RcuLastSectionZeroCycleCount = RcuSectionZeroCycleCount;
                    
                counter_t ZeroCycleCount = RcuSectionZeroCycleCount;
                counter_t Difference = RcuSectionCycleCount - ZeroCycleCount;
                
                int OldRcuDeletionQueueBack = RcuDeletionQueueBack;
                
                if (RcuSectionCycleCount > ZeroCycleCount + 1)
                {                   
                    int i = 0, upperI = 0;
                    bool FoundBad = false;
                    

                
                    for (; i < OldRcuDeletionQueueBack; i++)
                    {
                        RcuCollectableRef* ref = (RcuCollectableRef*)&RcuDeletionQueue[i];
                    
                        // Spin until we get the object in the queue
                        while ((ref->Object==NULL) || (ref->CycleCount==0)) ;
                    
                        // If we have not yet encountered a possible out of order insertion yet,
                        // and last time RcuSectionHolderCount became zero is greater than 1 cycle after 
                        // insertion of the record in the queue, or last time when we waited for the cycle 
                        // count to become zero is more than 1 cycle after the insertion of the record in
                        // the queue, we can safely delete it, because it is extremely unlikely that the 
                        // reference to that copy is held now, because we always atomically increment
                        // the RcuSectionHolderCount by one before copying a reference.
                        if ((!FoundBad) && ((RcuLastSectionZeroCycleCount > ref->CycleCount + 1) ||
                                    (ZeroCycleCount > ref->CycleCount + 1) ))
                        {                           
                            delete ref->Object;
                            
                            upperI = i + 1;
                        } else {
                            // This is to ensure that when we have out of order insertions in RcuDeletionQueue, 
                            // we do not end up deleting the wrong object.
                            FoundBad = true;
                        }
                    }
                
                    int ActualCount;
                
                    if (upperI!=0)
                    {
                        memmove((void*)RcuDeletionQueue, (void*)&RcuDeletionQueue[upperI], 
                            (ActualCount = OldRcuDeletionQueueBack - upperI) * sizeof(RcuCollectableRef));
                    
                        memset((void*)&RcuDeletionQueue[ActualCount], 0,
                            (OldRcuDeletionQueueBack - ActualCount) * sizeof(RcuCollectableRef));
                
                        int PostMoveRcuDeletionQueueBack;
                
                        do {
                            PostMoveRcuDeletionQueueBack = RcuDeletionQueueBack;
                
                            for (int j = OldRcuDeletionQueueBack; j < PostMoveRcuDeletionQueueBack; j++)
                            {
                                volatile RcuCollectableRef* ref = &RcuDeletionQueue[j];
                        
                                // Spin until we get the object in the queue
                                while ((ref->Object==NULL) || (ref->CycleCount==0)) ;
                            }
                
                            int MoveCount;
                
                            memmove((void*)&RcuDeletionQueue[ActualCount], 
                                (void*)&RcuDeletionQueue[OldRcuDeletionQueueBack], 
                                (MoveCount = PostMoveRcuDeletionQueueBack - OldRcuDeletionQueueBack) * sizeof(RcuCollectableRef));
                    
                            memset((void*)&RcuDeletionQueue[ActualCount+MoveCount], 
                                   0, 
                                   (PostMoveRcuDeletionQueueBack - MoveCount - ActualCount) * sizeof(RcuCollectableRef));
                
                            ActualCount += MoveCount;
                
                            OldRcuDeletionQueueBack = PostMoveRcuDeletionQueueBack;

                
                        } while (!__sync_bool_compare_and_swap(&RcuDeletionQueueBack, PostMoveRcuDeletionQueueBack, ActualCount));              
                    }
                
                } 
                
                
                if (RcuSectionHolderCount==0)
                    RcuLastSectionZeroCycleCount = RcuSectionCycleCount;
                
                
                    
                RcuSectionCycleCount++;
                
                if ((MAX_RCU - RcuDeletionQueueBack) >  MAX_RCU_BUFFER_ZONE)
                    this_thread::sleep_for(chrono::milliseconds(50));
                else
                    this_thread::sleep_for(chrono::milliseconds(0));
                
            } 
            
            unique_lock<mutex> lock(ThreadExitedConditionMutex);
            
            ThreadExited = true;
            ThreadExitedConditionVariable.notify_one();
        }

    public:
        static RcuManager& Instance()
        {
            return instance;
        }
        
        int GetMaxRcuDeletionQueueBack() const
        {
            return MaxRcuDeletionQueueBack;
        }
        
    protected:
        template <typename T>
        friend class RcuReader;
        
        template <typename T>
        friend class RcuSlot;

        // This function must not be directly called, it is called by the RcuReader<T> class
        int LockReadingSection()
        {
	    // wait for the RcuDeletionQueueBack to have enough room, if we are too close to the
	    // buffer zone, to prevent deadlocking on very high throughput
 	    while ((MAX_RCU - RcuDeletionQueueBack) <=  MAX_RCU_BUFFER_ZONE) ;
	    
            return __sync_add_and_fetch(&RcuSectionHolderCount, 1);
        }

        // This function must not be directly called, it is called by the RcuReader<T> class
        int UnlockReadingSection()
        {
            counter_t DeletionQueuedCounter = RcuSectionCycleCount;
            int SectionHolderCount =  __sync_sub_and_fetch(&RcuSectionHolderCount, 1);
            
            // Note down the Cycle when the SectionHolder Count is zero.
            if (SectionHolderCount == 0)
                RcuSectionZeroCycleCount = DeletionQueuedCounter;   
                
            return SectionHolderCount;
        }

        // This function must not be directly called, it is called by the RcuReader<T> class
        void QueueForDeletion(IRcuCollectable* p)
        {
            // wait for the RcuDeletionQueueBack to have enough room, if we are too close to the
            // buffer zone, to prevent overflowing the Queue
            while ((MAX_RCU - RcuDeletionQueueBack) <=  MAX_RCU_BUFFER_ZONE) ;
            
            int RcuDeletionIndex = __sync_fetch_and_add(&RcuDeletionQueueBack, 1);

            RcuDeletionQueue[RcuDeletionIndex].Object = p;          
            RcuDeletionQueue[RcuDeletionIndex].CycleCount = RcuSectionCycleCount;
            
            MaxRcuDeletionQueueBack = std::max(MaxRcuDeletionQueueBack, RcuDeletionQueueBack);
        }
        

};

RcuManager RcuManager::instance;



template<typename T>
class RcuReader
{
    protected:
        const T* Object;
        
    protected:
        RcuReader(volatile T* const* object)
        {
            RcuManager::Instance().LockReadingSection();

            Object = (const T*)*object;
        }
        
    public:
        RcuReader(const RcuReader& other) 
        {
            RcuManager::Instance().LockReadingSection();

            Object = other.Object;
        }
        
        ~RcuReader()
        {
            RcuManager::Instance().UnlockReadingSection();
        }
        
        const T* operator->()
        {
            return Object;
        }
        
        bool operator==(const T* object)
        {
            return Object==object;
        }
        
        bool operator!=(const T* object)
        {
            return Object!=object;
        }
        
        const T& operator*()
        {
            return (const T&)*Object;
        }
        
        friend class RcuSlot<T>;
};

template<typename T>
class RcuSlot
{
    protected:
        volatile T* Object;


    public:
        RcuSlot() : Object(NULL)
        {
        }
        

        ~RcuSlot() 
        {
            UpdateReference(NULL);
        }

        void UpdateReference(T* newObject)
        {
            T* PreviousObject = (T*)Object;

            // This code will only spin if the reference RcuSlot is
            // being changed by multiple threads
            while   (!__sync_bool_compare_and_swap(&Object, PreviousObject, newObject))
            {
                PreviousObject = (T*)Object;                
            }

            if (PreviousObject!=NULL)
                RcuManager::Instance().QueueForDeletion(new RcuCollectable<T>(PreviousObject));

        }
        
        RcuReader<T> Read()
        {
            return RcuReader<T>(&Object);
        }

        T* Copy()
        {
            RcuReader<T> Reader(&Object);
            
            T* p = new T(*Reader);
            
            return p;
        }

        
};

RcuSlot<string> TestSlot[4];
bool        ThreadsDone = false;

void PlayingThread(int l)
{

    for (int i = 0; i < 10000; i++)
    {
        stringstream stm;
        
        stm << "Thread " << l <<  "-" << i << endl;
        
        TestSlot[l].UpdateReference(new string(stm.str()));

    }

}

void PrintingThread()
{
    int j = 0;
    
    while (!ThreadsDone)
    {
        for (int i = 0; i < 4; i++, j++)
		{
            printf("%s\n", TestSlot[i].Read()->c_str());

		    if ((j % 1024)==0)
			usleep(10000);
		}
    }
}

int main(void)
{
    for (int i = 0; i < 4; i++)
        TestSlot[i].UpdateReference(new string("Hello"));
    
    thread pool[4], printthread[4];

	    for (int i = 0; i < 4; i++)
		printthread[i] = thread(PrintingThread);
    
    for (int i = 0; i < 4; i++)
    {
        pool[i] = thread(PlayingThread, i);
    }       
    
    for (int i = 0; i < 4; i++)
    {
        pool[i].join();             
    }   
    
    ThreadsDone = true;
    
    for (int i = 0; i < 4; i++)
		    printthread[i].join();
    

    cout << "Maximum Rcu Deletion QueueBack Reached:" << RcuManager::Instance().GetMaxRcuDeletionQueueBack() << endl;
        
    return 0;
}


