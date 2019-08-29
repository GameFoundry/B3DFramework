---
title: Core Thread
---

bs::f is a multi-threaded framework that has two primary threads. One is the main thread on which the application is started, this is where your game code runs and what majority of users will be working with, we call this the **simulation** thread. The second thread is the rendering thread, this is where all calls to render API (like Vulkan/DirectX/OpenGL) are made. This thread also deals with the OS (like the main message loop). We call this the **core** thread.

Various other operations can use threads other than the two primary ones (async resource loading, physics, animation, etc.) in the form of worker threads or tasks. But we won't touch on those as they act as standard threads and require no special handling.

Dealing with the core thread on the other hand requires some knowledge of how it interacts with the simulation thread. The core threads deals with:
 - Render API calls (Vulkan/DirectX/OpenGL)
 - Renderer
 - OS functionality (window creation, main message loop)

Here are some examples of when the simulation thread needs to interact with the core thread:
 - You add a new renderable object to the scene. The simulation thread needs to notify the renderer.
 - You modify a property on a renderable/camera/light object. The simulation thread needs to notify the renderer.
 - You create a resource like a mesh or a texture. The simulation thread must schedule resource creation with the render API (i.e. creating vertex/index buffers for mesh, texture surfaces for texture).
 - You destroy a resource like a mesh or texture. The simulation thread must call into render API to free the GPU portions of those objects.
 - You create a new window and the simulation thread needs to schedule its creation on the core thread.
 - You resize/move a window and the simulation thread needs to notify the core thread.
 
As you can see the communication is one directional. Simulation thread calls into the core thread when it needs to notify it of a certain event. In rare cases the core thread needs to notify the simulation thread (e.g. when the user moves/resizes the window the simulation thread needs to be aware of the new position/size), but this is handled on a per-case basis using normal thread synchronization primitives, and we won't touch on it further.

# Command queue
All the operations listed above happen with the help of a command queue. When simulation thread needs to notify the core thread about something it queues a command, which is then eventually received and processed by the core thread.

@bs::CoreThread manages all operations on the command queues. Use @bs::CoreThread::queueCommand to send a new command to the core thread.

~~~~~~~~~~~~~{.cpp}
void doSomething()
{ }

// Queue doSomething method to be executed on the core thread
gCoreThread().queueCommand(&doSomething);
~~~~~~~~~~~~~

Note that each thread has its own internal command queue. So calling this method from different threads will fill up their separate command queues. This is important because queuing the command does not actually make it sent to the core thread yet. Instead you must submit the commands after you are done queuing.

## Submitting commands
Commands that are queued aren't yet visible to the core thread. In order to make them visible you must call @bs::CoreThread::submit, which will submit all the commands for the current thread's command queue. You may also call @bs::CoreThread::submitAll to submit queues for all threads.

~~~~~~~~~~~~~{.cpp}
// Submit all commands queued since the last submit call
gCoreThread().submit();
~~~~~~~~~~~~~

By default commands are also submitted automatically at the end of every frame, just before rendering. Therefore normally you do not need to submit commands manually.

The most common case for submitting commands manually is when you need the results of the command right away. In such a case you can call **CoreThread::submit()** or **CoreThread::submitAll()** with the parameter `blockUntilComplete` set to true. This will block the calling thread until the command finishes executing. Note that this is an expensive operation and shouldn't be done in performance critical code.

~~~~~~~~~~~~~{.cpp}
// Submit all commands queued since the last submit call and wait until they're done executing
gCoreThread().submit(true);
~~~~~~~~~~~~~

## Internal queue
Internal command queue is different from the per-thread command queues because that's the only command queue that the core thread actually sees. For example, when you call **CoreThread::submit()** the system takes all the commands from the per-thread command queue and moves them to the internal command queue, making them visible to the core thread.

You may directly queue commands on the internal command queue by calling **CoreThread::queueCommand()** with the @bs::CTQF_InternalQueue flag. When such a command is submitted it is immediately visible to the core thread and does not require a separate call to **CoreThread::submit()**. 

If you wish to block the calling thread until the internally queued command finishes executing you may provide the @bs::CTQF_BlockUntilComplete flag.

~~~~~~~~~~~~~{.cpp}
void doSomething()
{ }

// Submit a command directly on the internal queue
gCoreThread().queueCommand(&doSomething, CTQF_InternalQueue);
~~~~~~~~~~~~~

There is only one internal command queue, so different threads can write to it in an interleaved manner, unlike with per-thread queues. Note that internal command queue is slower than per-thread queues and you should prefer them instead.

Also note that since commands queued on the internal command queue are seen by the core thread immediately, they will execute before commands previously queued on per-thread queues, unless they were submitted before you queued the command on the internal queue.

## Returning values
Sometimes a queued command needs to return a value to the simulation thread (for example, when reading pixels from a texture). This can be performed by calling @bs::CoreThread::queueReturnCommand. Aside from the return value it operates in the same manner as **CoreThread::queueCommand()**.

