Resource saving and loading					{#resourceSavingAndLoading}
===============
[TOC]

All resource save and load operations are managed through the @ref bs::Resources "Resources" module, accessible through @ref bs::gResources() "gResources"().

# Saving {#resourceSavingAndLoading_a}
Once a resource has been imported you can save it for later use. The advantage of saving a resource (instead of importing it every time) is performance - resource import is usually a costly operation. Saved resources remain in engine-friendly format and can be easily loaded later. 

To save a resource call @ref bs::Resources::save "Resources::save()". Lets see an example where we import a texture and then save it for later use:

~~~~~~~~~~~~~{.cpp}
// Import a texture named "myTexture.jpg" from the disk
HTexture texture = gImporter().import<Texture>("myTexture.jpg");

// Save the texture for later use
gResources().save(texture, "myTexture.asset");
~~~~~~~~~~~~~

Note that resources can also be created within the engine, and don't necessarily have to be imported. e.g. you can populate texture pixels or mesh vertices programatically, and then save the resource in this same manner. We will show later how to manually create resources.

# Loading {#resourceSavingAndLoading_b}
Once a resource has been saved you can load it at any time using @ref bs::Resources::load "Resources::load()". Lets load the texture we just saved:

~~~~~~~~~~~~~{.cpp}
HTexture loadedTexture = gResources().load<Texture>("myTexture.asset");
~~~~~~~~~~~~~

> If you attempt to load a resource that has already been loaded, the system will return the existing resource.

# Asynchronous loading {#resourceSavingAndLoading_c}
Resources can be loaded asynchronously (in the background) by calling @ref bs::Resources::loadAsync "Resources::loadAsync()". The interface is identical to **Resources::load()**. The main difference is that the returned handle will contain a reference to a resource that hasn't been loaded yet. 

You can check if a resource handle is pointing to a loaded resource by calling @ref bs::ResourceHandle::isLoaded "ResourceHandle::isLoaded()".

~~~~~~~~~~~~~{.cpp}
HMesh mesh = gResources().loadAsync("myMesh.asset");

if(mesh.isLoaded())
{
	// Check if loaded before doing something
}
~~~~~~~~~~~~~ 

> Note that not-yet-loaded resource handles can be provided to some engine systems, but not all. Generally a system will note in its documentation if it works with such resource handles.

You can block the current thread until a resource is loaded by calling @ref bs::ResourceHandle::blockUntilLoaded "ResourceHandle::blockUntilLoaded()".

~~~~~~~~~~~~~{.cpp}
mesh.blockUntilLoaded();
// Makes sure the mesh is loaded at this point
~~~~~~~~~~~~~ 

# Resource lifetime {#resourceSavingAndLoading_d}
Whenever you load a resource, that resource will be kept loaded until all references to that resource are lost. Each resource handle (e.g. **HMesh**) that exists represents a single reference. By default an additional "internal" reference is also created and held by the system internally. This ensures the resource stays loaded even when all handles are destroyed.

This internal reference must be released by calling @ref bs::Resources::release() "Resources::release()".

~~~~~~~~~~~~~{.cpp}
gResources().release(mesh);
~~~~~~~~~~~~~

> Note that if you called **Resources::load()** multiple times, you must also call **Resources::release()** the same amount of times.

But you can also force the system to not create the internal reference by passing a custom @ref bs::ResourceLoadFlag "ResourceLoadFlag" to the @ref bs::Resources::load() "Resources::load()" method. This flag should not include the @ref bs::ResourceLoadFlag::KeepInternalRef "ResourceLoadFlag::KeepInternalRef".

~~~~~~~~~~~~~{.cpp}
HMesh mesh = gResources().load("myMesh.asset", ResourceLoadFlag::Default & ~ResourceLoadFlag::KeepInternalRef);
~~~~~~~~~~~~~ 

# Weak handles {#resourceSavingAndLoading_e}
In case you want to keep a reference to a resource without incrementing the reference count you can use a weak handle instead of a normal one. Weak handles are represented by the @ref bs::WeakResourceHandle<T> "WeakResourceHandle<T>" class and can be retrieved from normal handles by calling @ref bs::ResourceHandle<T>::getWeak "ResourceHandle<T>::getWeak()". Other than that they are accessed the same as normal handles.

~~~~~~~~~~~~~{.cpp}
// Load a mesh and store a handle as normal
HMesh mesh = gResources().load("myMesh.asset");

// Create a weak handle
WeakResourceHandle<Mesh> weakMesh = mesh.getWeak();
~~~~~~~~~~~~~ 

# Resource dependencies {#resourceSavingAndLoading_f}
Whenever you load a resource the system will automatically enumerate all dependencies of that resource and attempt to load them as well. For example when loading a **Material** it will automatically load its **Shader** and any referenced **Texture** resources.

In case you wish to prevent that you can not provide the @ref bs::ResourceLoadFlag::LoadDependencies "ResourceLoadFlag::LoadDependencies" flag when calling **Resources::load()** (provided by default).

~~~~~~~~~~~~~{.cpp}
// Loads a resource without any dependencies, nor keeps an internal reference
HMesh mesh = gResources().load("myMesh.asset", ResourceLoadFlag::None);
~~~~~~~~~~~~~ 