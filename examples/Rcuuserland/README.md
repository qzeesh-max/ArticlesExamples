# Implementing RCUs in Userland

*May 26^th, 2012 By Zeeshan Qazi*

**RCU** – ***R**ead **C**opy **U**pdate* is a mechanism for SMP synchronization that eliminates (or reduces) the need for spinning or blocking to gain access to a resource while allowing safe access to resources shared between multiple threads. The concept behind RCU is simple to explain in a few words:

- Readers read a pointer to the object from the slot holding the active pointer for the object. They do this while incrementing a counter (`RcuSectionHolderCount`) atomically, this is done to prevent the pointer being read from getting collected.

- Writers get a copy of the object that is presently in the slot by reading the object by becoming Readers only for the time that is needed to copy the object.

- Writer then modifies the copy of the object, and then updates the reference in the slot, while queuing the old reference for deletion.

- RCUs typically have a monitoring thread running and monitoring the old references queued for deletion. When sufficient time has passed between `RcuSectionHolderCount` going to zero and an item being queued it is collected.

Typically, RCUs are implemented in the Kernel in contexts where states of threads reading these RCU "protected" objects is known to the Kernel, so to say that when a context switch of such threads occurs while the `RcuSectionHolderCount` is zero, then all stale objects that were once read are immediately collected. Unfortunately, this luxury is largely unavailable in the User Mode, because we are not really able to tell when a thread switch occurs. But obviously, we have ways around the problem as we will see in the implementation shown in this article.

RCUs are not an ideal solution for complex objects like large maps or vectors, or objects that may need a lot of time to copy around. RCUs are typically intended to be used in contexts where there is a single writer, and multiple readers. Our implementation has a safe-guard in it to allow it to be used in contexts with multiple writers and multiple readers, though such a use case is practically a rarety. Our implementation's safe-guard for this use case does not add any additional penalty while the use case is not occurring. This is achived by using atomic compare and exchange intrinsics available to us from the compiler.

We are using `boost::thread` and other `boost` classes in this implementation to make this implementation cross platform. But our choice of intrinsics in this implementation are the ones available to us in GNU C++ compiler, but other compilers have their own variants of these intrinsic functions.

First let's list all the include files that are used in this implementation:

[   Download](javascript:DoLink('/download-sourcecode.php?Example=rcu-userland-ex1');)
   Compilation Instructions:   g++ -std=c++11 rcu-userland-ex1.cpp  -L /usr/lib64 -lboost_system -lboost_thread-mt -g3 -O3 -o rcu-userland-ex1

