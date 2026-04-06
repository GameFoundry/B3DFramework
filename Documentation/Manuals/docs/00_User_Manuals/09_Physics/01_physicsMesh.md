---
title: Physics meshes
---

Physics meshes are represented using the @b3d::PhysicsMesh class. They are resources, meaning they can be imported, saved and loaded as any other resource.

![Physics mesh](../../Images/MeshCollider.png)  

# Import
The import process is slightly different from other resources because physics meshes are imported from the same file types as normal meshes. First create a **MeshImportOptions** object, and specify that you wish to import a physics mesh along with a normal mesh by setting @b3d::MeshImportOptions::CollisionMeshType.

~~~~~~~~~~~~~{.cpp}
SPtr<MeshImportOptions> importOptions = MeshImportOptions::Create();
importOptions->CollisionMeshType = CollisionMeshType::Normal;
~~~~~~~~~~~~~

To perform the actual import, call @b3d::Importer::ImportAll instead of **Importer::Import<T>()** we have been calling so far. **Importer::ImportAll()** should be used when the import operation can return more than one resource (since **Importer::Import<T>()** will only return the default resource, i.e. a **Mesh**).

**Importer::ImportAll()** will return a @b3d::MultiResource object containing an array of @b3d::SubResource objects. In our case there are two sub-resources: the first one is the normal **Mesh**, and the second one will be the physics mesh we requested.

~~~~~~~~~~~~~{.cpp}
SPtr<MultiResource> resources = GetImporter().ImportAll("dragon.fbx", importOptions);

HPhysicsMesh physicsMesh = B3DStaticResourceCast<PhysicsMesh>(resources->Entries[1].Value);
~~~~~~~~~~~~~

Query physics mesh properties:

~~~~~~~~~~~~~{.cpp}
// Get the mesh type
PhysicsMeshType meshType = physicsMesh->GetType();

// Get the mesh data (vertices and indices)
SPtr<MeshData> meshData = physicsMesh->GetMeshData();
~~~~~~~~~~~~~

> In case there are multiple resources and you're not sure which one is the physics mesh, you can check the **Name** field on the **SubResource** object. Physics meshes are always named "collision".

# Performance
Note that performing physics tests is expensive and you should always strive to minimize the number of triangles in a physics mesh. Using a render mesh for physics is almost never a good idea. Instead, aim to have a few dozen triangles in your physics mesh, with a few hundred for the most complex geometry. Whenever possible, use the basic collider shapes like box or sphere instead of physics meshes.

# Physics mesh types
During import when setting **MeshImportOptions::CollisionMeshType**, we provide a @b3d::CollisionMeshType enum to specify which type of mesh to import. It has three possible values:
 - **CollisionMeshType::None** - No collision mesh will be imported
 - **CollisionMeshType::Normal** - Normal triangle mesh will be imported
 - **CollisionMeshType::Convex** - A convex hull will be generated from the source mesh

Normal (triangle) meshes are imported as-is. They can represent very complex geometry, but such meshes cannot be used for dynamic physical objects (shown later). Instead, such meshes can only be used for static scene geometry.

Convex meshes, on the other hand, can be used for dynamic physical objects. They are also significantly faster to process by the physics system, and you should strive to use them whenever possible. They do come with two major restrictions:
 - They can have a maximum of 256 vertices
 - They must be convex (have no holes or indentations, and be closed)

When the convex mesh option is enabled, the system will automatically convert any non-convex meshes to convex ones on import. This process is also known as "gift-wrapping", as the resulting convex object looks as if you gift wrapped the original object.

The resulting **PhysicsMesh** will have one of two @b3d::PhysicsMeshType values:
 - **PhysicsMeshType::Triangle** - A regular triangle mesh that can be of arbitrary size but cannot be used for triggers and non-kinematic objects
 - **PhysicsMeshType::Convex** - A convex mesh that will not have more than 256 vertices and can be used with dynamic objects

# Creating physics meshes manually

You can also create physics meshes programmatically by providing mesh data directly:

~~~~~~~~~~~~~{.cpp}
// Assuming you have vertex and index data
SPtr<MeshData> meshData = MeshData::Create(vertexCount, indexCount, vertexLayout);
// ... populate mesh data with vertices and indices ...

// Create a convex physics mesh
HPhysicsMesh convexMesh = PhysicsMesh::Create(meshData, PhysicsMeshType::Convex);

// Or create a triangle mesh
HPhysicsMesh triangleMesh = PhysicsMesh::Create(meshData, PhysicsMeshType::Triangle);
~~~~~~~~~~~~~
 
