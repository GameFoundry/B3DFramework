---
title: Creating new resource types
---

Throughout the previous manuals we have shown how to import, load and save a variety of resource types. But you can also add brand new resource types of your own.

# Custom resource type
To create a custom resource type you need to implement the @b3d::Resource interface. **Resource** derives from @b3d::IReflectable, @b3d::IScriptExportable, and @b3d::CoreObject. These classes make up majority of its interface, and we have already shown how to implement them in the [render thread](../Low_Level_rendering/renderThread) and [serializing objects](User_Manuals/Gameplay/serializingObjects) manuals.

~~~~~~~~~~~~~{.cpp}
class MyResource : public Resource
{
public:
	MyResource()
		:Resource(false)
	{ }
	
	u32 someData; // Fields should be referenced by the RTTI code, so they get saved
	u32 moreData; 

	friend class MyResourceRTTI;
	static RTTITypeBase* GetRTTIStatic() { return MyResourceRTTI::instance(); }
	RTTITypeBase* GetRTTI() const override { return GetRTTIStatic(); }
};

// MyResourceRTTI implemented as a standard RTTI object, as discussed previously
~~~~~~~~~~~~~

The constructor of **Resource** accepts one parameter, signifying whether or not your resource requires a render thread representation. If you set this to false then you don't need to implement any of the render thread sync functionality (as described in [render thread](../01_Low_Level_rendering/00_renderThread.md) manual), and @b3d::CoreObject::GetRenderProxy() method will simply return null. In general if your resource doesn't need to be used on the render thread set this to false. On the other hand, if it is used by systems like the renderer or the render API, set it to true.

# Custom resource handle
To create a handle for your custom resource, similarly to **HTexture** or **HMesh**, simply create a *typedef* using the @b3d::ResourceHandle<T> type. Generally you want to define this in some header included by all files, for convenience.

~~~~~~~~~~~~~{.cpp}
typedef ResourceHandle<MyResource> HMyResource;
~~~~~~~~~~~~~

# Custom resource creation
When creating a new instance of your resource you need to follow all the rules for **CoreObject** creation. This will yield you a shared pointer to the resource. To create a handle to the resource you must call @b3d::Resources::CreateResourceHandle. Because of this complex initialization procedure you are encouraged to write a static **Create()** method that handles all of it internally, same as for **CoreObject**%s.

~~~~~~~~~~~~~{.cpp}
class MyResource : public Resource
{
public:
	// ... other MyResource code
	
	static HMyResource Create()
	{
		// Standard core object initialization
		TShared<MyResource> sptr = B3DMakeSharedFromExisting<MyResource>(new(B3DAllocate<MyResource>()) MyResource());
		sptr->SetShared(sptr);
		sptr->Initialize();
	
		// Create a handle
		return B3DStaticResourceCast<MyResource>(GetResources().CreateResourceHandle(sptr));
	}
};
~~~~~~~~~~~~~

# Optional features
Implementations above represent the minimal set of features to create your own resource, but there are some optional features we'll cover in this section.

## Dependencies
If your custom resource is dependant on some other resource make sure to implement the @b3d::Resource::GetResourceDependencies method which should return a list of all resources it is dependant upon. For example, a **Material** resource is dependant on the **Shader** resource, as well as any **Texture** resources set as its parameters.

~~~~~~~~~~~~~{.cpp}
// Assuming our MyResource class is dependant on a texture
class MyResource : public Resource
{
public:
	// ... other MyResource code
	
	void GetResourceDependencies(FrameVector<HResource>& dependencies) const override
	{
		dependencies.push_back(mSomeTexture);
	}
	
private:
	HTexture mSomeTexture;
};
~~~~~~~~~~~~~

Such dependant resources will be checked when @b3d::ResourceHandleBase::IsLoaded or @b3d::ResourceHandleBase::BlockUntilLoaded is called with `checkDependencies` parameter enabled. This ensures you have an easy to way to check if a resource is truly usable (it is loaded, as well as everything it depends on). 

## Resource listener
If an object is interested in learning when a certain resource has been loaded or updated you can implement the @b3d::IResourceListener interface. This interface can be implemented both on resources and on normal objects.

To implement it you must return a list of resource handles you are interested in tracking by implementing the @b3d::IResourceListener::GetListenerResources method. Whenever the dependant resource list changes you should call @b3d::IResourceListener::MarkListenerResourcesDirty.

~~~~~~~~~~~~~{.cpp}
class SomeClass : public IResourceListener
{
public:
	void SetTexture(const HTexture& tex)
	{
		mSomeTexture = tex;
		
		// This lets the listener know the list of listener resources changed and a new list needs to be retrieved from GetListenerResources()
		MarkListenerResourcesDirty();
	}

	void GetListenerResources(Vector<HResource>& resources)
	{
		// Place any resources you are interested in listening for in the "resources" vector
		if (mSomeTexture != nullptr)
			resources.push_back(mSomeTexture);
	}
	
private:
	HTexture mSomeTexture;
};
~~~~~~~~~~~~~

Whenever a resource you are listening for is loaded @b3d::IResourceListener::NotifyResourceLoaded will be called. Similarly whenever a resource is re-imported @b3d::IResourceListener::NotifyResourceChanged will be called.

~~~~~~~~~~~~~{.cpp}
class SomeClass : public IResourceListener
{
public:
	// ... other SomeClass code
	
	void NotifyResourceLoaded(const HResource& resource)
	{
		// "resource" could be an asynchronously loaded resource, and we wait on initialization of this object until it is loaded
		Initialize();
	}

	void NotifyResourceChanged(const HResource& resource)
	{
		// User could have triggered a reimport for the resource in which case we might need to do some form of re-initialization.
		// This is especially useful for resource hot-swap, where user can just reimport a resource with new data and the system
		// automatically starts using a new resource throughout the application.
		Reinitialize();
	}
};
~~~~~~~~~~~~~

**IResourceListener::NotifyResourceLoaded()** triggers when a resource is loaded in response to a @b3d::Resources::Load or @b3d::Resources::LoadAsync call.

**IResourceListener::NotifyResourceChanged()** triggers whenever user calls @b3d::Resources::Update. This method will update the contents of a resource handle with new resource data, and trigger any listeners.

**Resources** module also provides @b3d::Resources::onResourceLoaded, @b3d::Resources::onResourceDestroyed and @b3d::Resources::onResourceModified events which may be used for a similar purpose, but **IResourceListener** is more efficient since it only tracks specific resources, while these events trigger for every resource.