`
- #include &lt;time.h&gt;
- **2** `#include `
- **3** `#include `
- **4** `#include `
- **5** `#include `
- **6** `#include `
- **7** `#include `
- **8** `#include `
- **9** `#include `
- **10** `#include `
- **11** `#include `
- **12** ``
- **13** `using namespace boost;`
- **14** `using namespace std;`
- **15** ``
- **16** `// This number should be reasonably sized to the through-put requirement of the application`
- **17** `#define MAX_RCU 16384`
- **18** ``
- **19** `// This number should ideally be set to the number of threads using RCUs in the application`
- **20** `#define MAX_RCU_BUFFER_ZONE 256`
- **21** ``
- **22** ``
`

For our implementation to be able to correctly destroy an object instance once stored inside the RCU slot by the Monitor thread when such an object instance becomes stale, we need to have a wrapper to carry this reference on the deletion queue. This reference must be carried in a template class derived from a class with virtual destructor to allow the Monitor thread to be able to find the correct destructor to call when such an object instance is deleted.

`
- **23** `// This is base of the holder for the RCU class to allow collection of derived template class with`
- **24** `// proper invokation of the destructor`
- **25** `class IRcuCollectable`
- **26** `{`
- **27** `public:`
- **28** `virtual ~IRcuCollectable() { }`
- **29** `};`
- **30** ``
- **31** `// This is class is specialized to hold the object that is being used with the RCU`
- **32** `template`
- **33** `class RcuCollectable : public IRcuCollectable`
- **34** `{`
- **35** `protected:`
- **36** `T* Object;`
- **37** ``
- **38** `public:`
- **39** `RcuCollectable(T* object): Object(object)`
- **40** `{`
- **41** `}`
- **42** ``
- **43** `~RcuCollectable()`
- **44** `{`
- **45** `delete Object;`
- **46** `}`
- **47** `};`
- **48** ``
`

We now need to implement a structure for storing the pointer to this object on the deletion queue along with a sequence number used to determine when this object was actually added to the queue. This allows us to figure out if the object should be collected or not by the monitor thread. We will also define here the type used for our counter for determining the age of a reference queued for collection called `counter_t`.

`
- **49** `typedef unsigned long int counter_t;`
- **50** ``
- **51** `// This is a slot on the RCU Queue`
- **52** `struct RcuCollectableRef`
- **53** `{`
- **54** `volatile IRcuCollectable* Object;`
- **55** `volatile counter_t CycleCount;`
- **56** `};`
- **57** ``
`

`RcuCollectableRef` structure will be used inside the deletion queue that is maintained by the monitor class that monitors the deletion queue. The monitor class (`RcuManager`) is intended to be a singleton and basically fullfil the following obligations:

- Maintain an atomic counter that keeps track of how many readers are presently reading RCU objects.

- Maintain the deletion queue where overwritten references are copied to when a writer updates the reference.

- Maintain an atomic timer counter that keeps track of time, atomically so that we can tag object instances queued for deletion to determine their age.

- Collect all objects that are determined to be no longer referenced from and of `RcuReader` instances.

So let us implement this monitor class as a singleton to achieve precisely the objectives mentioned above:

`
- **58** ``
- **59** `// Forward reference for RcuReader`
- **60** `template`
- **62** ``
- **63** `// Forward reference for RcuSlot`
- **64** `template`
- **65** `class RcuSlot;`
- **66** ``
- **67** `class RcuManager`
- **68** `{`
- **69** `protected:`
- **70** `volatile int RcuSectionHolderCount;`
- **71** `volatile int RcuDeletionQueueBack;`
- **72** `volatile RcuCollectableRef* RcuDeletionQueue;`
- **73** `volatile counter_t RcuSectionCycleCount;`
- **74** `volatile counter_t RcuSectionZeroCycleCount;`
- **75** `volatile int MaxRcuDeletionQueueBack;`
- **76** `static RcuManager instance;`
- **77** `mutex ThreadExitedConditionMutex;`
- **78** `condition_variable ThreadExitedConditionVariable;`
- **79** `volatile bool   ExitThread;`
- **80** `volatile bool   ThreadExited;`
- **81** ``
- **82** ``
- **83** ``
- **84** `protected:`
- **85** ``
- **86** `// Initialize the object, and the RcuDeletionQueue.`
- **87** `RcuManager() : RcuSectionHolderCount(0),`
- **88** `RcuDeletionQueueBack(0),`
- **89** `// RcuSectionCycleCount and RcuSectionZeroCycleCount are both one because`
- **90** `// we use zero to mean value has not been placed yet on the Queue.`
- **91** `RcuSectionCycleCount(1),`
- **92** `RcuSectionZeroCycleCount(1),`
- **93** `ExitThread(false),`
- **94** `ThreadExited(false),`
- **95** `// MaxRcuDeletionQueueBack field is solely for diagnostic purposes`
- **96** `MaxRcuDeletionQueueBack(0)`
- **97** `{`
- **98** `RcuDeletionQueue = new RcuCollectableRef[MAX_RCU];`
- **99** ``
- **100** `// RcuDeletionQueue should be filled with zeroes to allow`
- **101** `// it to be usable sans locking`
- **102** `memset((void*)RcuDeletionQueue, 0, MAX_RCU*sizeof(RcuCollectableRef));`
- **103** ``
- **104** `// create a thread to monitor the deletion Queue to eliminate any objects that need`
- **105** `// to be deleted`
- **106** `thread(bind(type(), &RcuManager::MonitorThread, this));`
- **107** `}`
- **108** ``
- **109** `~RcuManager()`
- **110** `{`
- **111** `ExitThread = true;`
- **112** ``
- **113** `unique_lock lock(ThreadExitedConditionMutex);`
- **114** ``
- **115** `while (!ThreadExited)`
- **116** `ThreadExitedConditionVariable.wait(lock);`
- **117** ``
- **118** `delete[] RcuDeletionQueue;`
- **119** `}`
- **120** ``
- **121** `void MonitorThread()`
- **122** `{`
- **123** `counter_t RcuLastSectionZeroCycleCount = 0;`
- **124** ``
- **125** `while ((!ExitThread) || (RcuDeletionQueueBack!=0))`
- **126** `{`
- **127** `if (RcuLastSectionZeroCycleCount==0)`
- **128** `RcuLastSectionZeroCycleCount = RcuSectionZeroCycleCount;`
- **129** ``
- **130** `counter_t ZeroCycleCount = RcuSectionZeroCycleCount;`
- **131** `counter_t Difference = RcuSectionCycleCount - ZeroCycleCount;`
- **132** ``
- **133** `int OldRcuDeletionQueueBack = RcuDeletionQueueBack;`
- **134** ``
- **135** `if (RcuSectionCycleCount > ZeroCycleCount + 1)`
- **136** `{`
- **137** `int i = 0, upperI = 0;`
- **138** `bool FoundBad = false;`
- **139** ``
- **140** ``
- **141** ``
- **142** `for (; i Object==NULL) || (ref->CycleCount==0)) ;`
- **148** ``
- **149** `// If we have not yet encountered a possible out of order insertion yet,`
- **150** `// and last time RcuSectionHolderCount became zero is greater than 1 cycle after`
- **151** `// insertion of the record in the queue, or last time when we waited for the cycle`
- **152** `// count to become zero is more than 1 cycle after the insertion of the record in`
- **153** `// the queue, we can safely delete it, because it is extremely unlikely that the`
- **154** `// reference to that copy is held now, because we always atomically increment`
- **155** `// the RcuSectionHolderCount by one before copying a reference.`
- **156** `if ((!FoundBad) && ((RcuLastSectionZeroCycleCount > ref->CycleCount + 1) ||`
- **157** `(ZeroCycleCount > ref->CycleCount + 1) ))`
- **158** `{`
- **159** `delete ref->Object;`
- **160** ``
- **161** `upperI = i + 1;`
- **162** `} else {`
- **163** `// This is to ensure that when we have out of order insertions in RcuDeletionQueue,`
- **164** `// we do not end up deleting the wrong object.`
- **165** `FoundBad = true;`
- **166** `}`
- **167** `}`
- **168** ``
- **169** `int ActualCount;`
- **170** ``
- **171** `if (upperI!=0)`
- **172** `{`
- **173** `memmove((void*)RcuDeletionQueue, (void*)&RcuDeletionQueue[upperI],`
- **174** `(ActualCount = OldRcuDeletionQueueBack - upperI) * sizeof(RcuCollectableRef));`
- **175** ``
- **176** `memset((void*)&RcuDeletionQueue[ActualCount], 0,`
- **177** `(OldRcuDeletionQueueBack - ActualCount) * sizeof(RcuCollectableRef));`
- **178** ``
- **179** `int PostMoveRcuDeletionQueueBack;`
- **180** ``
- **181** `do {`
- **182** `PostMoveRcuDeletionQueueBack = RcuDeletionQueueBack;`
- **183** ``
- **184** `for (int j = OldRcuDeletionQueueBack; j Object==NULL) || (ref->CycleCount==0)) ;`
- **190** `}`
- **191** ``
- **192** `int MoveCount;`
- **193** ``
- **194** `memmove((void*)&RcuDeletionQueue[ActualCount],`
- **195** `(void*)&RcuDeletionQueue[OldRcuDeletionQueueBack],`
- **196** `(MoveCount = PostMoveRcuDeletionQueueBack - OldRcuDeletionQueueBack) * sizeof(RcuCollectableRef));`
- **197** ``
- **198** `memset((void*)&RcuDeletionQueue[ActualCount+MoveCount],`
- **199** `0,`
- **200** `(PostMoveRcuDeletionQueueBack - MoveCount - ActualCount) * sizeof(RcuCollectableRef));`
- **201** ``
- **202** `ActualCount += MoveCount;`
- **203** ``
- **204** `OldRcuDeletionQueueBack = PostMoveRcuDeletionQueueBack;`
- **205** ``
- **206** ``
- **207** `} while (!__sync_bool_compare_and_swap(&RcuDeletionQueueBack, PostMoveRcuDeletionQueueBack, ActualCount));`
- **208** `}`
- **209** ``
- **210** `}`
- **211** ``
- **212** ``
- **213** `if (RcuSectionHolderCount==0)`
- **214** `RcuLastSectionZeroCycleCount = RcuSectionCycleCount;`
- **215** ``
- **216** ``
- **217** ``
- **218** `RcuSectionCycleCount++;`
- **219** ``
- **220** `if ((MAX_RCU - RcuDeletionQueueBack) >  MAX_RCU_BUFFER_ZONE)`
- **221** `this_thread::sleep(posix_time::milliseconds(50));`
- **222** `else`
- **223** `this_thread::sleep(posix_time::milliseconds(0));`
- **224** ``
- **225** `}`
- **226** ``
- **227** `unique_lock lock(ThreadExitedConditionMutex);`
- **228** ``
- **229** `ThreadExited = true;`
- **230** `ThreadExitedConditionVariable.notify_one();`
- **231** `}`
- **232** ``
- **233** `public:`
- **234** `static RcuManager& Instance()`
- **235** `{`
- **236** `return instance;`
- **237** `}`
- **238** ``
- **239** `int GetMaxRcuDeletionQueueBack() const`
- **240** `{`
- **241** `return MaxRcuDeletionQueueBack;`
- **242** `}`
- **243** ``
- **244** `protected:`
- **245** `template `
- **246** `friend class RcuReader;`
- **247** ``
- **248** `template `
- **249** `friend class RcuSlot;`
- **250** ``
- **251** `// This function must not be directly called, it is called by the RcuReader class`
- **252** `int LockReadingSection()`
- **253** `{`
- **254** `// wait for the RcuDeletionQueueBack to have enough room, if we are too close to the`
- **255** `// buffer zone, to prevent deadlocking on very high throughput`
- **256** `while ((MAX_RCU - RcuDeletionQueueBack)  class`
- **262** `int UnlockReadingSection()`
- **263** `{`
- **264** `counter_t DeletionQueuedCounter = RcuSectionCycleCount;`
- **265** `int SectionHolderCount =  __sync_sub_and_fetch(&RcuSectionHolderCount, 1);`
- **266** ``
- **267** `// Note down the Cycle when the SectionHolder Count is zero.`
- **268** `if (SectionHolderCount == 0)`
- **269** `RcuSectionZeroCycleCount = DeletionQueuedCounter;`
- **270** ``
- **271** `return SectionHolderCount;`
- **272** `}`
- **273** ``
- **274** `// This function must not be directly called, it is called by the RcuReader class`
- **275** `void QueueForDeletion(IRcuCollectable* p)`
- **276** `{`
- **277** `// wait for the RcuDeletionQueueBack to have enough room, if we are too close to the`
- **278** `// buffer zone, to prevent overflowing the Queue`
- **279** `while ((MAX_RCU - RcuDeletionQueueBack) `
- **297** `class RcuReader`
- **298** `{`
- **299** `protected:`
- **300** `const T* Object;`
- **301** ``
- **302** `protected:`
- **303** `RcuReader(volatile T* const* object)`
- **304** `{`
- **305** `RcuManager::Instance().LockReadingSection();`
- **306** ``
- **307** `Object = (const T*)*object;`
- **308** `}`
- **309** ``
- **310** `public:`
- **311** `RcuReader(const RcuReader& other)`
- **312** `{`
- **313** `RcuManager::Instance().LockReadingSection();`
- **314** ``
- **315** `Object = other.Object;`
- **316** `}`
- **317** ``
- **318** `~RcuReader()`
- **319** `{`
- **320** `RcuManager::Instance().UnlockReadingSection();`
- **321** `}`
- **322** ``
- **323** `const T* operator->()`
- **324** `{`
- **325** `return Object;`
- **326** `}`
- **327** ``
- **328** `bool operator==(const T* object)`
- **329** `{`
- **330** `return Object==object;`
- **331** `}`
- **332** ``
- **333** `bool operator!=(const T* object)`
- **334** `{`
- **335** `return Object!=object;`
- **336** `}`
- **337** ``
- **338** `const T& operator*()`
- **339** `{`
- **340** `return (const T&)*Object;`
- **341** `}`
- **342** ``
- **343** `friend class RcuSlot;`
- **344** `};`
- **345** ``
- **346** `template`
- **347** `class RcuSlot`
- **348** `{`
- **349** `protected:`
- **350** `volatile T* Object;`
- **351** ``
- **352** ``
- **353** `public:`
- **354** `RcuSlot() : Object(NULL)`
- **355** `{`
- **356** `}`
- **357** ``
- **358** ``
- **359** `~RcuSlot()`
- **360** `{`
- **361** `UpdateReference(NULL);`
- **362** `}`
- **363** ``
- **364** `void UpdateReference(T* newObject)`
- **365** `{`
- **366** `T* PreviousObject = (T*)Object;`
- **367** ``
- **368** `// This code will only spin if the reference RcuSlot is`
- **369** `// being changed by multiple threads`
- **370** `while   (!__sync_bool_compare_and_swap(&Object, PreviousObject, newObject))`
- **371** `{`
- **372** `PreviousObject = (T*)Object;`
- **373** `}`
- **374** ``
- **375** `if (PreviousObject!=NULL)`
- **376** `RcuManager::Instance().QueueForDeletion(new RcuCollectable(PreviousObject));`
- **377** ``
- **378** `}`
- **379** ``
- **380** `RcuReader Read()`
- **381** `{`
- **382** `return RcuReader(&Object);`
- **383** `}`
- **384** ``
- **385** `T* Copy()`
- **386** `{`
- **387** `RcuReader Reader(&Object);`
- **388** ``
- **389** `T* p = new T(*Reader);`
- **390** ``
- **391** `return p;`
- **392** `}`
- **393** ``
- **394** ``
- **395** `};`
`

We have now implemented the RCU mechanism for the Userland. As you can see that it is not all that complicated after all. Here are a few things you need to remember while using this mechanism:

- RCUs are intended mostly for having no more than one thread updating the object reference at a time. Though our implementation supports updating object references from more than one thread, there is practically no reason for doing that through RCUs as two overlapping updates done through this mechanism will not be combined and one of them will be lost in an unpredictable manner.

- None of the references to copy of the object should be saved beyond the lifetime of the `RcuReader` object. The `RcuReader` should only be obtained on the stack and should not be kept alive for a prolonged period of time. Copies of the objects can be obtained to be kept for longer periods by a call to `RcuSlot::Copy` method.

- If you intend to update an object based on what is already stored in the object itself, then you may obtain copy of the object to modify by calling the `RcuSlot::Copy` method. Once you have modified the copy you can update the reference in the slot to the pointer with the reference to the modified object by a call to `RcuSlot::UpdateReference`.

- `MAX_RCU` and `MAX_RCU_BUFFER_ZONE` macros define parameters that directly affect the performance of this RCU library. `MAX_RCU` denotes the maximum count of object instances in the deletion queue, where as `MAX_RCU_BUFFER_ZONE` denotes the size of the buffer zone, typically intended to be about the same as the count of threads expected to utilize the RCU synchronized objects. If an application needs very high rates of turnover of objects stored in RCU slots, then the `RCU_MAX` has to be larger to support this facility. There is need for experimentation to understand the needs of a specific application. Once the buffer zone defined by the `RCU_MAX_BUFFER_ZONE` is breached then spinning is needed while queuing objects for deletion while other objects are destroyed and removed from the queue. This implementation of RCUs will typically never spin on threads utilizing the RCUs unless the deletion queue is inadequate for the RCU turnover, or multiple writers (as in the threads doing the writing) are calling `RcuSlot::UpdateReference` on the same `RcuSlot` instance simultaneously.

- Though we have not suppressed the copy constructor in our code above for `RcuSlot`, this class should neither be copy constructed, nor should it ever be assigned to directly (that is without using the methods provided).

I hope that our walkthrough of the implementation of RCUs in this tutorial has been enlightening to you. Due to time constraints and wide series of use cases possible, unfortunately we would not be able to work on an example on using the above implementation. I hope that information provided above is sufficient in facilitating implementation of some test programs to do the same, however the **Download** button above can be used to obtain a simple example.
