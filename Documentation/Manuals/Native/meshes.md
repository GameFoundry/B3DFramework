Meshes									{#meshes}
===============
[TOC]

Mesh is an object represented by a set of points, their properties and a set of primitives formed by those points. In Banshee mesh is represented with the @ref BansheeEngine::Mesh "Mesh" and @ref BansheeEngine::MeshCore "MeshCore" classes. Both of these provide almost equivalent functionality, but the former is for use on the simulation thread, and the latter is for use on the core thread. If you are confused by the dual nature of the objects, read the [core thread](@ref coreThread) manual. 

We're going to focus on the simulation thread implementation in this manual, and then note the differences in the core thread version at the end.

# Creating a mesh {#meshes_a}
To create a mesh call @ref BansheeEngine::Mesh::create "Mesh::create" or one if its overloads. You'll need to populate the @ref BansheeEngine::MESH_DESC "MESH_DESC" structure and pass it as a parameter. At minimum you need to provide a @ref BansheeEngine::VertexDataDesc "vertex description" that describes in what format are the vertices stored in the vertex buffer and the number of vertices and indices. 

Optionally you can also provide the type of indices, type of primitives, and a set of sub-meshes:
 - Indices can be 32- or 16-bit. This is controlled by the `indexType` parameter. Smaller indices can be used to save bandwidth if your mesh has a small enough number of primitives.
 - `drawOp` parameter determines how are the indices interpreted. By default they're interpreted as a list of triangles (every three indices forming a triangle, one after another) but different @ref BansheeEngine::DrawOperationType "types" are supported.
 - A mesh can have multiple sub-meshes. Each sub-mesh is described by an offset and a range of indices that belong to it. Sub-meshes can be used for rendering sections of a mesh, instead of all of it (for example if a single mesh uses different materials).
 
For example to create a simple mesh:
~~~~~~~~~~~~~{.cpp}
// Creates an empty mesh with 36 indices and 8 vertices
MESH_DESC meshDesc;
meshDesc.numVertices = 8;
meshDesc.numIndices = 36;

SPtr<VertexDataDesc> vertexDesc = ...; // Vertex description creation is explained later
meshDesc.vertexDesc = vertexDesc;

HMesh mesh = Mesh::create(meshDesc);
~~~~~~~~~~~~~ 
 
You can also create a non-empty mesh by creating it with a populated @ref BansheeEngine::MeshData "MeshData" object. More about @ref BansheeEngine::MeshData "MeshData" later. 
 
# Accessing properties {#meshes_b} 
You can access various mesh properties by calling @ref BansheeEngine::Mesh::getProperties() "Mesh::getProperties()" which will return an instance of @ref BansheeEngine::MeshProperties "MeshProperties" which contains information such as mesh bounds, number of indices/vertices and sub-meshes. 
 
# Reading/writing {#meshes_c}
To read and write from/to the mesh use the @ref BansheeEngine::Mesh::readSubresource "Mesh::readSubresource" and @ref BansheeEngine::Mesh::writeSubresource "Mesh::writeSubresource" methods. These expect an index of a sub-resource to read/write to, and a @ref BansheeEngine::MeshData "MeshData" object.

The sub-resource index for a mesh is currently ignored and should be always left as zero.

@ref BansheeEngine::MeshData "MeshData" object is just a container for a set of vertices and indices. You can create one manually or use @ref BansheeEngine::Mesh::allocateSubresourceBuffer "Mesh::allocateSubresourceBuffer" to create the object of valid size and format for the specified mesh. When reading from the mesh the buffer will be filled with vertices/indices from the mesh, and when writing you are expected to populate the object.

## MeshData {#meshes_c_a}
You can create @ref BansheeEngine::MeshData "MeshData" manually by calling @ref BansheeEngine::MeshData::create "MeshData::create" and providing it with vertex description, index type and number of vertices and indices. You must ensure that the formats and sizes matches the mesh this will be used on.

### Vertex description {#meshes_c_a_a}
Creating a vertex description is simple, it requires you to specify a list of vertex properties, each identified by a type and a semantic. The semantic determines to which GPU program input field does the property map to. See the [gpu program](@ref gpuPrograms) manual for more information about semantics. For example:
~~~~~~~~~~~~~{.cpp}
// Create a vertex with a position, normal and UV coordinates
SPtr<VertexDataDesc> vertexDesc = VertexDataDesc::create();
vertexDesc->addVertElem(VET_FLOAT3, VES_POSITION);
vertexDesc->addVertElem(VET_FLOAT3, VES_NORMAL);
vertexDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD);
~~~~~~~~~~~~~ 

See @ref BansheeEngine::VertexElementType "VertexElementType" for valid types, and @ref BansheeEngine::VertexElementSemantic "VertexElementSemantic" for a list of valid semantics. Each semantic can also optionally have an index in case multiple values of the same semantic are needed.

