---
title: Meshes
---

Meshes are used for defining surfaces of 2D and 3D objects, and are the primary building blocks of the scene. In the framework meshes are represented with the @b3d::Mesh class. A mesh is a resource, meaning it can be imported, saved and loaded as any other resource.

![Wireframe mesh](../../Images/DragonWireframe.png)

# Importing a mesh
Meshes can be imported from various third party formats, using the importer.

~~~~~~~~~~~~~{.cpp}
// Import a mesh named "dragon.fbx" from the disk
HMesh mesh = GetImporter().Import<Mesh>("dragon.fbx");
~~~~~~~~~~~~~

Supported formats are:
 - FBX
 - DAE (Collada)
 - OBJ
 - glTF / GLB

# Creating a mesh
Meshes can also be created manually, which we cover later in the [creating meshes](../12_Advanced_Rendering/06_creatingMeshes.md) manual.

# Mesh properties
Once a mesh has been imported, you can retrieve its properties like vertex & index counts, as well as its bounds by calling @b3d::Mesh::GetProperties, which returns a @b3d::MeshProperties object.

~~~~~~~~~~~~~{.cpp}
// Retrieve and print out various mesh properties
const auto& properties = mesh->GetProperties();

B3D_LOG(Info, LogGeneric, "Num. vertices: {0}", properties.VertexCount);
B3D_LOG(Info, LogGeneric, "Num. indices: {0}", properties.IndexCount);
B3D_LOG(Info, LogGeneric, "Radius: {0}", properties.Bounds.GetSphere().GetRadius());
~~~~~~~~~~~~~

> The debug logging functionality is explained in the [logging](../15_Utilities/07_logging.md) manual.

Additional mesh property information:

~~~~~~~~~~~~~{.cpp}
// Get number of sub-meshes
u32 subMeshCount = (u32)properties.SubMeshes.size();

// Get bounds
const Bounds& bounds = properties.Bounds;
AABox box = bounds.GetBox();
Sphere sphere = bounds.GetSphere();

// Check if mesh has skeleton (for skeletal animation)
if (mesh->GetSkeleton() != nullptr)
    B3D_LOG(Info, LogGeneric, "Mesh has skeleton with {0} bones",
        mesh->GetSkeleton()->GetNumBones());

// Check if mesh has morph shapes (for blend shape animation)
if (mesh->GetMorphShapes() != nullptr)
    B3D_LOG(Info, LogGeneric, "Mesh has {0} morph shapes",
        mesh->GetMorphShapes()->GetNumShapes());
~~~~~~~~~~~~~

# Customizing import
Mesh import can be customized by providing a @b3d::MeshImportOptions object to the importer.

~~~~~~~~~~~~~{.cpp}
auto importOptions = MeshImportOptions::Create();
// Set required options here (as described below)

HMesh mesh = GetImporter().Import<Mesh>("dragon.fbx", importOptions);
~~~~~~~~~~~~~

Lets see some of the options you can use for customizing import.

## Scale
@b3d::MeshImportOptions::ImportScale allows you to apply a uniform scale value to the mesh upon import. Although you can scale the size of a rendered mesh by adjusting the **SceneObject** transform when its placed in the scene, sometimes it is more useful to be able to do it once at import instead of every time you place it.

~~~~~~~~~~~~~{.cpp}
// Reduce the size of the mesh to 10% of its original size
importOptions->ImportScale = 0.1f;
~~~~~~~~~~~~~

## Normals
@b3d::MeshImportOptions::ImportNormals controls whether normal vectors are imported from the mesh file.

Normal vectors are used in lighting and are required for any meshes placed in the 3D scene (unless rendering them manually using some custom method). They allow the mesh to appear smooth even though its surface is made out of triangles.

Most 3D authoring tools generate normals for their meshes, but if normals are not present in the mesh file, the framework will attempt to generate normals automatically when this option is turned on.

