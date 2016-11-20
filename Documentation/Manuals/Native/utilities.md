Utilities									{#utilities}
===============
[TOC]

This manual will quickly go over all the important utility systems commonly used by Banshee. We won't go into major detail about these features, but will rather point you towards the relevant API documentation.

# Module {#utilities_a}
A @ref bs::Module<T> "Module<T>" is a specialized form of singleton used for many of Banshee's systems. Unlike normal singletons it requires manual startup and shutdown, which solves many of the problems associated with traditional singletons.

To use it for your own objects, simply inherit from it:
~~~~~~~~~~~~~{.cpp}
class MyModule : public Module<MyModule>
{ };
~~~~~~~~~~~~~

Use @ref bs::Module<T>::startUp "Module<T>::startUp" to start it up. Once started use @ref bs::Module<T>::instance "Module<T>::instance" to access its instance. Once done with it call @ref bs::Module<T>::shutDown "Module<T>::shutDown" to release it. For example:
~~~~~~~~~~~~~{.cpp}
MyModule::startUp();
MyModule::instance().doSomething();
MyModule::shutDown();
~~~~~~~~~~~~~

# Path {#utilities_b}
Use @ref bs::Path "Path" to manipulate file/folder paths. Initialize it with a path string and then call various methods to manipulate it. It is recommended to always store paths using @ref bs::Path "Path" instead of raw strings.

Some of the things you can do once a @ref bs::Path "Path" is constructed:
 - Retrieve the filename using @ref bs::Path::getFilename "Path::getFilename"
 - Retrieve the filename extension using @ref bs::Path::getExtension "Path::getExtension"
 - Get last element of path, either file or directory using @ref bs::Path::getTail "Path::getTail"
 - Iterate over directories, get drive, combine paths, convert relative to absolute paths and vice versa, and more...
 
For example:
~~~~~~~~~~~~~{.cpp}
Path myPath("C:\\Path\\To\\File.txt");

String filename = myPath.getFilename(); // Returns file
myPath.setExtension(".jpg"); // Path is now "C:\Path\To\File.jpg"
myPath.makeRelative("C:\\Path"); // Path is now "To\File.jpg"

Path a("C:\\Path\\To\\");
Path b("File.txt");
Path combined = a + b; // // Path is now "C:\Path\To\File.txt"
~~~~~~~~~~~~~
 
All @ref bs::Path "Path" methods that return strings come in two variants, one that returns a narrow (8-bit) character string like @ref bs::Path::getFilename "Path::getFilename", and one that contains wide character string like @ref bs::Path::getWFilename "Path::getWFilename".

When setting paths be careful with setting backslashes or slashes at the end of the path. Path with a no backslash/slash on the end will be interpreted as a file path, and path with a backslash/slash will be interpreted as a folder path. For example:
 - "C:\MyFolder" - "MyFolder" interpreted as a file, @ref bs::Path::getFilename "Path::getFilename" returns "MyFolder"
 - "C:\MyFolder\" - "MyFolder" interpreted as a folder, @ref bs::Path::getFilename "Path::getFilename" returns an empty string
 
# File system {#utilities_c}
The @ref bs::FileSystem "FileSystem" module allows you to open and create files, create folders, move/copy/remove files and folders, iterate all folder/files in a folder, get file size, check if folder/folder exists, get working path and others.

An example creating a folder and a file:
~~~~~~~~~~~~~{.cpp}
FileSystem::createDir("C:\\Path\\To\\");
SPtr<DataStream> fileStream = FileSystem::createAndOpenFile("C:\\Path\\To\\File.txt");
... write to data stream...
~~~~~~~~~~~~~
# Data streams {#utilities_d}
@ref bs::DataStream "Data streams" allow you to easily write/read binary/text data from/to disk/memory/etc. The two primary types of streams are @ref bs::MemoryDataStream "MemoryDataStream" for reading/writing directly to memory, and @ref bs::FileDataStream "FileDataStream" for reading/writing to a file.

You create memory streams by providing them with a pointer and size of a memory buffer, while you create file streams by calling @ref bs::FileSystem::openFile "FileSystem::openFile" or @ref bs::FileSystem::createAndOpenFile "FileSystem::createAndOpenFile". Once you are done with a stream make sure to close it by calling @ref bs::DataStream::close "DataStream::close". Stream will also be automatically closed when it goes out of scope.

Once you have a stream you can seek to a position within a stream and read/write to it. For example:
~~~~~~~~~~~~~{.cpp}
SPtr<DataStream> fileStream = FileSystem::createAndOpenFile("C:\\Path\\To\\File.txt");
// Write some string data
fileStream.writeString("Writing to a file");
// Write some binary data
UINT8* myBuffer = bs_alloc(1024);
... fill up the buffer with some data ...
fileStream.write(myBuffer, 1024);

fileStream.close();
~~~~~~~~~~~~~
 
# Events {#utilities_e}
@ref bs::TEvent<RetType, Args> "Events" allow your objects to expose events that may trigger during execution. External objects interested in those events can then register callbacks with those events and be notified when they happen. They are useful because they allow two objects to communicate without necessarily knowing about each other's types, which can reduce class coupling and improve design.

When creating an event, all you need to do it specify a format of the callback it sends out, for example:
~~~~~~~~~~~~~{.cpp}
class MySystem
{
	static Event<void()> myEvent; // Callback with no parameters
	static Event<void(UINT32)> myEvent2; // Callback with a UINT32 parameter
};
~~~~~~~~~~~~~

Then an external object can register itself with an event by calling @ref bs::TEvent<RetType, Args> "Event::connect". This method will return an @ref bs::HEvent "HEvent" handle. You can use this handle to manually disconnect from the event by calling @ref bs::HEvent::disconnect "HEvent::disconnect". For example:
~~~~~~~~~~~~~{.cpp}
// Subscribe to an event we defined previously
// Simply pass a function pointer matching the callback
HEvent eventHandle = MySystem::myEvent2.connect(&myEventReceiver);

void myEventReceiver(UINT32 val)
{
	// Do something
}
~~~~~~~~~~~~~

When using non-static class methods as callbacks, things get a little bit more complicated. This is because each such method by default expects a pointer to an instance of itself (`this` pointer). This is normally hidden from the programmer and happens under the hood, but we must handle it when dealing with callbacks. We can do this by using `std::bind` which allows us to replace function arguments with constant values. For example:
~~~~~~~~~~~~~{.cpp}
class EventReceiver
{
	EventReceiver()
	{
		// Convert a method with signature void(EventReceiver*, UINT32) into void(UINT32) by binding the "EventReceiver*"
		// argument to the value of "this". Read up on the C++ library for more information about std::bind.
		MySystem::myEvent2.connect(std::bind(&EventReceiver::myEventReceiver, this, std::placeholders::_1));
	}
	
	void myEventReceiver(UINT32 val)
	{
		// Do something
	}
};
~~~~~~~~~~~~~

Then when an object is ready to trigger an event simply call it like a functor:
~~~~~~~~~~~~~{.cpp}
MySystem::myEvent(); // Trigger an event with no arguments
MySystem::myEvent2(5); // Trigger an event with a single argument
~~~~~~~~~~~~~

# Any {#utilities_f}
Use the @ref bs::Any "Any" type to easily store any kind of object in it. For example:
~~~~~~~~~~~~~{.cpp}
Any var1 = Vector<String>();

struct MyStruct { int a; };
Any var2 = MyStruct();
~~~~~~~~~~~~~

Use @ref bs::any_cast "any_cast" and @ref bs::any_cast_ref "any_cast_ref" to retrieve valid types from an @ref bs::Any "Any" variable. For example:
~~~~~~~~~~~~~{.cpp}
Vector<String> val1 = any_cast<Vector<String>>(var1);
MyStruct& val2 = any_cast_ref<MyStruct>(var2);
~~~~~~~~~~~~~
# Flags {#utilities_g}
@ref bs::Flags<Enum, Storage> "Flags" provide a wrapper around an `enum` and allow you to easily perform bitwise operations on them without having to cast to integers. For example when using raw C++ you must do something like this:
~~~~~~~~~~~~~{.cpp}
enum class MyFlag
{
	Flag1 = 1<<0,
	Flag2 = 1<<1,
	Flag3 = 1<<2
};

MyFlag combined = (MyFlag)((UINT32)MyFlag::Flag1 | (UINT32)MyFlag::Flag2);
~~~~~~~~~~~~~

Which is cumbersome. Flags require an additional step to define the enum, but after that allow you to manipulate values much more nicely. 

To create @ref bs::Flags<Enum, Storage> "Flags" for an enum simply define a `typedef` with your enum type provided as the template parameter. You must also follow that definition with a @ref BS_FLAGS_OPERATORS macro in order to ensure all operators are properly defined. For example:
~~~~~~~~~~~~~{.cpp}
typedef Flags<MyFlag> MyFlags;
BS_FLAGS_OPERATORS(MyFlag)
~~~~~~~~~~~~~

Now you can do something like this:
~~~~~~~~~~~~~{.cpp}
MyFlags combined = MyFlag::Flag1 | MyFlag::Flag2;
~~~~~~~~~~~~~
# String {#utilities_h}
Banshee uses @ref bs::String "String" for narrow character strings (8-bit) and @ref bs::WString "WString" for wide character strings. Wide character strings are different size depending on platform.

A variety of string manipulation functionality is provided in @ref bs::StringUtil "StringUtil", like matching, replacing, comparing, formating and similar.

Conversion between various types (like int, float, bool, etc.) and string is provided via overloads of @ref bs::toString "toString" and @ref bs::toWString "toWString". You can also convert strings into different types by calling methods like @ref bs::parseINT32 "parseINT32", @ref bs::parseBool "parseBool", and similar for other types.

# Threading {#utilities_i}
## Primitives {#utilities_i_a}
This section describes the most basic primitives you can use to manipulate threads. All threading primitives use the standard C++ library constructs, so for more information you should read their documentation.

### Thread {#utilities_i_a_a}
To create a new thread use @ref bs::Thread "Thread", like so:
~~~~~~~~~~~~~{.cpp}
void workerFunc()
{
	// This runs on another thread
}

Thread myThread(&workerFunc);
~~~~~~~~~~~~~

### Mutex {#utilities_i_a_b}
Use @ref bs::Mutex "Mutex" and @ref bs::Lock "Lock" to synchronize access between multiple threads, like so:
~~~~~~~~~~~~~{.cpp}
Vector<int> output;
int startIdx = 0;
Mutex mutex;

void workerFunc()
{
	// Lock the mutex before modifying either "output" or "startIdx"
	// This ensures only one thread every accesses it at once
	Lock lock(mutex);
	output.push_back(startIdx++);
}

// Start two threads that write to "output"
Thread threadA(&workerFunc);
Thread threadB(&workerFunc);
~~~~~~~~~~~~~

If a mutex can be locked recursively, use @ref bs::RecursiveMutex "RecursiveMutex" and @ref bs::RecursiveLock "RecursiveLock" instead.

### Signal {#utilities_i_a_c}
Use @ref bs::Signal "Signal" to pause thread execution until another thread reaches a certain point. For example:
~~~~~~~~~~~~~{.cpp}
bool isReady = false;
int result = 0;

Signal signal;
Mutex mutex;

void workerFunc()
{
	for(int i = 0; i < 100000; i++)
		result += i; // Or some more complex calculation
	
	// Lock the mutex so we can safely modify isReady
	{
		Lock lock(mutex);
		isReady = true;		
	} // Automatically unlocked when lock goes out of scope
	
	// Notify everyone waiting that the signal is ready
	signal.notify_all();
}

// Start executing workerFunc
Thread myThread(&workerFunc);

// Wait until the signal is triggered, or until isReady is set to true, whichever comes first
Lock lock(mutex);
if(!isReady)
	signal.wait_for(lock);
~~~~~~~~~~~~~

### Other {#utilities_i_a_d}
The previous sections covered all the primitives, but there is some more useful functionality to be aware of:
 - @ref BS_THREAD_HARDWARE_CONCURRENCY - Returns number of logical CPU cores.
 - @ref BS_THREAD_CURRENT_ID - Returns @ref bs::ThreadId "ThreadId" of the current thread.
 - @ref BS_THREAD_SLEEP - Pauses the current thread for a set number of milliseconds.

## Thread pool {#utilities_i_b}
Instead of using @ref bs::Thread "Thread" as described in the previous section, you should instead use @ref bs::ThreadPool "ThreadPool" for running threads. @ref bs::ThreadPool "ThreadPool" allows you to re-use threads and avoid paying the cost of thread creation and destruction. It keeps any thread that was retired in idle state, and will re-use it when user requests a new thread.

An example:
~~~~~~~~~~~~~{.cpp}
void workerFunc()
{
	// This runs on another thread
}

ThreadPool::instance().run("MyThread", &workerFunc);
~~~~~~~~~~~~~

## Task scheduler {#utilities_i_c}
@ref bs::TaskScheduler "TaskScheduler" allows even more fine grained control over threads. It ensures there are only as many threads as the number of logical CPU cores. This ensures good thread distribution accross the cores, so that multiple threads don't fight for resources on the same core.

It accomplishes that by storing each worker function as a @ref bs::Task "Task". It then dispatches tasks to threads that are free. In case tasks are dependant on one another you may also provide task dependencies, as well as task priorities.

An example:
~~~~~~~~~~~~~{.cpp}
void workerFunc()
{
	// This runs on another thread
}

// Create a task with no dependency and normal priority
SPtr<Task> task = Task::create("MyTask", &workerFunc);
TaskScheduler::instance().addTask(task);
~~~~~~~~~~~~~

# Math {#utilities_j}
Majority of the math related functionality is located in the @ref bs::Math "Math" class. 

Some other useful math classes are:
 - @ref bs::Vector2 "Vector2"
 - @ref bs::Vector3 "Vector3"
 - @ref bs::Vector4 "Vector4"
 - @ref bs::Matrix3 "Matrix3"
 - @ref bs::Matrix4 "Matrix4"
 - @ref bs::Quaternion "Quaternion"
 - @ref bs::Radian "Radian"
 - @ref bs::Degree "Degree"
 - @ref bs::Ray "Ray"
 - @ref bs::Plane "Plane"
 - @ref bs::Rect2 "Rect2"
 - @ref bs::Rect2I "Rect2I"
 - @ref bs::Vector2I "Vector2I"

# Time {#utilities_k}
To access timing information use the @ref bs::Time "Time" module, more easily accessible via @ref bs::gTime "gTime" method:
 - @ref bs::Time::getTime "Time::getTime" - Returns time since start-up in seconds, updated once per frame.
 - @ref bs::Time::getFrameDelta "Time::getFrameDelta" - Returns the time between execution of this and last frame.
 - @ref bs::Time::getFrameIdx "Time::getFrameIdx" - Returns a sequential index of the current frame.
 - @ref bs::Time::getTimePrecise "Time::getTimePrecise" - Returns time suitable for precision measurements. Returns time at the exact time it was called, instead of being updated once per frame.
 
# Logging {#utilities_l}
To report warnings and errors use the @ref bs::Debug "Debug" module. Call @ref bs::Debug::logDebug "Debug::logDebug", @ref bs::Debug::logWarning "Debug::logWarning" and @ref bs::Debug::logError "Debug::logError" to log messages. 

Use @ref bs::Debug::saveLog "Debug::saveLog" to save a log to the disk in HTML format. Use use @ref bs::Debug::getLog "Debug::getLog" to get a @ref bs::Log "Log" object you can manually parse.

Macros for common log operations are also provided: @ref LOGDBG, @ref LOGWRN and @ref LOGERR. They're equivalent to the methods above.

# Crash handling {#utilities_m}
Use the @ref bs::CrashHandler "CrashHandler" to report fatal errors. Call @ref bs::CrashHandler::reportCrash "CrashHandler::reportCrash" to manually trigger such an error. An error will be logged, a message box with relevant information displayed and the application terminated.

You can also use @ref BS_EXCEPT macro, which internally calls @ref bs::CrashHandler::reportCrash "CrashHandler::reportCrash" but automatically adds file/line information.

@ref bs::CrashHandler "CrashHandler" also provides @ref bs::CrashHandler::getStackTrace "CrashHandler::getStackTrace" that allows you to retrieve a stack trace to the current method.

# Dynamic libraries {#utilities_n}
Use @ref bs::DynLibManager "DynLibManager" to load dynamic libraries (.dll, .so). It has two main methods:
 - @ref bs::DynLibManager::load "DynLibManager::load" - Accepts a file name to the library, and returns the @ref bs::DynLib "DynLib" object if the load is successful or null otherwise. 
 - @ref bs::DynLibManager::unload "DynLibManager::unload" - Unloads a previously loaded library.
 
Once the library is loaded you can use the @ref bs::DynLib "DynLib" object, and its @ref bs::DynLib::getSymbol "DynLib::getSymbol" method to retrieve a function pointer within the dynamic library, and call into it. For example if we wanted to retrieve a function pointer for the `loadPlugin` method:
~~~~~~~~~~~~~{.cpp}
// Load library
DynLib* myLibrary = DynLibManager::instance().load("myPlugin");

// Retrieve function pointer (symbol)
typedef void* (*LoadPluginFunc)();
LoadPluginFunc loadPluginFunc = (LoadPluginFunc)myLibrary->getSymbol("loadPlugin");

// Call the function
loadPluginFunc();

// Assuming we're done, unload the plugin
DynLibManager::instance().unload(myLibrary);
~~~~~~~~~~~~~

# Testing {#utilities_o}
Implement @ref bs::TestSuite "TestSuite" to set up unit tests for your application. To register new tests call @ref BS_ADD_TEST. Test is assumed to succeed unless either @ref BS_TEST_ASSERT or @ref BS_TEST_ASSERT_MSG are triggered.

~~~~~~~~~~~~~{.cpp}
class MyTestSuite : TestSuite
{
public:
	EditorTestSuite()
	{
		BS_ADD_TEST(MyTestSuite::myTest);
	}
	
private:
	void myTest()
	{
		BS_TEST_ASSERT_MSG(2 + 2 == 4, "Something really bad is going on.");
	}
};
~~~~~~~~~~~~~

To run all tests create a instance of the @ref bs::TestSuite "TestSuite" and run it, like so:
~~~~~~~~~~~~~{.cpp}
SPtr<TestSuite> tests = MyTestSuite::create<MyTestSuite>();
tests->run(ExceptionTestOutput());
~~~~~~~~~~~~~

When running the test we provide @ref bs::ExceptionTestOutput "ExceptionTestOutput" which tells the test runner to terminate the application when a test fails. You can implement your own @ref bs::TestOutput "TestOutput" to handle test failure more gracefully.

# Allocators {#utilities_p}
Banshee allows you to allocate memory in various ways, so you can have fast memory allocations for many situations.
## General {#utilities_p_a}
The most common memory allocation operations are `new`/`delete` or `malloc`/`free`. Banshee provides its own wrappers for these methods as @ref bs::bs_new "bs_new"/@ref bs::bs_delete "bs_delete" and @ref bs::bs_alloc "bs_alloc"/@ref bs::bs_free "bs_free". They provide the same functionality but make it possible for Banshee to track memory allocations which can be useful for profiling and debugging. You should always use them instead of the standard ones.

Use @ref bs::bs_newN "bs_newN"/@ref bs::bs_deleteN "bs_deleteN" to create and delete arrays of objects.

~~~~~~~~~~~~~{.cpp}
UINT8* buffer = (UINT8*)bs_alloc(1024); // Allocate a raw buffer of 1024 bytes.
Vector2* vector = bs_new<Vector2>(); // Allocate and construct a vector
Vector2** vectors = bs_newN<Vector2>(5); // Allocate an array of five vectors

// Free and destruct everything
bs_free(buffer);
bs_delete(vector);
bs_deleteN(vectors, 5);
~~~~~~~~~~~~~

## Stack {#utilities_p_b}
Stack allocator allows you to allocate memory quickly, usually without a call to the OS memory manager, usually making the allocation only little more expensive than using the internal OS stack. It also allocates memory with zero fragmentation, which can be very important for large applications such as games. Whenever possible you should use this allocator instead of the general purpose allocator.

However it comes with a downside that it can only deallocate memory in the opposite order it was allocated. This usually only makes it suitable for temporary allocations within a single method, where you can guarantee the proper order.

Use @ref bs::bs_stack_alloc "bs_stack_alloc" / @ref bs::bs_stack_free "bs_stack_free" and @ref bs::bs_stack_new "bs_stack_new" / @ref bs::bs_stack_delete "bs_stack_delete" to allocate/free memory using the stack allocator.

For example:
~~~~~~~~~~~~~{.cpp}
UINT8* buffer = bs_stack_alloc(1024);
... do something with buffer ...
UINT8* buffer2 = bs_stack_alloc(512);
... do something with buffer2 ...
bs_stack_free(buffer2); // Must free buffer2 first!
bs_stack_free(buffer);
~~~~~~~~~~~~~

## Frame {#utilities_p_c}
Frame allocator is very similar to the stack allocator and it provides the same benefits (it's also very fast and causes no fragmentation). However it has different memory deallocation restrictions which make it usable in more situations than a stack allocator, at the cost of using up more memory.

Frame allocator segments all allocated memory into "frames". These frames are stored in a stack-wise fashion, and must be deallocated in the opposite order they were allocated, similar to how the stack allocator works. The difference is that frame allocator is not able to free memory for individual objects, but only for entire frames.

This releases the restriction that memory must be freed in the order it was allocated, which makes the allocator usable in more situations, but it also means that a lot of memory might be wasted as unused memory will be kept until the entire frame is freed.

Use @ref bs::bs_frame_alloc "bs_frame_alloc" / @ref bs::bs_frame_free "bs_frame_free" or @ref bs::bs_frame_new "bs_frame_new" / @ref bs::bs_frame_delete "bs_frame_delete" to allocate/free memory using the frame allocator. Calls to @ref bs::bs_frame_free "bs_frame_free" / @ref bs::bs_frame_delete "bs_frame_delete" are required even through the frame allocator doesn't process individual deallocations, and this is used primarily for debug purposes.

Use @ref bs::bs_frame_mark "bs_frame_mark" to start a new frame. All frame allocations should happen after this call. If you don't call @ref bs::bs_frame_mark "bs_frame_mark" a global frame will be used. Once done with your calculations use @ref bs::bs_frame_clear "bs_frame_clear" to free all memory in the current frame. The frames have to be released in opposite order they were created.

For example:
~~~~~~~~~~~~~{.cpp}
// Mark a new frame
bs_frame_mark();
UINT8* buffer = bs_frame_alloc(1024);
... do something with buffer ...
UINT8* buffer2 = bs_frame_alloc(512);
... do something with buffer2 ...
bs_frame_free(buffer); // Only does some checks in debug mode, doesn't actually free anything
bs_frame_free(buffer2); // Only does some checks in debug mode, doesn't actually free anything
bs_frame_clear(); // Frees memory for both buffers
~~~~~~~~~~~~~

You can also create your own frame allocators by constructing a @ref bs::FrameAlloc "FrameAlloc" and calling memory management methods on it directly. This can allow you to use a frame allocator on a more global scope. For example if you are running some complex algorithm involving multiple classes you might create a frame allocator to be used throughout the algorithm, and then just free all the memory at once when the algorithm finishes.

You may also use frame allocator to allocate containers like @ref bs::String "String", @ref bs::Vector "Vector" or @ref bs::Map "Map". Simply mark the frame as in the above example, and then use the following container alternatives: @ref bs::String "FrameString", @ref bs::FrameVector "FrameVector" or @ref bs::FrameMap "FrameMap" (other container types also available). For example:

~~~~~~~~~~~~~{.cpp}
// Mark a new frame
bs_frame_mark();
{
	FrameVector<UINT8> vector;
	... populate the vector ... // No dynamic memory allocation cost as with a normal Vector
} // Block making sure the vector is deallocated before calling bs_frame_clear
bs_frame_clear(); // Frees memory for the vector
~~~~~~~~~~~~~

## Static {#utilities_p_d}
@ref bs::StaticAlloc<BlockSize, MaxDynamicMemory> "Static allocator" is the only specialized type of allocator that is used for permanent allocations. It allows you to pre-allocate a static buffer on the internal stack. It will then use internal stack memory until it runs out, after which it will use normal dynamic allocations. If you can predict a good static buffer size you can guarantee that most of your objects don't allocate any heap memory, while wasting minimum memory on the stack. This kind of allocator is mostly useful when you have many relatively small objects, each of which requires dynamic allocation of a different size.

An example:
~~~~~~~~~~~~~{.cpp}
class MyObj
{
	StaticAlloc<512> mAlloc; // Ensures that every instance of this object has 512 bytes pre-allocated
	UINT8* mData = nullptr;
	
	MyObj(int size)
	{
		// As long as size doesn't go over 512 bytes, no dynamic allocations will be made
		mData = mAlloc.alloc(size);
	}
	
	~MyObj()
	{
		mAlloc.free(mData);
	}
}

~~~~~~~~~~~~~

## Shared pointers {#utilities_p_e}
Shared pointers are smart pointers that will automatically free memory when the last reference to the pointed memory goes out of scope. They're implemented as @ref bs::SPtr "SPtr", which is just a wrapper for the standard C++ library `std::shared_ptr`. Use @ref bs::bs_shared_ptr_new "bs_shared_ptr_new" to create a new shared pointer, or @ref bs::bs_shared_ptr "bs_shared_ptr" to create one from an existing instance. The pointer memory is allocated and freed using the general allocator.

For example:
~~~~~~~~~~~~~{.cpp}
class MyClass() {};

// Create a shared pointer with a new instance of MyClass
SPtr<MyClass> myObj = bs_shared_ptr_new<MyClass>();

MyClass* myRawObj = bs_new<MyClass>();

// Create a shared pointer with an existing instance of MyClass
SPtr<MyClass> myObj2 = bs_shared_ptr(myRawObj);
~~~~~~~~~~~~~