---
title: Resource saving and loading
---

All resource save and load operations are managed through the @b3d::Resources module, accessible through @b3d::GetResources().

# Saving resources
Once a resource has been imported you can save it into a package for later use. The advantage of saving a resource (instead of importing it every time) is performance - resource import is usually a costly operation. Saved resources remain in an engine-friendly format and can be quickly loaded later.

Resources are saved into **packages**, which are container files that can hold one or more resources. To save a resource, you use @b3d::Resources::SaveAsSinglePackage, which creates a package containing a single resource:

~~~~~~~~~~~~~{.cpp}
// Import a texture named "myTexture.jpg" from the disk
HTexture texture = GetImporter().Import<Texture>("myTexture.jpg");

// Save the texture into a package
GetResources().SaveAsSinglePackage(texture, "D:/MyGame/Assets/", "BrickTexture");
~~~~~~~~~~~~~

This will create a package file at `D:/MyGame/Assets/BrickTexture.b3d` containing the texture resource.

> Note that resources can also be created within the engine programmatically, and don't necessarily have to be imported. For example, you can create textures with custom pixel data or meshes with procedural geometry, then save the resource in this same manner.

## Save options
You can customize the save operation by providing @b3d::ResourceSaveOptions:

~~~~~~~~~~~~~{.cpp}
ResourceSaveOptions saveOptions;
saveOptions.Overwrite = true;  // Overwrite if file exists
saveOptions.Compress = true;   // Use compression
saveOptions.VirtualPathPrefix = "/Game/Textures/";  // Virtual path prefix

GetResources().SaveAsSinglePackage(texture, "D:/MyGame/Assets/", "BrickTexture", saveOptions);
~~~~~~~~~~~~~

The **VirtualPathPrefix** is important - it allows you to load the resource using a virtual path instead of the physical file path (more on this in the packages manual).

# Loading resources
Once a resource has been saved into a package, you can load it at any time using @b3d::Resources::Load. Resources can be loaded using either a physical path or a virtual path.

## Loading by physical path
A physical path is the actual file system path to the package and resource:

~~~~~~~~~~~~~{.cpp}
// Load texture by physical path: packagePath/resourceName
HTexture texture = GetResources().Load<Texture>("D:/MyGame/Assets/BrickTexture.b3d/BrickTexture");
~~~~~~~~~~~~~

## Loading by virtual path
If you saved the resource with a virtual path prefix, you can load it using the virtual path:

~~~~~~~~~~~~~{.cpp}
// Load texture by virtual path (if saved with /Game/Textures/ prefix)
HTexture texture = GetResources().Load<Texture>("/Game/Textures/BrickTexture");
~~~~~~~~~~~~~

## Load options
You can customize the load operation using @b3d::ResourceLoadOptions:

~~~~~~~~~~~~~{.cpp}
ResourceLoadOptions loadOptions;
loadOptions.AsynchronousLoad = true;       // Load asynchronously
loadOptions.LoadDependencies = true;        // Load resource dependencies
loadOptions.KeepInternalReference = true;   // Keep internal reference

HTexture texture = GetResources().Load<Texture>("D:/MyGame/Assets/BrickTexture.b3d/BrickTexture", loadOptions);
~~~~~~~~~~~~~

> If you attempt to load a resource that has already been loaded, the system will return the existing resource instead of loading it again.

# Asynchronous loading
Resources can be loaded asynchronously (in the background) by setting the **AsynchronousLoad** option to true (which is the default). The returned handle will be valid immediately, but the resource data may not be loaded yet.

You can check if a resource is loaded by calling @b3d::ResourceHandle::IsLoaded:

~~~~~~~~~~~~~{.cpp}
ResourceLoadOptions loadOptions;
loadOptions.AsynchronousLoad = true;
HMesh mesh = GetResources().Load<Mesh>("myMesh.b3d/Mesh", loadOptions);

if (mesh.IsLoaded())
{
	// Resource is loaded and ready to use
}
~~~~~~~~~~~~~

You can block the current thread until the resource is loaded by calling @b3d::ResourceHandle::BlockUntilLoaded:

~~~~~~~~~~~~~{.cpp}
mesh.BlockUntilLoaded();
// Mesh is now guaranteed to be loaded
~~~~~~~~~~~~~

> Note that not-yet-loaded resource handles can be provided to some engine systems. Generally a system will note in its documentation if it works with such resource handles. For example, you can assign a not-yet-loaded texture to a material, and it will automatically be used once loaded.

