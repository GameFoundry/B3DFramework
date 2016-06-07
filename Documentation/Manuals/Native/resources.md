Resources						{#resources}
===============
[TOC]

Resources are objects that can be saved and loaded to/from a disk, can be referenced in a unique manner, and have their lifetime managed by the resources manager. Objects like @ref BansheeEngine::Texture "Texture", @ref BansheeEngine::Mesh "Mesh" or @ref BansheeEngine::Shader "Shader" are all resources.

A resource in Banshee is represented by the @ref BansheeEngine::Resource "Resource" class. A @ref BansheeEngine::Resource "Resource" is a @ref BansheeEngine::CoreObject "CoreObject" and if you're not familiar with core objects you should read the [core object](@ref coreThread) manual.

All resources in Banshee are managed by the @ref BansheeEngine::Resources "Resources" manager. Use @ref BansheeEngine::gResources "gResources()" to access its instance easily.

# Loading {#resources_a}
To load a resource call @ref BansheeEngine::Resources::load "Resource::load" with a file path to the resource. If the path is valid a handle to the resource will be returned.

You can also call @ref BansheeEngine::Resources::loadAsync "Resource::loadAsync". Unlike the previous method this method will not block the current thread and will instead proceed with loading the resource on a separate thread. You will receive an uninitialized resource handle. You must use @ref BansheeEngine::ResourceHandleBase::isLoaded "HResource::isLoaded" to check if the handle is loaded before using it, or @ref BansheeEngine::ResourceHandleBase::blockUntilLoaded "HResource::blockUntilLoaded" to force the current thread to wait until it is loaded.

An example of both synchronous and asynchronous loads:
~~~~~~~~~~~~~{.cpp}
// Load a texture synchronously
HTexture textureA = gResources().load<Texture>("textureA.asset");

// Load a texture asynchronously
HTexture textureB = gResources().loadAsync<Texture>("textureB.asset");

while(!textureB.isLoaded())
{
	// Do something else
}
~~~~~~~~~~~~~

The system internally keeps track of all resources, so calling load multiple times in a row will return a previously loaded resource if one exists.

## Resource handles {#resources_a_a}
As we saw in the previous example resources are referenced by @ref BansheeEngine::ResourceHandleBase "resource handles". Different handles exist for each resource type, and they always being with a prefix "H" followed by the resource name, e.g. @ref BansheeEngine::ResourceHandleBase "HTexture".

Handles act like pointers, and you can access the resource within the handle by using the pointer member "->" operator. Using this operator is only valid if the handle points to a loaded resource (@ref BansheeEngine::ResourceHandleBase::isLoaded "HResource::isLoaded" returns true). Using the normal member access "." operator works on the handle data directly and is always safe.

Handles are used instead of pointers because:
 - Lifetime of the resource can be automatically managed
 - User can keep a reference to an unloaded resources
 - Are handled specially when serializing/deserializing objects
 
A little more elaboration for the last point. If you have read the [RTTI](@ref rtti) manual you know that objects can reference different field types. And resource handles can be referenced by "Reflectable" field types, allowing you to easily save/load references to resources within your objects with no additional code.

Each handle internally stores an UUID that uniquely identifies the resource. You can access it by calling @ref BansheeEngine::ResourceHandleBase::getUUID() "HResource::getUUID".

## Resource dependencies {#resource_a_b}
We mentioned in the last section that objects (including resources) can reference resources. For example a @ref BansheeEngine::Font "Font" resource will reference @ref BansheeEngine::Texture "Texture" resources which store its pre-rendered font glyphs.

When loading a resource you have a choice to load only that exact resource, or if you wish to load all of its dependencies as well. This is handled by the `loadDependencies` parameter of both @ref BansheeEngine::Resources::load "Resource::load" and @ref BansheeEngine::Resources::loadAsync "Resource::loadAsync". If you load a resource without dependencies, and need them later you can just call @ref BansheeEngine::Resources::load "Resource::load" or @ref BansheeEngine::Resources::loadAsync "Resource::loadAsync" on that same resource with `loadDependencies` set to true.

## Resource lifetime {#resource_a_c}
Once a resource is loaded it will stay loaded until all references (handles) to it go out of scope. Once the last handle goes out of scope the system will unload the resource.

If you wish to keep a handle to a resource that doesn't keep it loaded, you can use @ref BansheeEngine::TResourceHandle<T, true> "WeakResourceHandle<T>". You retrieve such a handle by calling @ref BansheeEngine::TResourceHandle<T, WeakHandle>::getWeak "HResource::getWeak" method on a normal handle. You can convert a weak handle into a normal handle by calling @ref BansheeEngine::TResourceHandle<T, WeakHandle>::lock "HResource::lock". Before doing so you should verify that the resource is still loaded.

Weak handles can also be used for referencing resources for serialization/deserialization, same as normal handles can. However a weak handle will never force the referenced resource to load, even if `loadDependencies` parameter is set to true when loading. You must call @ref BansheeEngine::Resources::load(const WeakResourceHandle<T>&, ResourceLoadFlags) "Resource::load" to manually load such a resource.

You can also force a resource to stay loaded, even after all the handles go out of scope. You do it by setting `keepInternalReference` of @ref BansheeEngine::Resources::load "Resource::load" or @ref BansheeEngine::Resources::loadAsync "Resource::loadAsync" to true. In order to free such a resource you should call @ref BansheeEngine::Resources::release "Resources::release". Be aware that each call to @ref BansheeEngine::Resources::load "Resource::load" or @ref BansheeEngine::Resources::loadAsync "Resource::loadAsync" with this parameter enabled will require a separate @ref BansheeEngine::Resources::release "Resources::release" call. @ref BansheeEngine::Resources::release "Resources::release" will not release the resource immediately if there are still active handles to it, it will merely remove the internal reference so the resource can be freed when the handles go out of scope.

You can also use @ref BansheeEngine::Resources::unloadAllUnused "Resources::unloadAllUnused" which will unload any resource that has no handles referencing it.

# Saving {#resources_b}
To save a resource to the disk call @ref BansheeEngine::Resources::save(const HResource&, const Path&, bool) "Resources::save". You need to provide a handle to the resource, and a path to save the resource to.

If a resource already exists on the disk (i.e. it was created by @ref BansheeEngine::Resources::load "Resource::load" instead of manually), you can instead call @ref BansheeEngine::Resources::save(const HResource&) "Resources::save" to overwrite the existing file.

An example:
~~~~~~~~~~~~~{.cpp}
HTexture texture = Texture::create(TEX_TYPE_2D, 1280, 720, 0, PF_R8G8B8A8);
... fill texture with some data ...

gResources().save(texture, "myTexture.asset");
~~~~~~~~~~~~~

# Creating your own resource {#resources_c}
To create your own resource, derive a class from @ref BansheeEngine::Resource "Resource" and:
 - Implement the @ref BansheeEngine::CoreObject "CoreObject" interface as described in the [core thread](@ref coreThread) manual
 - Implement a RTTI object as described in the [RTTI](@ref rtti) manual
 
Every resource is also a @ref BansheeEngine::CoreObject "CoreObject", but not every resource actually needs a core thread representation (for example a @ref BansheeEngine::Font "Font" is not relevant on the core thread). In such a case you can provide false to the `requiresGpuInitialization` parameter of the @ref BansheeEngine::CoreObject "CoreObject" constructor. This also means you can ignore implementing most of the @ref BansheeEngine::CoreObject "CoreObject" interface.

When creating resources, aside from the normal @ref BansheeEngine::CoreObject "CoreObject" creation requirements, you must also call @ref BansheeEngine::Resources::_createResourceHandle "Resources::_createResourceHandle". This method accepts a resource pointer and returns a resource handle. Similar to @ref BansheeEngine::CoreObject "CoreObject" initialization, this should probably be wrapped as a part of a static `create` method and hidden from the user.

See @ref BansheeEngine::Font "Font" implementation in `BsFont.cpp` for an example of a simple resource implementation.

## Exposing resources to scripting {#resources_c_a}
Once you have a custom resource you might want to expose it to the scripting API. Take a look at the [scripting interop guide](@ref scripting) to learn how to create scripting API objects. 

Every resource scripting interop object must implement @ref BansheeEngine::TScriptResource "TScriptResource", but other than that creating GUI interop objects is the same as the general case described in the guide above. See @ref BansheeEngine::ScriptFont "ScriptFont" in `BsScriptFont.cpp` for an example.

Creating managed GUI objects is again the same as the general case. Take a look at the managed `Font` implementation in `Font.cs` for an example.

Resources are unlike most other scripting objects because they can be created in the native code and then used in the managed code, or vice versa. Resources are also serialized by reference, which requires some specialized code for managed serialization to properly recognize the resource. 

### Native <-> managed resource conversion {#resources_c_a_a}
When a resource is created in native code, but needs to be passed to scripting code, the system creates a script interop object and a managed object, which wrap the native resource. The system also needs to be aware if such managed representation of an object already exists, and return an existing one if so. This is the job of the @ref BansheeEngine::ScriptResourceManager "ScriptResourceManager".

You will need to update this manager with your custom resource type. Relevant methods are:
 - @ref BansheeEngine::ScriptResourceManager::createScriptResource "ScriptResourceManager::createScriptResource" - Creates a new managed object instance from a native resource (only valid if one doesn't already exist). This method has two overloads, and is specialized for specific resource types. Make sure to add a specialization for both overloads, with your custom resource type. And also update the specialization that accepts a generic @ref BansheeEngine::ResourceHandleBase "HResource" with an entry of your own resource. You can use existing resources as an example of how it should be done, only the type names need changing.
 - @ref BansheeEngine::ScriptResourceManager::getScriptResource<RetType, InType> "ScriptResourceManager::getScriptResource" - Retrieves an existing managed object instance from a native resource handle. Optionally creates the managed object if one doesn't already exist. Add a specialization for your own resource type similar to methods above.

### Managed resource serialization {#resources_c_a_b}
In order for your new resource type to be properly referenced by serialized managed objects you need to make a few more additions:
 - First of append a new resource type to the @ref BansheeEngine::ScriptReferenceType "ScriptReferenceType" enum.
 - Append a new entry to @ref BansheeEngine::ManagedSerializableTypeInfoRef::getMonoClass "ManagedSerializableTypeInfoRef::getMonoClass" which maps the enum entry above to your resource script interop object type
 - Append a new entry to @ref BansheeEngine::ManagedSerializableTypeInfoRef::isTypeLoaded "ManagedSerializableTypeInfoRef::isTypeLoaded" so it always returns true if your resource is encountered (since it's built-in, it's always loaded)
 - Append a new entry in the @ref BansheeEngine::ScriptAssemblyManager::getTypeInfo "ScriptAssemblyManager::getTypeInfo" which maps the managed resource object type to the enum above
 - Append a new entry to the `getResourceFieldLookup` method in `BsManagedSerializableField.cpp` which uses the native <-> managed conversion methods to map between a native and a managed instance of a resource during serialization/deserialization
 
For all of these you can use examples of existing resources which already exist within the mentioned methods, it is just a matter of changing the type/enum names for your own resource. 
 
### Other {#resources_c_a_c}
In order for your resources to be properly recognized by GUI drop fields, as well as the project library append new entries to @ref BansheeEngine::ScriptResource::getClassFromTypeId "ScriptResource::getClassFromTypeId" and @ref BansheeEngine::ScriptResource::getTypeFromTypeId "ScriptResource::getTypeFromTypeId" which map between the enum above and the RTTI type ID of the resource.

# Advanced {#resources_d}
This section lists some more advanced (and optional) features when dealing with resources.

## Dependencies {#resources_d_a}
When creating a custom resource that is dependant on some other resource(s), make sure to implement @ref BansheeEngine::Resource::getResourceDependencies "Resource::getResourceDependencies" which returns a list of all resources it is dependant upon. These resources will be checked when @ref BansheeEngine::ResourceHandleBase::isLoaded "HResource::isLoaded" or @ref BansheeEngine::ResourceHandleBase::blockUntilLoaded "HResource::blockUntilLoaded" is called with `checkDependencies` parameter enabled.

This does not impact the dependencies that are loaded with @ref BansheeEngine::Resources::load "Resources::load". ALL referenced non-weak resource handles will be loaded regardless of @ref BansheeEngine::Resource::getResourceDependencies "Resource::getResourceDependencies" implementation.

## Resource listener {#resources_d_b}
If an object is interested in learning when a certain resource has been loaded, or updated you can implement the @ref BansheeEngine::IResourceListener "IResourceListener" interface. For example this can be used for refreshing a @ref BansheeEngine::Material "Material" if the @ref BansheeEngine::Shader "Shader" it is dependant upon changes. This also allows you to wait when initializing an object dependant on other resources, in case those resources are being loaded asynchronously.

To implement it you must return a list of resource handles you are interested in tracking by implementing the @ref BansheeEngine::IResourceListener::getListenerResources "IResourceListener::getListenerResources" method. Whenever the dependant resource list changes you should call @ref BansheeEngine::IResourceListener::markListenerResourcesDirty "IResourceListener::markListenerResourcesDirty".

After you have set up the listener resources, whenever such a resource is loaded @ref BansheeEngine::IResourceListener::notifyResourceLoaded "IResourceListener::notifyResourceLoaded" will be called, and when it is updated (via @ref BansheeEngine::Resources::update "Resources::update") @ref BansheeEngine::IResourceListener::notifyResourceChanged "IResourceListener::notifyResourceChanged" will be called.
 
@ref BansheeEngine::Resources "Resources" also provides @ref BansheeEngine::Resources::onResourceLoaded "Resources::onResourceLoaded", @ref BansheeEngine::Resources::onResourceDestroyed "Resources::onResourceDestroyed" and @ref BansheeEngine::Resources::onResourceModified "Resources::onResourceModified" events which may be used for similar purpose, but @ref BansheeEngine::IResourceListener "IResourceListener" is more efficient since it only tracks specific resources.

## Resource manifest {#resources_d_c}
Every time a resource is saved via @ref BansheeEngine::Resources::save "Resources::save", its UUID and file path is stored in a @ref BansheeEngine::ResourceManifest "ResourceManifest". This manifest is used primarily by the deserialization process, so it knows to find resource file paths from the resource handle UUIDs that were serialized.

Without this manifest every time a user saved an object referencing a resource (like a scene), then restarted the application the system would no longer know where to locate the referenced resources. When using the editor or the scripting API the manifest is managed for you, but if you are building an application on a lower level, you must manage it yourself.

Use @ref BansheeEngine::Resources::getResourceManifest "Resources::getResourceManifest" to retrieve a manifest. By default only one manifest with the name "Default" is registered. You will need to save this manifest before application shut-down and restore it upon application start-up. To restore a manifest call @ref BansheeEngine::Resources::registerResourceManifest "Resources::registerResourceManifest".

You can register multiple manifests and the system will scan all of them to find resource paths. If a resource exists in multiple ones, then the latest registered one takes priority. You can unregister a manifest by calling @ref BansheeEngine::Resources::unregisterResourceManifest "Resources::unregisterResourceManifest"