~~~~~~~~~~~~~{.cpp}
// Import or generate normals for the mesh
importOptions->ImportNormals = true;
~~~~~~~~~~~~~

## Tangents
@b3d::MeshImportOptions::ImportTangents controls whether tangent vectors are imported from the mesh file.

Tangent vectors (along with normal vectors) are required if your rendering shader uses normal maps. Similar to normals, if tangents are not present in the mesh file, the framework will attempt to generate them automatically.

~~~~~~~~~~~~~{.cpp}
// Import or generate tangents for the mesh
importOptions->ImportTangents = true;
~~~~~~~~~~~~~

## Animation data
You can control whether skeletal animation, blend shapes, and animation clips are imported:

~~~~~~~~~~~~~{.cpp}
// Import skeleton data (bone weights, indices, bind poses)
importOptions->ImportSkin = true;

// Import blend shapes (morph targets)
importOptions->ImportBlendShapes = true;

// Import animation clips
importOptions->ImportAnimation = true;

// Enable keyframe reduction to reduce animation clip size
importOptions->ReduceKeyFrames = true;

// Import root motion as separate curves
importOptions->ImportRootMotion = true;
~~~~~~~~~~~~~

Animation import is covered in detail in the [Animation](../10_Animation/00_animationClip.md) manual.

## Collision mesh
You can import a collision mesh along with the visual mesh:

~~~~~~~~~~~~~{.cpp}
// Import a triangle mesh for collision
importOptions->CollisionMeshType = CollisionMeshType::Normal;

// Or import a convex hull
importOptions->CollisionMeshType = CollisionMeshType::Convex;

// Don't import collision mesh (default)
importOptions->CollisionMeshType = CollisionMeshType::None;
~~~~~~~~~~~~~

The collision mesh will be available as a separate resource after import. See the [Physics mesh](../09_Physics/01_physicsMesh.md) manual for more information.

## CPU caching
Sometimes you need to import a mesh you don't want to only use for rendering, but rather for manually reading its contents. When that's the case you can enable the @b3d::MeshImportOptions::CpuCached option.

This will allow you to call @b3d::Mesh::GetCachedData and to manually read individual vertices and indices of the mesh.

Note that caching a mesh means its data will be available in system memory, essentially doubling its memory usage.

~~~~~~~~~~~~~{.cpp}
// Enable caching
importOptions->CpuCached = true;

// Import mesh
HMesh mesh = GetImporter().Import<Mesh>("dragon.fbx", importOptions);

// Read cached data
TShared<MeshData> meshData = mesh->GetCachedData();

// Read vertex positions using an iterator
auto positionIterator = meshData->GetVec3DataIter(VES_POSITION);
while (positionIterator.MoveNext())
{
    Vector3 position = positionIterator.GetValue();
    B3D_LOG(Info, LogGeneric, "Vertex: ({0}, {1}, {2})",
        position.x, position.y, position.z);
}

// Or read all positions at once
u32 vertexCount = meshData->GetVertexCount();
Vector<Vector3> positions(vertexCount);
meshData->GetVertexData(VES_POSITION, positions.data(),
    vertexCount * sizeof(Vector3));
~~~~~~~~~~~~~

> **MeshData** is explained in detail in the [creating meshes](../12_Advanced_Rendering/06_creatingMeshes.md) manual.

# Reading mesh data
When you have CPU-cached mesh data, you can read various vertex attributes:

~~~~~~~~~~~~~{.cpp}
TShared<MeshData> meshData = mesh->GetCachedData();

// Get vertex count
u32 vertexCount = meshData->GetVertexCount();

// Get index count
u32 indexCount = meshData->GetIndexCount();

// Read positions
auto positionIterator = meshData->GetVec3DataIter(VES_POSITION);
for (u32 i = 0; i < vertexCount; i++)
{
    Vector3 position = positionIterator.GetValue();
    // Process position...
    positionIterator.MoveNext();
}

