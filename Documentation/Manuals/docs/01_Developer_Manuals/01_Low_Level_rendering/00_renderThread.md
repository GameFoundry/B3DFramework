---
title: Render Thread
---

Framework is a multi-threaded framework that has two primary threads. One is the main thread on which the application is started, this is where your game code runs and what majority of users will be working with, we call this the **simulation** thread. The second thread is the rendering thread, this is where all calls to render API (like Vulkan/DirectX) are made. 

Various other operations can use threads other than the two primary ones (async resource loading, physics, animation, etc.) in the form of worker threads or tasks. But we won't touch on those as they act as standard threads and require no special handling.

Dealing with the render thread on the other hand requires some knowledge of how it interacts with the simulation thread. The render thread deals with:
 - Render API calls (Vulkan/DirectX)
 - Renderer

Here are some examples of when the simulation thread needs to interact with the render thread:
 - You add a new renderable object to the scene. The simulation thread needs to notify the renderer.
 - You modify a property on a renderable/camera/light object. The simulation thread needs to notify the renderer.
 - You create a resource like a mesh or a texture. The simulation thread must schedule resource creation with the render API (i.e. creating vertex/index buffers for mesh, texture surfaces for texture).
 - You destroy a resource like a mesh or texture. The simulation thread must call into render API to free the GPU portions of those objects.

As you can see the communication is one directional. Simulation thread calls into the render thread when it needs to notify it of a certain event. In rare cases the render thread needs to notify the simulation thread (e.g. when the user moves/resizes the window the simulation thread needs to be aware of the new position/size), but this is handled on a per-case basis using normal thread synchronization primitives, and we won't touch on it further.

# Command queue
All the operations listed above happen with the help of a command queue. When simulation thread needs to notify the render thread about something it queues a command, which is then eventually received and processed by the render thread.

@b3d::RenderThread manages all operations on the command queue. Use @b3d::RenderThread::PostCommand to send a new command to the render thread.

~~~~~~~~~~~~~{.cpp}
void DoSomething()
{ }

// Queue DoSomething method to be executed on the render thread
GetRenderThread().PostCommand(&DoSomething, "DoSomething");
~~~~~~~~~~~~~

Commands are queued immediately and become visible to the render thread once posted. The render thread processes commands continuously as they arrive.

## Blocking execution
Sometimes you may need to wait for a command to complete before continuing execution on the calling thread. To do this, pass `true` as the third parameter to **RenderThread::PostCommand**. This will block the calling thread until the command finishes executing. Note that this is an expensive operation and shouldn't be done in performance critical code.

~~~~~~~~~~~~~{.cpp}
// Queue a command and wait until it completes
GetRenderThread().PostCommand(&DoSomething, "DoSomething", true);
~~~~~~~~~~~~~

## Returning values
Sometimes a queued command needs to return a value to the simulation thread (for example, when reading pixels from a texture). You can use @b3d::AsyncOp to handle this. The command callback should call @b3d::AsyncOp::CompleteOperation when done, and the calling thread can check if the operation has completed via @b3d::AsyncOp::HasCompleted or block until completion using @b3d::AsyncOp::BlockUntilComplete.

~~~~~~~~~~~~~{.cpp}
void DoSomethingAndReturn(AsyncOp& asyncOperation)
{
	int result = 5 + 3;
	asyncOperation.CompleteOperation(result);
}

AsyncOp asyncOperation;
GetRenderThread().PostCommand([asyncOperation]()
{
	DoSomethingAndReturn(asyncOperation);
}, "DoSomethingAndReturn");

// Do something else...

if(asyncOperation.HasCompleted())
{
	String valueString = ToString(asyncOperation.GetReturnValue<int>());
	B3D_LOG(Warning, LogUncategorized, "Returned value: {0}", valueString);
}
~~~~~~~~~~~~~

**AsyncOp** also allows you to block the calling thread by calling @b3d::AsyncOp::BlockUntilComplete. This is similar to blocking directly on the **RenderThread::PostCommand()** call, but can be more useful if you're not immediately sure if you need to wait for the result or not.

# Core objects
Core objects are objects that need to exist on both simulation and render threads. Although you could technically handle such cases manually by using the command queue, it is useful to provide an interface that allows the user to work normally with an object without needing to know about the threading internals, and this is where core objects come in.

For example, a @b3d::Mesh is a core object because we want to allow the user to intuitively work with it on the simulation thread (without having to know about command queues or the render thread), but we also want to use it on the render thread (it needs to create index/vertex buffers on the GPU, and have a Vulkan/DirectX representation that can be used by the renderer).

Every core object is split into two interfaces:
 - @b3d::CoreObject - Implementations of this interface represents the simulation thread counterpart of the object.
 - @b3d::render::RenderProxy - Implementations of this interface represents the render thread counterpart of the object.

When a **CoreObject** is created it internally queues the creation of its **render::RenderProxy** counterpart on the command queue. Similar thing happens when it is destroyed, a destroy operation is queued and sent to the render thread.