Each vertex element can also be placed in a "stream". In the example above all the elements are in the 0th stream, but you can place different elements in different streams by providing a stream index to @ref BansheeEngine::VertexDataDesc::addVertElem "VertexDataDesc::addVertElem". Different streams will result in multiple vertex buffers created within the mesh. This can be useful if you are rendering the mesh with multiple GPU programs and not every GPU program requires all vertex elements, in which case you could only bind a subset of the vertex buffers and reduce the bandwidth costs by not sending the unnecessary data.

Once the @ref BansheeEngine::VertexDataDesc "VertexDataDesc" structure has been filled, you can use it for initializing a @ref BansheeEngine::Mesh "Mesh" or @ref BansheeEngine::MeshData "MeshData". You can also use it to manually create a vertex declaration by calling @ref BansheeEngine::VertexDeclaration::create "VertexDeclaration::create". @ref BansheeEngine::VertexDeclaration "VertexDeclaration" can be manually bound to the @ref BansheeEngine::RenderAPI "RenderAPI", which can be useful in the case you're manually creating vertex or index buffers.

### Reading/writing MeshData {#meshes_c_a_b}
@ref BansheeEngine::MeshData "MeshData" provides multiple methods of reading/writing vertex data. The most trivial is setting the data in-bulk by using @ref BansheeEngine::MeshData::setVertexData "MeshData::setVertexData" and @ref BansheeEngine::MeshData::getVertexData "MeshData::getVertexData" which set vertex data for a single vertex element all at once. 

~~~~~~~~~~~~~{.cpp}
SPtr<MeshData> meshData = ...; // Assume we have 4 vertices, with only a position element

Vector3 myVertices[4];
// ... Set vertex positions ...

// Write the vertices
meshData->setVertexData(VES_POSITION, myVertices, sizeof(myVertices));
~~~~~~~~~~~~~

You can also use @ref BansheeEngine::MeshData::getElementData "MeshData::getElementData" which will return a pointer to the starting point of the vertex data for a specific element. You can then iterate over the pointer to read/write values. Make sure to use @ref BansheeEngine::VertexDataDesc::getVertexStride "VertexDataDesc::getVertexStride" to know how many bytes to advance between elements.

~~~~~~~~~~~~~{.cpp}
SPtr<MeshData> meshData = ...; // Assume we have 4 vertices, with only a position element

UINT8* vertices = meshData->getElementData(VES_POSITION);
UINT32 stride = meshData->getVertexDesc()->getVertexStride();

for(UINT32 i = 0; i < 4; i++)
{
	Vector3 myVertex(i, i, 0);
	memcpy(vertices, &myVertex, sizeof(myVertex));
	
	vertices += stride;
}
~~~~~~~~~~~~~

And finally you can use iterators: @ref BansheeEngine::MeshData::getVec2DataIter "MeshData::getVec2DataIter", @ref BansheeEngine::MeshData::getVec3DataIter "MeshData::getVec3DataIter", @ref BansheeEngine::MeshData::getVec4DataIter "MeshData::getVec4DataIter", @ref BansheeEngine::MeshData::getDWORDDataIter "MeshData::getDWORDDataIter". They are similar to the previous example but you don't need to manually worry about the vertex stride, or going outside of valid bounds.

~~~~~~~~~~~~~{.cpp}
SPtr<MeshData> meshData = ...; // Assume we have 4 vertices, with only a position element

auto iter = meshData->getVec3DataIter(VES_POSITION);

int i = 0;
do {
	vecIter.addValue(myVertex(i, i, 0)); // Automatically advances the iterator
	i++;
} while(vecIter.moveNext()); // Iterate until we reach the end
~~~~~~~~~~~~~

Accessing indices is simpler and is done through @ref BansheeEngine::MeshData::getIndices32 "MeshData::getIndices32" or @ref BansheeEngine::MeshData::getIndices16 "MeshData::getIndices16" depending if the indices are 32- or 16-bit. The returned value is a pointer to the index buffer you can use to read/write the indices directly.

~~~~~~~~~~~~~{.cpp}
SPtr<MeshData> meshData = ...; // Assume we have 6 32-bit indices

UINT32* indices = meshData->getIndices32();
indices[0] = 0;
indices[1] = 1;
indices[2] = 2;

indices[3] = 2;
indices[4] = 1;
indices[5] = 3;
~~~~~~~~~~~~~

## Cached CPU data {#meshes_c_b}
When you read from a mesh using the @ref BansheeEngine::Mesh::readSubresource "Mesh::readSubresource" method the read will be performed from the GPU. This is useful if the GPU has in some way modified the mesh, but will also incur a potentially large performance penalty because it will introduce a CPU-GPU synchronization point. In a lot of cases you might just want to read mesh data from a mesh that was imported or created on the CPU in some other way.