Since we don't know when will the core thread execute a queued command, we have no guarantees when will its return value be available. Therefore this method will return an @bs::AsyncOp object. This object can be used for checking if the return value is available by calling @bs::AsyncOp::hasCompleted. If the return value is available you can retrieve it via @bs::AsyncOp::getReturnValue<T>.

~~~~~~~~~~~~~{.cpp}
void doSomethingAndReturn(AsyncOp& asyncOp)
{
	int result = 5 + 3;
	asyncOp._completeOperation(result);
}

AsyncOp asyncOp = gCoreThread().queueReturnCommand(&doSomethingAndReturn);
gCoreThread().submit();

// do something ...

if(asyncOp.hasCompleted()) // Or just block until done, as mentioned below
{
	String valueStr = toString(asyncOp.getReturnValue<int>());
	gDebug().logWarning("Returned value: " + valueStr);
}
~~~~~~~~~~~~~

As seen in the example, the callback method must accept an **AsyncOp** parameter, and set its data when done by calling @bs::AsyncOp::_completeOperation().

**AsyncOp** also allows you to block the calling thread by calling @bs::AsyncOp::blockUntilComplete. This is similar to blocking directly on the **CoreThread::submit()** or **CoreThread::queueReturnCommand()** calls, but can be more useful if you're not immediately sure if you need to wait for the result or not.

# Core objects
Core objects are objects that need to exist on both simulation and core threads. Although you could technically handle such cases manually by using the command queue, it is useful to provide an interface that allows the user to work normally with an object without needing to know about the threading internals, and this is where core objects come in.

For example, a @bs::Mesh is a core object because we want to allow the user to intuitively work with it on the simulation thread (without having to know about command queues or the core thread), but we also want to use it on the core thread (it needs to create index/vertex buffers on the GPU, and have a Vulkan/DirectX/OpenGL representation that can be used by the renderer).

Every core object is split into two interfaces:
 - @bs::CoreObject - Implementations of this interface represents the simulation thread counterpart of the object.
 - @bs::ct::CoreObject - Implementations of this interface represents the core thread counterpart of the object.
  
When a **CoreObject** is created it internally queues the creation of its **ct::CoreObject** counterpart on the command queue. Similar thing happens when it is destroyed, a destroy operation is queued and sent to the core thread. 

Aside from initialization/destruction, core objects also support synchronization of data between the two threads (e.g. a @bs::Light is a core object, and when the user changes light radius, it is automatically synchronized to its core thread counterpart @bs::ct::Light). We talk more about this later.

Both core thread counterpart class objects have the same name (e.g. **Mesh** or **Light**), but the core-thread counterpart is in the *ct* namespace. In fact, most classes meant to be used on the core thread (core objects or not), will be in the *ct* namespace.

## Creating your own core objects
To create a custom core object, you need to implement the **CoreObject** class, and its core thread counterpart **ct::CoreObject**.

~~~~~~~~~~~~~{.cpp}
class MyCoreObject : public CoreObject
{
	// ...
};

namespace ct
{
	class MyCoreObject : public CoreObject
	{
		// ...
	};
}
~~~~~~~~~~~~~

> Note that usually you want these two classes to share data and functionality (at least somewhat), and therefore you'll want to use base classes or templates to avoid redundant code.

At minimum the **CoreThread** implementation requires an implementation of the @bs::CoreObject::createCore method, which creates and returns the core-thread counterpart of the object.

~~~~~~~~~~~~~{.cpp}
class MyCoreObject : public CoreObject
{
	SPtr<ct::CoreObject> createCore() override const 
	{ 
		SPtr<ct::MyCoreObject> ptr = bs_shared_ptr_new<ct::MyCoreObject>();
		ptr->_setThisPtr(ptr);
		
		return ptr; 
	}
};
~~~~~~~~~~~~~

When creating your core object it's important to note they require specific initialization steps. As seen in the example, **ct::CoreObject** implementation needs to be created as a normal shared pointer, and the pointer instance must be assigned after creation by calling @bs::ct::CoreObject::_setThisPtr.

For **CoreObject** implementation additional rules apply. Its shared pointer must be created using @bs::bs_core_ptr<T> method, followed by a call to @bs::CoreObject::_setThisPtr and finally a call to @bs::CoreObject::initialize. Due to the complex initialization procedure it is always suggested that you create a static `create` method that does these steps automatically. In fact **CoreObject** constructor is by default protected so you cannot accidently create it incorrectly.

~~~~~~~~~~~~~{.cpp}
SPtr<MyCoreObject> MyCoreObject::create()
{
	// Because of the protected constructor we need to use placement new operator
	MyCoreObject* ptr = new (bs_alloc<MyCoreObject>()) MyCoreObject();

	SPtr<MyCoreObject> sptr = bs_core_ptr<MyCoreObject>(ptr);
	sptr->_setThisPtr(sptr);
	sptr->initialize();
	
	return sptr;
}
~~~~~~~~~~~~~

