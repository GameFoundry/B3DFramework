---
title: Creating new resource types
---

Throughout the previous manuals we have shown how to import, load and save a variety of resource types. But you can also add brand new resource types of your own.

# Custom resource type
To create a custom resource type you need to implement the @bs::Resource interface. **Resource** derives from both @bs::CoreObject and @bs::IReflectable. These two classes make up majority of its interface, and we have already shown how to implement them in the @ref coreThread and @ref serializingObjects manuals.

~~~~~~~~~~~~~{.cpp}
class MyResource : public Resource
{
public:
	MyResource()
		:Resource(false)
	{ }
	
	UINT32 someData; // Fields should be referenced by the RTTI code, so they get saved
	UINT32 moreData; 

	friend class MyResourceRTTI;
	static RTTITypeBase* getRTTIStatic() { return MyResourceRTTI::instance(); }
	RTTITypeBase* getRTTI() const override { return getRTTIStatic(); }
};

// MyResourceRTTI implemented as a standard RTTI object, as discussed previously
~~~~~~~~~~~~~

The constructor of **Resource** accepts one parameter, signifying whether or not your resource requires a core-thread representation. If you set this to false then you don't need to implement any of the core thread sync functionality (as described in @ref coreThread manual), and @bs::CoreObject::getCore() method will simply return null. In general if your resource doesn't need to be used on the core thread set this to false. On the other hand, if it is used by systems like the renderer or the render API, set it to true.

# Custom resource handle
To create a handle for your custom resource, similarly to **HTexture** or **HMesh**, simply create a *typedef* using the @bs::ResourceHandle<T> type. Generally you want to define this in some header included by all files, for convenience.

~~~~~~~~~~~~~{.cpp}
typedef ResourceHandle<MyResource> HMyResource;
~~~~~~~~~~~~~

# Custom resource creation
When creating a new instance of your resource you need to follow all the rules for **CoreObject** creation. This will yield you a shared pointer to the resource. To create a handle to the resource you must call @bs::Resources::_createResourceHandle. Because of this complex initialization procedure you are encouraged to write a static **create()** method that handles all of it internally, same as for **CoreObject**%s.

~~~~~~~~~~~~~{.cpp}
class MyResource : public Resource
{
public:
	// ... other MyResource code
	
	static HMyResource create()
	{
		// Standard core object initialization
		SPtr<MyResource> sptr = bs_core_ptr<MyResource>(bs_new<MyResource>());
		sptr->_setThisPtr(sptr);
		sptr->initialize();
	
		// Create a handle
		return static_resource_cast<MyResource>(gResources()._createResourceHandle(sptr));
	}
};
~~~~~~~~~~~~~

# Optional features
Implementations above represent the minimal set of features to create your own resource, but there are some optional features we'll cover in this section.

## Dependencies
If your custom resource is dependant on some other resource make sure to implement the @bs::Resource::getResourceDependencies method which should return a list of all resources it is dependant upon. For example, a **Material** resource is dependant on the **Shader** resource, as well as any **Texture** resources set as its parameters.

~~~~~~~~~~~~~{.cpp}
// Assuming our MyResource class is dependant on a texture
class MyResource : public Resource
{
public:
	// ... other MyResource code
	
	void getResourceDependencies(FrameVector<HResource>& dependencies) const override
	{
		dependencies.push_back(mSomeTexture);
	}
	
private:
	HTexture mSomeTexture;
};
~~~~~~~~~~~~~

Such dependant resources will be checked when @bs::ResourceHandleBase::isLoaded or @bs::ResourceHandleBase::blockUntilLoaded is called with `checkDependencies` parameter enabled. This ensures you have an easy to way to check if a resource is truly usable (it is loaded, as well as everything it depends on). 

## Resource listener
If an object is interested in learning when a certain resource has been loaded or updated you can implement the @bs::IResourceListener interface. This interface can be implemented both on resources and on normal objects.

To implement it you must return a list of resource handles you are interested in tracking by implementing the @bs::IResourceListener::getListenerResources method. Whenever the dependant resource list changes you should call @bs::IResourceListener::markListenerResourcesDirty.

~~~~~~~~~~~~~{.cpp}
class SomeClass : public IResourceListener
{
public:
	void setTexture(const HTexture& tex)
	{
		mSomeTexture = tex;
		
		// This lets the listener know the list of listener resources changed and a new list needs to be retrieved from getListenerResources()
		markListenerResourcesDirty();
	}

	void getListenerResources(Vector<HResource>& resources)
	{
		// Place any resources you are interested in listening for in the "resources" vector
		if (mSomeTexture != nullptr)
			resources.push_back(mSomeTexture);
	}
	
private:
	HTexture mSomeTexture;
};
~~~~~~~~~~~~~

Whenever a resource you are listening for is loaded @bs::IResourceListener::notifyResourceLoaded will be called. Similarly whenever a resource is re-imported @bs::IResourceListener::notifyResourceChanged will be called.

~~~~~~~~~~~~~{.cpp}
class SomeClass : public IResourceListener
{
public:
	// ... other SomeClass code
	
	void notifyResourceLoaded(const HResource& resource)
	{
		// "resource" could be an asynchronously loaded resource, and we wait on initialization of this object until it is loaded
		initialize();
	}

	void notifyResourceChanged(const HResource& resource)
	{
		// User could have triggered a reimport for the resource in which case we might need to do some form of re-initialization.
		// This is especially useful for resource hot-swap, where user can just reimport a resource with new data and the system
		// automatically starts using a new resource throughout the application.
		reinitialize();
	}
};
~~~~~~~~~~~~~

**IResourceListener::notifyResourceLoaded()** triggers when a resource is loaded in response to a @bs::Resources::load or @bs::Resources::loadAsync call.

**IResourceListener::notifyResourceChanged()** triggers whenever user calls @bs::Resources::update. This method will update the contents of a resource handle with new resource data, and trigger any listeners.

**Resources** module also provides @bs::Resources::onResourceLoaded, @bs::Resources::onResourceDestroyed and @bs::Resources::onResourceModified events which may be used for a similar purpose, but **IResourceListener** is more efficient since it only tracks specific resources, while these events trigger for every resource.