For this reason @ref BansheeEngine::Mesh::readData "Mesh::readData" exists. It will read data quickly with little performance impact. However you must create the mesh using the @ref BansheeEngine::MU_CPUCACHED "MU_CPUCACHED" usage. This also means that the mesh will keep a copy of its data in system memory, so use it sparingly. If the mesh is modified from the GPU this method will not reflect such changes.

# Rendering using the mesh {#meshes_d}
To use a mesh for rendering you need to bind its vertex buffer(s), index buffer, vertex declaration and draw operation using the @ref BansheeEngine::RenderAPI "RenderAPI". Relevant methods are @ref BansheeEngine::RenderAPI::setVertexBuffers "RenderAPI::setVertexBuffers", @ref BansheeEngine::RenderAPI::setIndexBuffer "RenderAPI::setIndexBuffer", @ref BansheeEngine::RenderAPI::setVertexDeclaration "RenderAPI::setVertexDeclaration" and @ref BansheeEngine::RenderAPI::setDrawOperation "RenderAPI::setDrawOperation". See below for information about how to retrieve this information from a mesh.

See the [render API](@ref renderAPI) manual for more information on how to manually execute rendering commands.

If working on the core thread you can use the helper @ref BansheeEngine::RendererUtility::draw "RendererUtility::draw" method that performs these operations for you.

# Saving/loading {#meshes_e}
A mesh is a @ref BansheeEngine::Resource "Resource" and can be saved/loaded like any other. See the [resource](@ref resources) manual.

# Core thread meshes {#meshes_f}
So far we have only talked about the simulation thread @ref BansheeEngine::Mesh "Mesh" but have ignored the core thread @ref BansheeEngine::MeshCore "MeshCore". The functionality between the two is mostly the same, with the major difference being that the core thread version doesn't require a @ref BansheeEngine::CoreThreadAccessor "CoreAccessor" for access, and you can instead perform operations on it directly.

You can also use the core thread version to access and manipulate vertex and index buffers directly (including binding them to the pipeline as described earlier). Use @ref BansheeEngine::MeshCore::getVertexData "MeshCore::getVertexData" to retrieve information about all @ref BansheeEngine::VertexBufferCore "vertex buffers", @ref BansheeEngine::MeshCore::getIndexBuffer "MeshCore::getIndexBuffer" to retrieve the @ref BansheeEngine::IndexBufferCore "index buffer" and @ref BansheeEngine::MeshCore::getVertexDesc "MeshCore::getVertexDesc" to retrieve the @ref BansheeEngine::VertexDataDesc "vertex description".

# Advanced meshes {#meshes_g}
So far we have described the use of standard meshes. And while the described meshes can be used for all purposes, when we have a mesh that is updated often (every frame or every few frames) it can be beneficial for performance to use a different mesh type. This is because updates to a normal mesh can introduce GPU-CPU synchronization points if that mesh is still being used by the GPU while we are updating it (which can happen often).

So instead we use a concept of a @ref BansheeEngine::MeshHeap "MeshHeap", which allows us to quickly create and allocate new meshes, without having to worry about introducing GPU-CPU synchronization points. This is perfect for systems like GUI or grass rendering, which might have their meshes rebuilt nearly every frame. The only downside is that the heap will allocate more memory than actually required for a mesh, as it keeps the memory for both the newly updated mesh and the one currently being used.

Creation of a @ref BansheeEngine::MeshHeap::create "MeshHeap::create" is nearly identical to creation of a @ref BansheeEngine::Mesh "Mesh". It requires a vertex description and an index type, while the index/vertex counts are mostly arbitrary (the heap will grow as needed).

You can allocate meshes from the heap by calling @ref BansheeEngine::MeshHeap::alloc "MeshHeap::alloc" and providing it with a @ref BansheeEngine::MeshData "MeshData". When done with the mesh call @ref BansheeEngine::MeshHeap::dealloc "MeshHeap::dealloc". @ref BansheeEngine::MeshHeap::alloc "MeshHeap::alloc" returns a @ref BansheeEngine::TransientMesh "TransientMesh" which shares mostly the same interface as a normal @ref BansheeEngine::Mesh "Mesh", with a few restrictions. You are not allowed to read/write to it (aside from the initial value), and it doesn't support sub-meshes.

A simple example of a mesh heap:
~~~~~~~~~~~~~{.cpp}
SPtr<VertexDataDesc> vertexDesc = VertexDataDesc::create();
vertexDesc->addVertElem(VET_FLOAT3, VES_POSITION);

// Create the mesh heap (normally you you create this once and use it throughout the application)
SPtr<MeshHeap> meshHeap = MeshHeap::create(100, 100, vertexDesc);

SPtr<MeshData> meshData = ...; // Assume we create mesh data like in one of the previous sections
SPtr<TransientMesh> mesh = meshHeap->alloc(meshData);
// Draw using the mesh
meshHeap->dealloc(mesh);

~~~~~~~~~~~~~

See @ref BansheeEngine::GUIManager "GUIManager" implementation in "BsGUIManager.cpp" for a functional example.