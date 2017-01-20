Physics meshes						{#physicsMesh}
===============

Physics meshes are represented using the @ref bs::PhysicsMesh "PhysicsMesh" class. They are resources, meaning they can be imported, saved and loaded as we described in the Resource manuals.

![Physics mesh](MeshCollider.png)  

# Import
The import process is slightly different from other resources because physics meshes are imported from the same file types as normal meshes. We must first create **MeshImportOptions** object, and specify that we wish to import a physics mesh along with a normal mesh by calling @ref bs::MeshImportOptions::setCollisionMeshType "MeshImportOptions::setCollisionMeshType()".

~~~~~~~~~~~~~{.cpp}
auto importOptions = MeshImportOptions::create();
importOptions->setCollisionMeshType(CollisionMeshType::Normal);
~~~~~~~~~~~~~

Then in order to perform actual import we call @ref bs::Importer::importAll "Importer::importAll()", instead of **Importer::import<T>()** we have been calling so far. **Importer::importAll()** should be used when import operation can return more than one resource since **Importer::import<T>()** will only return the default resource (i.e. a **Mesh**).

**Importer::importAll()** will return an array of resources as @ref bs::SubResource "SubResource" objects. In our case there are two sub resources, first one is the normal **Mesh**, and the second one will be the physics mesh we requested.

~~~~~~~~~~~~~{.cpp}
auto resources = gImporter().importAll("dragon.fbx", importOptions);

HPhysicsMesh physicsMesh = static_resource_cast<PhysicsMesh>(resources[1].value);
~~~~~~~~~~~~~

> In case there are multiple resources and you're not sure which one is the physics mesh you can check the **name** field on the **SubResource** object. Physics meshes are always named "collision".

# Performance
Note that performing physics tests is expensive and you should always strive to minimize the amount of triangles in a physics mesh. Using a render mesh for physics is almost never a good idea. Instead strive to have a few dozen triangles in your physics mesh, with a few hundred for the most complex geometry. Whenever possible use the basic collider shapes like box/sphere instead of physics meshes.

# Physics mesh types
During import when calling **MeshImportOptions::setCollisionMeshType()** we provided a @ref bs::CollisionMeshType "CollisionMeshType" enum to specify we want a mesh imported. It has two possible values:
 - **CollisionMeshType::Normal**
 - **CollisionMeshType::Convex**
 
Normal meshes are imported as-is. They can represent very complex geometry, but such meshes cannot be used for dynamic physical objects (shown later). Instead such meshes can only be used for static scene geometry.

Convex meshes on the other hand can be used for dynamic physical objects. They are also significantly faster to process by the physics system and you should strive to use them whenever possible. They do come with two major restrictions though:
 - They can have a maximum of 255 faces
 - They must be convex (have no holes or indentations, and be closed)

When convex mesh option is enabled the system will automatically convert any non-convex meshes to convex ones on import. This process is also known as "gift-wrapping", as the resulting convex object looks like as if you gift wrapped the original object.

@ref TODO_IMAGE 
 