Aside from initialization/destruction, core objects also support synchronization of data between the two threads (e.g. a @b3d::Light is a core object, and when the user changes light radius, it is automatically synchronized to its render thread counterpart @b3d::render::Light). We talk more about this later.

Both core thread counterpart class objects have the same name (e.g. **Mesh** or **Light**), but the render thread counterpart is in the *render* namespace. In fact, most classes meant to be used on the render thread (core objects or not), will be in the *render* namespace.

## Creating your own core objects
To create a custom core object, you need to implement the **CoreObject** class, and its render thread counterpart **render::RenderProxy**.

~~~~~~~~~~~~~{.cpp}
class MyCoreObject : public CoreObject
{
	// ...
};

namespace render
{
	class MyCoreObject : public RenderProxy
	{
		// ...
	};
}
~~~~~~~~~~~~~

> Note that usually you want these two classes to share data and functionality (at least somewhat), and therefore you'll want to use base classes or templates to avoid redundant code.

At minimum the **CoreObject** implementation requires an implementation of the @b3d::CoreObject::CreateRenderProxy method, which creates and returns the render thread counterpart of the object.

~~~~~~~~~~~~~{.cpp}
class MyCoreObject : public CoreObject
{
	SPtr<render::RenderProxy> CreateRenderProxy() const override
	{
		render::MyCoreObject* renderProxy = new (B3DAllocate<render::MyCoreObject>()) render::MyCoreObject();
		SPtr<render::MyCoreObject> renderProxyShared = B3DMakeSharedFromExisting<render::MyCoreObject>(renderProxy);
		renderProxyShared->SetShared(renderProxyShared);

		return renderProxyShared;
	}
};
~~~~~~~~~~~~~

When creating your core object it's important to note they require specific initialization steps. As seen in the example, **render::RenderProxy** implementation needs to be created using placement new with **B3DAllocate**, then wrapped in a shared pointer using **B3DMakeSharedFromExisting**, and the pointer instance must be assigned after creation by calling @b3d::render::RenderProxy::SetShared.

For **CoreObject** implementation additional rules apply. Its shared pointer must be created using @b3d::B3DMakeSharedFromExisting<T> method, followed by a call to @b3d::CoreObject::SetShared and finally a call to @b3d::CoreObject::Initialize. Due to the complex initialization procedure it is always suggested that you create a static `Create` method that does these steps automatically. In fact **CoreObject** constructor is by default protected so you cannot accidently create it incorrectly.

~~~~~~~~~~~~~{.cpp}
SPtr<MyCoreObject> MyCoreObject::Create()
{
	// Because of the protected constructor we need to use placement new operator
	MyCoreObject* object = new (B3DAllocate<MyCoreObject>()) MyCoreObject();

	SPtr<MyCoreObject> sharedPointer = B3DMakeSharedFromExisting<MyCoreObject>(object);
	sharedPointer->SetShared(sharedPointer);
	sharedPointer->Initialize();

	return sharedPointer;
}
~~~~~~~~~~~~~

Once a core object is created you can use it as a normal object, while you can retrieve its render thread counterpart by calling @b3d::CoreObject::GetRenderProxy, which you can use on the render thread (e.g. when calling **RenderThread::PostCommand()**). Object creation/destruction will happen automatically on the valid thread, and you also get the ability to synchronize information between the two (see below).

### render::RenderProxy initialization
When creating the render thread counterpart object **render::RenderProxy** it is important to perform any initialization in the @b3d::render::RenderProxy::Initialize method instead of the constructor. This is because the constructor will be executed on the simulation thread, but **RenderProxy::Initialize()** will be executed on the render thread.

The destructor is always assumed to be executed on the render thread. For this reason you must ensure never to store references to **render::RenderProxy** on the simulation thread, because if they go out of scope there it will trigger an error. Similar rule applies to **CoreObject** as it shouldn't be stored on the render thread.

### Synchronization
Earlier we mentioned that aside from handling construction/destruction the core objects also provide a way to synchronize between the two threads. The synchronization is always one way, from **CoreObject** to **render::RenderProxy**.

Synchronization should happen whenever some property on the **CoreObject** changes, that you would wish to make available on the render thread (e.g. a radius of a light source). To synchronize implement the @b3d::CoreObject::CreateRenderProxySyncPacket method, which generates a data packet for synchronization, and @b3d::render::RenderProxy::SyncFromCoreObject which applies it.

The synchronized data is transferred between the objects via a @b3d::RenderProxySyncPacket structure. For convenience the engine provides a set of macros to make defining sync packets easier: @b3d::B3D_SYNC_BLOCK_BEGIN, @b3d::B3D_SYNC_BLOCK_ENTRY, and @b3d::B3D_SYNC_BLOCK_END.

Here's an example of how to define a sync packet:

~~~~~~~~~~~~~{.cpp}
// In the header file, declare the sync packet structure
class MyCoreObject : public CoreObject
{
	struct FullSyncPacket; // Forward declaration

