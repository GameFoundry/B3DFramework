---
title: Resource packages
---

Packages are container files that store one or more resources in an efficient, compressed format. They are the primary way to distribute and organize game assets.

> **Note:** Packages and @b3d::PackageManager are low-level concepts that are typically managed automatically by the engine. For most use cases, you should use the higher-level @b3d::Resources API to load resources without needing to manage packages directly. This manual is useful if you need fine-grained control over package loading or are building tools that work with the package system.

# What are packages?
A **package** is a single file (with `.b3d` extension) that can contain multiple resources along with their metadata. Packages provide several benefits:

- **Organization**: Group related resources together
- **Performance**: Resources in packages load faster than individual files
- **Compression**: Packages can compress resources to reduce disk space
- **Distribution**: Easier to distribute game assets as fewer, larger files

# Package paths
Resources in packages can be accessed using two types of paths:

## Physical paths
A physical path is the actual file system path to the package and resource:
```
D:/MyGame/Assets/Textures.b3d/BrickAlbedo
```

This consists of:
- Package path: `D:/MyGame/Assets/Textures.b3d`
- Resource path within package: `BrickAlbedo`

## Virtual paths
A virtual path is a logical path that maps to a physical location:
```
/Game/Textures/BrickAlbedo
```

Virtual paths are more flexible - you can move packages to different locations without breaking references, as long as you maintain the same virtual path mapping.

# The PackageManager
The @b3d::PackageManager module (accessible via @b3d::GetPackageManager()) handles all package operations. It manages package loading, saving, and provides the mapping between virtual and physical paths.

## Loading packages
Before you can load resources from a package, you must load the package itself using @b3d::PackageManager::LoadOrGetPackage:

~~~~~~~~~~~~~{.cpp}
// Load a package from disk
auto packageLock = GetPackageManager().LoadOrGetPackage("D:/MyGame/Assets/Textures.b3d");

if (packageLock != nullptr)
{
	// Package loaded successfully
	// The packageLock keeps the package loaded
}
~~~~~~~~~~~~~

> The returned @b3d::PackageReadLock must be kept alive as long as you're accessing the package. When it goes out of scope, the package may be unloaded if no other locks exist.

### Virtual path prefix
When loading a package, you can specify a virtual path prefix that will be used for all resources in the package:

~~~~~~~~~~~~~{.cpp}
// Load package with virtual path prefix
auto packageLock = GetPackageManager().LoadOrGetPackage(
	"D:/MyGame/Assets/Textures.b3d",
	"/Game/Textures/"  // Virtual path prefix
);

// Now resources can be loaded using virtual paths
HTexture texture = GetResources().Load<Texture>("/Game/Textures/BrickAlbedo");
~~~~~~~~~~~~~

### Loading multiple packages
You can load all packages in a folder using @b3d::PackageManager::LoadPackages:

~~~~~~~~~~~~~{.cpp}
// Load all packages in the Assets folder and its subfolders
GetPackageManager().LoadPackages(
	"D:/MyGame/Assets/",          // Folder path
	true,                          // Recursive (search subfolders)
	"/Game/",                      // Virtual path prefix
	true                           // Add subfolder names to virtual path
);

// With addSubFoldersToVirtualPath = true:
// - D:/MyGame/Assets/Textures.b3d -> /Game/Textures/
// - D:/MyGame/Assets/Audio.b3d -> /Game/Audio/
~~~~~~~~~~~~~

## Unloading packages
Packages can be unloaded using @b3d::PackageManager::UnloadPackage:

~~~~~~~~~~~~~{.cpp}
GetPackageManager().UnloadPackage("D:/MyGame/Assets/Textures.b3d");
~~~~~~~~~~~~~

> Be careful when unloading packages - any resources loaded from that package will become invalid. Make sure all resources are unloaded first, or that no code is holding references to them.

# Saving packages
The @b3d::PackageManager provides advanced control over package saving. While @b3d::Resources::SaveAsSinglePackage is convenient for single resources, @b3d::PackageManager::SavePackage allows you to create packages with multiple resources.

## Creating multi-resource packages
To create a package with multiple resources, you need to work with the @b3d::Package class directly:

~~~~~~~~~~~~~{.cpp}
// Create a new package
TShared<Package> package = B3DMakeShared<Package>();

// Add resources to the package
package->AddResource("BrickAlbedo", brickAlbedoTexture);
package->AddResource("BrickNormal", brickNormalTexture);
package->AddResource("BrickRoughness", brickRoughnessTexture);

// Save the package
PackageManagerSavePackageOptions saveOptions;
saveOptions.Overwrite = true;
saveOptions.Compress = true;
saveOptions.VirtualPathPrefix = "/Game/Textures/Brick/";

auto writeLock = GetPackageManager().SavePackage(
	package,
	"D:/MyGame/Assets/BrickTextures.b3d",
	saveOptions
);
~~~~~~~~~~~~~

All three textures can now be loaded using their virtual paths:
- `/Game/Textures/Brick/BrickAlbedo`
- `/Game/Textures/Brick/BrickNormal`
- `/Game/Textures/Brick/BrickRoughness`