# Resource lifetime
When you load a resource, that resource will be kept loaded until all references to it are lost. Each resource handle (e.g. **HMesh**) represents a single reference. Additionally, an "internal" reference is automatically created and held by the resource system (if **KeepInternalReference** is true, which is the default).

This internal reference ensures the resource stays loaded even when all external handles are destroyed. This is useful for resources you want to keep in memory for quick access.

## Releasing internal references
To allow a resource to be unloaded when all external handles are destroyed, you must release the internal reference by calling @b3d::Resources::ReleaseInternalReference:

~~~~~~~~~~~~~{.cpp}
HMesh mesh = GetResources().Load<Mesh>("myMesh.b3d/Mesh");

// Use the mesh...

// Release the internal reference
GetResources().ReleaseInternalReference(mesh);

// Now when 'mesh' goes out of scope, the resource will be unloaded
~~~~~~~~~~~~~

> Note that if you call **Resources::Load()** multiple times for the same resource, you must also call **Resources::ReleaseInternalReference()** the same number of times to fully release the internal reference.

Alternatively, you can load the resource without creating an internal reference in the first place:

~~~~~~~~~~~~~{.cpp}
ResourceLoadOptions loadOptions;
loadOptions.KeepInternalReference = false;

HMesh mesh = GetResources().Load<Mesh>("myMesh.b3d/Mesh", loadOptions);
// Resource will be unloaded when all handles are destroyed
~~~~~~~~~~~~~

## Unloading all unused resources
Instead of manually releasing internal references, you can unload all resources that aren't externally referenced by calling @b3d::Resources::UnloadAllUnused:

~~~~~~~~~~~~~{.cpp}
// Unload all resources that don't have external references
GetResources().UnloadAllUnused();
~~~~~~~~~~~~~

This is useful for cleaning up resources after a level change or when you want to free memory.

# Weak handles
In case you want to keep a reference to a resource without incrementing the reference count, you can use a weak handle. Weak handles are represented by the @b3d::TWeakResourceHandle class and can be retrieved from normal handles by calling the **GetWeak()** method:

~~~~~~~~~~~~~{.cpp}
// Load a mesh and store a handle as normal
HMesh mesh = GetResources().Load<Mesh>("myMesh.b3d/Mesh");

// Create a weak handle
TWeakResourceHandle<Mesh> weakMesh = mesh.GetWeak();

// Weak handle doesn't prevent the resource from being unloaded
// To use the resource, you need to lock the weak handle back to a strong handle:
HMesh strongMesh = weakMesh.Lock();
if (strongMesh != nullptr)
{
	// Resource is still loaded, safe to use
}
~~~~~~~~~~~~~

# Resource dependencies
When you load a resource, the system will automatically enumerate all dependencies of that resource and attempt to load them as well. For example, when loading a **Material** it will automatically load its **Shader** and any referenced **Texture** resources.

This behavior is controlled by the **LoadDependencies** option (enabled by default):

~~~~~~~~~~~~~{.cpp}
ResourceLoadOptions loadOptions;
loadOptions.LoadDependencies = false;  // Don't load dependencies

HMaterial material = GetResources().Load<Material>("myMaterial.b3d/Material", loadOptions);
// Material is loaded, but its textures and shader are not
~~~~~~~~~~~~~

# Checking resource existence
You can check if a resource exists at a given path without loading it using @b3d::Resources::Exists:

~~~~~~~~~~~~~{.cpp}
if (GetResources().Exists("myMesh.b3d/Mesh"))
{
	// Resource exists, safe to load
	HMesh mesh = GetResources().Load<Mesh>("myMesh.b3d/Mesh");
}
~~~~~~~~~~~~~

# Resource events
The Resources system provides events you can subscribe to for monitoring resource state:

~~~~~~~~~~~~~{.cpp}
// Called when a resource has been loaded
GetResources().OnResourceLoaded.Connect([](const HResource& resource)
{
	B3D_LOG(Info, LogGeneric, "Resource loaded: {0}", resource.GetId());
});

// Called when a resource has been destroyed
GetResources().OnResourceDestroyed.Connect([](const UUID& resourceId)
{
	B3D_LOG(Info, LogGeneric, "Resource destroyed: {0}", resourceId);
});

// Called when a resource has been modified
GetResources().OnResourceModified.Connect([](const HResource& resource)
{
	B3D_LOG(Info, LogGeneric, "Resource modified: {0}", resource.GetId());
});
~~~~~~~~~~~~~