	// ...
private:
	int mField1;
	float mField2;
};

namespace render
{
	class MyCoreObject : public RenderProxy
	{
		friend class b3d::MyCoreObject;

		// ...
	private:
		int mField1;
		float mField2;
	};
}

// In the cpp file, define the sync packet using macros
B3D_SYNC_BLOCK_BEGIN(MyCoreObject, FullSyncPacket)
	B3D_SYNC_BLOCK_ENTRY(mField1)
	B3D_SYNC_BLOCK_ENTRY(mField2)
B3D_SYNC_BLOCK_END
~~~~~~~~~~~~~

The sync packet definition creates a structure that automatically copies fields from the source object (simulation thread) and can apply them to the destination object (render thread). The packet data is allocated using a fast frame allocator that doesn't require explicit deallocation.

To use the sync packet, implement **CoreObject::CreateRenderProxySyncPacket** and **RenderProxy::SyncFromCoreObject**:

~~~~~~~~~~~~~{.cpp}
// CoreObject (creates the synchronization data)
RenderProxySyncPacket* MyCoreObject::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	FullSyncPacket* syncPacket = allocator.Construct<FullSyncPacket>(*this, allocator, flags);
	return syncPacket;
}

// render::RenderProxy (receives the synchronization data)
void MyCoreObject::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	RenderProxySyncPacket* syncPacket = data.GetSyncPacket();
	if(syncPacket == nullptr)
		return;

	syncPacket->ApplySyncData(this);

	// Potentially trigger something depending on new data
}
~~~~~~~~~~~~~

Whenever you need to trigger synchronization you must call @b3d::CoreObject::MarkRenderProxyDataDirty which notifies the system that synchronization is required. This will in turn trigger a call to **CoreObject::CreateRenderProxySyncPacket** method you implemented earlier. Synchronization happens automatically for all dirty core objects once per frame. Optionally you may call @b3d::CoreObject::SyncToRenderProxy() to manually queue the synchronization immediately.

~~~~~~~~~~~~~{.cpp}
void MyCoreObject::SetField1(int value)
{
	mField1 = value;
	MarkRenderProxyDataDirty(); // Schedule sync to render thread
}
~~~~~~~~~~~~~

The sync packet macros support several variants:
 - **B3D_SYNC_BLOCK_ENTRY** - Automatically syncs a field that exists in both objects
 - **B3D_SYNC_BLOCK_ENTRY_CUSTOM_SETTER** - Field is automatically read from source but must be manually applied to destination
 - **B3D_SYNC_BLOCK_ENTRY_CUSTOM_GETTER** - Field must be manually populated in packet but is automatically applied to destination
 - **B3D_SYNC_BLOCK_ENTRY_CUSTOM** - Field must be manually populated and applied

### Dependencies
Core objects might be dependant on other core objects. For example a @b3d::Material is dependant on a @b3d::Shader. Whenever the shader's object is marked as dirty the material might need to perform synchronization as well. In general whenever a dependency core object is marked as dirty, its dependant will be synchronized as well.

To add dependencies implement the @b3d::CoreObject::GetCoreDependencies method, which populates a provided array with all currently valid dependencies. Whenever the dependencies change call @b3d::CoreObject::MarkDependenciesDirty so the system can refresh its dependency list.

You can also optionally override @b3d::CoreObject::OnDependencyDirty to customize behavior when a dependency is marked dirty, allowing fine-grained control over which dirty flags should propagate to the dependent object.

## Deserialization
When creating RTTI for a **CoreObject** you must take care not to fully initialize the object until deserialization of the object's fields is done.

Essentially this means that @b3d::RTTITypeBase::NewRttiObject must return a pointer to the core object on which **CoreObject::Initialize()** hasn't been called yet. You must then call **CoreObject::Initialize()** manually in @b3d::RTTIType::OnOperationEnded.

This ensures that all information was properly deserialized before **CoreObject::Initialize()** is ran.

## Other features
Core objects also have some other potentially useful features:
 - @b3d::CoreObject::GetInternalId will return a globally unique ID for the core object
 - @b3d::CoreObject::Destroy will destroy the core object and its render thread counterpart. You do not need to call this manually as it will be automatically called when the object goes out of scope (is no longer referenced). The render thread counterpart will not be destroyed if something on the render thread is still holding a reference to it.
 - Override @b3d::CoreObject::Initialize or @b3d::CoreObject::Destroy methods instead of using the constructor/destructor. This ensures that your initialization code runs after things like serialization, and also allows you to call virtual methods.
 - You can construct a core object without a render thread counterpart by passing `false` to the **CoreObject** constructor. Simply don't override @b3d::CoreObject::CreateRenderProxy. This is useful when creating resources, which all by default derive from **CoreObject** but simpler resources might not require render proxy features.
 - Core objects always hold a shared pointer to themselves. Use @b3d::CoreObject::GetShared to access it.