// Read normals
auto normIter = meshData->GetVec3DataIter(VES_NORMAL);
for (u32 i = 0; i < vertexCount; i++)
{
    Vector3 normal = normIter.GetValue();
    // Process normal...
    normIter.MoveNext();
}

// Read texture coordinates
auto uvIter = meshData->GetVec2DataIter(VES_TEXCOORD);
for (u32 i = 0; i < vertexCount; i++)
{
    Vector2 texCoord = uvIter.GetValue();
    // Process UV...
    uvIter.MoveNext();
}

// Read indices (32-bit)
if (meshData->GetIndexType() == IT_32BIT)
{
    u32* indices = meshData->GetIndices32();
    for (u32 i = 0; i < indexCount; i++)
    {
        u32 index = indices[i];
        // Process index...
    }
}
// Read indices (16-bit)
else
{
    u16* indices = meshData->GetIndices16();
    for (u32 i = 0; i < indexCount; i++)
    {
        u16 index = indices[i];
        // Process index...
    }
}
~~~~~~~~~~~~~

Available vertex element semantics include:
- **VES_POSITION** - Vertex position (Vector3)
- **VES_NORMAL** - Vertex normal (Vector3)
- **VES_TANGENT** - Vertex tangent (Vector4)
- **VES_TEXCOORD** - Texture coordinates (Vector2)
- **VES_COLOR** - Vertex color (u32 as RGBA)
- **VES_BLEND_WEIGHTS** - Bone weights for skinning (Vector4)
- **VES_BLEND_INDICES** - Bone indices for skinning (u32)

# Updating mesh data
You can also write to mesh data and update the GPU buffers:

~~~~~~~~~~~~~{.cpp}
// Allocate mesh data matching the mesh format
TShared<MeshData> meshData = mesh->AllocBuffer();

// Modify vertex positions
auto positionIterator = meshData->GetVec3DataIter(VES_POSITION);
for (u32 i = 0; i < meshData->GetVertexCount(); i++)
{
    Vector3 position = positionIterator.GetValue();
    position.y += 10.0f; // Move all vertices up
    positionIterator.SetValue(position);
    positionIterator.MoveNext();
}

// Write the modified data back to the mesh
mesh->WriteData(meshData, false);
~~~~~~~~~~~~~

> **Note:** `WriteData()` is an asynchronous operation. The mesh will be updated on the next frame.

# Using builtin meshes
For prototyping and testing, you can use builtin meshes instead of importing:

~~~~~~~~~~~~~{.cpp}
// Get a builtin sphere mesh
HMesh sphereMesh = GetBuiltinResources().GetBuiltinMesh(BuiltinMesh::Sphere);

// Other available builtin meshes:
HMesh boxMesh = GetBuiltinResources().GetBuiltinMesh(BuiltinMesh::Box);
HMesh coneMesh = GetBuiltinResources().GetBuiltinMesh(BuiltinMesh::Cone);
HMesh cylinderMesh = GetBuiltinResources().GetBuiltinMesh(BuiltinMesh::Cylinder);
HMesh quadMesh = GetBuiltinResources().GetBuiltinMesh(BuiltinMesh::Quad);
HMesh discMesh = GetBuiltinResources().GetBuiltinMesh(BuiltinMesh::Disc);
~~~~~~~~~~~~~

Builtin meshes are immediately available and don't require importing or loading.

# Calculating bounds
You can calculate the bounds of mesh data:

~~~~~~~~~~~~~{.cpp}
TShared<MeshData> meshData = mesh->GetCachedData();

// Calculate bounds from all vertices
Bounds bounds = meshData->CalculateBounds();

// Get axis-aligned bounding box
AABox box = bounds.GetBox();
Vector3 min = box.GetMin();
Vector3 max = box.GetMax();
Vector3 center = box.GetCenter();

// Get bounding sphere
Sphere sphere = bounds.GetSphere();
Vector3 sphereCenter = sphere.GetCenter();
float radius = sphere.GetRadius();
~~~~~~~~~~~~~