## Save options
@b3d::PackageManagerSavePackageOptions provides control over the save operation:

~~~~~~~~~~~~~{.cpp}
PackageManagerSavePackageOptions saveOptions;
saveOptions.Overwrite = true;                    // Overwrite existing package
saveOptions.Compress = true;                      // Use compression
saveOptions.CopyLoadStatesOnOverwrite = false;   // Preserve load states when overwriting
saveOptions.VirtualPathPrefix = "/Game/Assets/"; // Virtual path for resources
saveOptions.MetaDataPaddingByteCount = 1024;     // Reserve space for metadata updates
~~~~~~~~~~~~~

The **MetaDataPaddingByteCount** is useful for packages you expect to update frequently - it reserves extra space so you can update package metadata without rewriting the entire package.

# Package locking
Package operations use a locking mechanism to ensure thread safety. There are two types of locks:

## Read locks
A **read lock** (@b3d::PackageReadLock) allows reading from a package while preventing writes. Multiple read locks can exist simultaneously:

~~~~~~~~~~~~~{.cpp}
AcquirePackageReadLockOptions options;
options.LoadIfMissing = true;              // Load package if not loaded
options.BlockUntilAcquired = true;         // Wait if write lock exists
options.VirtualPathPrefix = "/Game/";      // Virtual path prefix

UPtr<PackageReadLock> readLock;
AcquirePackageLockResult result = GetPackageManager().AcquireReadLock(
	"D:/MyGame/Assets/Textures.b3d",
	options,
	readLock
);

if (result == AcquirePackageLockResult::Acquired)
{
	// Can now read from the package
	const TShared<Package>& package = readLock->GetPackage();
}
~~~~~~~~~~~~~

## Write locks
A **write lock** (@b3d::PackageWriteLock) allows modifying a package. Only one write lock can exist, and no read locks can exist while it's held:

~~~~~~~~~~~~~{.cpp}
AcquirePackageWriteLockOptions options;
options.AllowCreateNew = false;      // Fail if package doesn't exist
options.BlockUntilAcquired = true;   // Wait for existing locks

UPtr<PackageWriteLock> writeLock;
AcquirePackageLockResult result = GetPackageManager().AcquireWriteLock(
	"D:/MyGame/Assets/Textures.b3d",
	options,
	writeLock
);

if (result == AcquirePackageLockResult::Acquired)
{
	// Can now modify the package
	const TShared<Package>& package = writeLock->GetPackage();
}
~~~~~~~~~~~~~

# Updating packages
Packages can be updated in place without creating a new file:

## Updating package metadata
If you need to update package metadata (like resource information) without changing the actual resource data, use @b3d::PackageManager::SavePackageMetaData:

~~~~~~~~~~~~~{.cpp}
UPtr<PackageWriteLock> writeLock;
GetPackageManager().AcquireWriteLock("D:/MyGame/Assets/Textures.b3d", options, writeLock);

// Modify package metadata...

// Save just the metadata (fast operation)
if (!GetPackageManager().SavePackageMetaData(*writeLock))
{
	// Failed - metadata doesn't fit in reserved space
	// Need to perform full save instead
}
~~~~~~~~~~~~~

This is much faster than saving the entire package, but only works if the metadata fits in the space reserved by **MetaDataPaddingByteCount**.

## Changing package paths
You can change where a package is stored or how it's accessed:

~~~~~~~~~~~~~{.cpp}
// Move package to new physical location
GetPackageManager().ChangePhysicalPackagePath(*writeLock, "D:/NewLocation/Textures.b3d");

// Change virtual path prefix
GetPackageManager().ChangeVirtualPackagePath(*writeLock, "/NewGame/Textures/");
~~~~~~~~~~~~~

# Path resolution
The PackageManager provides utilities for resolving between physical and virtual paths:

~~~~~~~~~~~~~{.cpp}
// Resolve virtual path to physical package path
TOptional<ResourcePackagePath> packagePath =
	GetPackageManager().TryResolveVirtualResourcePath("/Game/Textures/BrickAlbedo");

if (packagePath)
{
	B3D_LOG(Info, LogGeneric, "Package: {0}", packagePath->PhysicalPackagePath);
	B3D_LOG(Info, LogGeneric, "Resource: {0}", packagePath->ResourcePathWithinPackage);
}

// Resolve physical path to package components
packagePath = GetPackageManager().TryResolvePhysicalResourcePath(
	"D:/MyGame/Assets/Textures.b3d/BrickAlbedo"
);

// Find package containing a specific resource
TOptional<Path> packagePath2 = GetPackageManager().TryGetPackagePathForResource(resourceId);
~~~~~~~~~~~~~

# Resource metadata
Each resource in a package has associated metadata that can be retrieved without loading the resource:

~~~~~~~~~~~~~{.cpp}
// Get resource metadata
UUID resourceId = texture.GetId();
TShared<const PackageResourceMetaData> metadata =
	GetPackageManager().GetResourceMetaData(resourceId);

if (metadata)
{
	// Access metadata information
	// (Specific metadata fields depend on resource type)
}
~~~~~~~~~~~~~