Once a core object is created you can use it as a normal object, while you can retrieve its core thread counterpart by calling @bs::CoreObject::getCore, which you can use on the core thread (e.g. when calling **CoreThread::queueCommand()**). Object creation/destruction will happen automatically on the valid thread, and you also get the ability to synchronize information between the two (see below).

### ct::CoreObject initialization
When creating the core thread counterpart object **ct::CoreObject** it is important to perform any initialization in the @bs::CoreObject::initialize method instead of the constructor. This is because the constructor will be executed on the simulation thread, but **CoreObject::initialize()** will be executed on the core thread.

The destructor is always assumed to be executed on the core thread. For this reason you must ensure never to store references to **ct::CoreObject** on the simulation thread, because if they go out of scope there it will trigger an error. Similar rule applies to **CoreObject** as it shouldn't be stored on the core thread.

### Synchronization
Earlier we mentioned that aside from handling construction/destruction the core objects also provide a way to synchronize between the two threads. The synchronization is always one way, from **CoreObject** to **ct::CoreObject**. 

Synchronization should happen whenever some property on the **CoreObject** changes, that you would wish to make available on the core thread (e.g. a radius of a light source). To synchronize implement the @bs::CoreObject::syncToCore(FrameAlloc*) method, which generates the data for synchronization, and @bs::ct::CoreObject::syncToCore which accepts it.

The synchronized data is transfered between the objects in the form of raw bytes, within the @bs::CoreSyncData structure. For convenience you can use @bs::rtti_size and @bs::rtti_write to encode fields into raw memory, and @bs::rtti_read to decode them. These are explained in more detail in the [advanced RTTI manual](../../User_Manuals/advancedRtti).

**CoreObject::syncToCore()** is provided an instance of @bs::FrameAlloc which should be used for allocating the serialization buffer. This is an allocator that is fast and doesn't require explicit memory deallocation making it perfect for synchronization. A simple synchronization example would look like so:
~~~~~~~~~~~~~{.cpp}
// CoreObject (creates the synchronization data)
CoreSyncData MyCoreObject::syncToCore(FrameAlloc* allocator) 
{
	uint32_t size = 0;
	size += rtti_size(mField1);
	size += rtti_size(mField2);

	uint8_t* buffer = allocator->alloc(size);
	Bitstream stream(buffer, size);

	rtti_write(mField1, stream);
	rtti_write(mField2, stream);

	return CoreSyncData(buffer, size);
}

// ct::CoreObject (receives the synchronization data)
void MyCoreObject::syncToCore(const CoreSyncData& data) 
{
	Bitstream stream(data.getBuffer(), data.getBufferSize());
	rtti_read(mField1, stream);
	rtti_read(mField2, stream); 
	
	// Potentially trigger something depending on new data
}
~~~~~~~~~~~~~

Whenever you need to trigger synchronization you must call @bs::CoreObject::markCoreDirty which notifies the system that synchronization is required. This will in turn trigger a call to **CoreObject::syncToCore** method you implemented earlier. Synchronization happens automatically for all dirty core objects once per frame. Optionally you may call @bs::CoreObject::syncToCore() to manually queue the synchronization on the per-thread command queue.

### Dependencies
Core objects might be dependant on other core objects. For example a @bs::Material is dependant on a @bs::Shader. Whenever the shader's object is marked as dirty the material might need to perform synchronization as well. In general whenever a dependency core object is marked as dirty, its dependant will be synchronized as well.

To add dependencies implement the @bs::CoreObject::getCoreDependencies method, which returns all currently valid dependencies. Whenever the dependencies change call @bs::CoreObject::markDependenciesDirty so the system can refresh its dependency list.

## Deserialization
When creating RTTI for a **CoreObject** you must take care not to fully initialize the object until deserialization of the object's fields is done.

Essentially this means that @bs::RTTITypeBase::newRTTIObject must return a pointer to the core object on which **CoreObject::initialize()** hasn't been called yet. You must then call **CoreObject::initialize()** manually in @bs::RTTITypeBase::onDeserializationEnded.

This ensures that all information was properly deserialized before **CoreObject::initialize()** is ran.

## Other features
Core objects also have some other potentially useful features:
 - @bs::CoreObject::getInternalID will return a globally unique ID for the core object
 - @bs::CoreObject::destroy will destroy the core object and its core thread counterpart. You do not need to call this manually as it will be automatically called when the object goes out of scope (is no longer referenced). The core thread counterpart will not be destroyed if something on the core thread is still holding a reference to it.
 - Override @bs::CoreObject::initialize or @bs::CoreObject::destroy methods instead of using the constructor/destructor. This ensures that your initialization code runs after things like serialization, and also allows you to call virtual methods.
 - You can construct a core object without a core thread counterpart. Simply don't override @bs::CoreObject::createCore. This is useful when creating resources, which all by default derive from **CoreObject** but simpler resources might not require core object features.
 - Core objects always hold a shared pointer to themselves. Use @bs::CoreObject::getThisPtr to access it.
