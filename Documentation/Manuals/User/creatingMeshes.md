Advanced meshes				{#creatingMeshes}
===============
[TOC]

In this chapter we'll learn how to create meshes manually and populate them with data. 

# Creating a mesh {#creatingMeshes_a}
To create a mesh call @ref bs::Mesh::create "Mesh::create()" or one if its overloads. You'll need to populate the @ref bs::MESH_DESC "MESH_DESC" structure and pass it as a parameter. At minimum the structure requires you to provide:
 - @ref bs::MESH_DESC::numVertices "MESH_DESC::numVertices" - Number of vertices in the mesh
 - @ref bs::MESH_DESC::numIndices "MESH_DESC::numIndices" - Number of indices in the mesh
 - @ref bs::MESH_DESC::vertexDesc "MESH_DESC::vertexDesc" - Structure of type @ref bs::VertexDataDesc "VertexDataDesc" that describes what kind of data does each individual vertex contains, which we'll discuss in detail later.

Optionally you can also provide:
 - @ref bs::MESH_DESC::indexType "MESH_DESC::indexType" - Type of each index in the index buffer. They can be 32 or 16 bit, as specified by the @ref bs::IndexType "IndexType" enum.
 - @ref bs::MESH_DESC::subMeshes "MESH_DESC::subMeshes" - A mesh can have multiple sub-meshes. Each sub-mesh is described by an offset and a range of indices that belong to it. Sub-meshes can be used for rendering sections of a mesh, instead of all of it (for example if a single mesh uses different materials). By default all indices are considered to be part of a single mesh.
 - @ref bs::MESH_DESC::usage "MESH_DESC::usage" - Usage flag that specifies how the mesh is intended to be used, in a form of @ref bs::MeshUsage "MeshUsage" enum.
 
Supported mesh usage flags are:
 - @ref bs::MU_STATIC "MeshUsage::MU_STATIC" - Specify for normal meshes that are created once (or updated very rarely)
 - @ref bs::MU_DYNAMIC "MeshUsage::MU_DYNAMIC" - Specify for meshes that are updated often (e.g. every frame)
 - @ref bs::MU_CPUCACHED "MeshUsage::MU_CPUCACHED" - Specify that any data written to the mesh (from the CPU) will be cached internally, allowing it to be accessed through **Mesh::readCachedData()**. Uses extra memory as data needs to be stored in both normal and GPU memory.
 
~~~~~~~~~~~~~{.cpp}
// Creates an empty mesh with 36 indices and 8 vertices
MESH_DESC meshDesc;
meshDesc.numVertices = 8;
meshDesc.numIndices = 36;

SPtr<VertexDataDesc> vertexDesc = ...; // Vertex description creation is explained below
meshDesc.vertexDesc = vertexDesc;

HMesh mesh = Mesh::create(meshDesc);
~~~~~~~~~~~~~

## Vertex description {#creatingMeshes_a_a}
To create a new vertex description object you call @ref bs::VertexDataDesc::create "VertexDataDesc::create()". After creation you need to specify a list of vertex elements by calling @ref bs::VertexDataDesc::addVertElem "VertexDataDesc::addVertElem()". Each vertex element is identified by:
 - Type - Determines the size and format of that specific property (e.g. a 3D float for a position property). All supported types are provided in the @ref bs::VertexElementType "VertexElementType" enum.
 - Semantic - Determines to which vertex GPU program input field will this property be mapped to. All supported semantic types are provided in the @ref bs::VertexElementSemantic "VertexElementSemantic" enum. Multiple types can be mapped to the same semantic by using the `semanticIdx` parameter.

~~~~~~~~~~~~~{.cpp}
// Create a vertex with a position, normal and UV coordinates
SPtr<VertexDataDesc> vertexDesc = VertexDataDesc::create();
vertexDesc->addVertElem(VET_FLOAT3, VES_POSITION);
vertexDesc->addVertElem(VET_FLOAT3, VES_NORMAL);
vertexDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD);
~~~~~~~~~~~~~ 

You may also specify these optional properties, primarily useful for low-level rendering:
 - Stream index - Each vertex element can also be placed in a "stream". In the example above all the elements are in the 0th stream, but you can place different elements in different streams by providing a stream index. This can be useful if you are rendering the mesh with multiple GPU programs and not every GPU program requires all vertex elements, in which case you could only bind a subset of the vertex buffers and reduce the bandwidth costs by not sending the unnecessary data.
 - Instance step rate - Values larger than 1 mean that data provided in the vertex buffer is not per vertex, but rather per instance. This is used for instanced rendering.

Once the **VertexDataDesc** structure has been filled, you can use it for initializing a **Mesh** as shown above.

# Writing mesh data {#creatingMeshes_b}
After mesh has been created you need to write some vertex and index data to it by calling @ref bs::Mesh::writeData "Mesh::writeData()". This method accepts a @ref bs::MeshData "MeshData" object.

~~~~~~~~~~~~~{.cpp}
SPtr<MeshData> meshData = ...; // Explained below
// ... populate meshData

mesh->writeData(meshData);
~~~~~~~~~~~~~ 

## Creating mesh data {#creatingMeshes_b_a}
You can create @ref bs::MeshData "MeshData" by calling @ref bs::MeshData::create(UINT32, UINT32, const SPtr<VertexDataDesc>&, IndexType) "MeshData::create()" and providing it with vertex description, index type and number of vertices and indices. You must ensure that the formats and sizes match the mesh this will be used on.

~~~~~~~~~~~~~{.cpp}
// Create mesh data able to contain 8 vertices of the format specified by vertexDesc, and 36 indices
SPtr<MeshData> vertexDesc = MeshData::create(8, 36, vertexDesc);
~~~~~~~~~~~~~ 

You can also create **MeshData** using an existing mesh by calling @ref bs::Mesh::allocBuffer "Mesh::allocBuffer()". This will create an object of adequate size and vertex description for use on that mesh.

~~~~~~~~~~~~~{.cpp}
SPtr<MeshData> vertexDesc = mesh->allocBuffer();
~~~~~~~~~~~~~ 

## Populating mesh data {#creatingMeshes_b_b}
Once **MeshData** has been created you need to populate it with vertices and indices. This can be done in a few ways.

The most basic way is setting the data by using @ref bs::MeshData::setVertexData "MeshData::setVertexData()" which set vertex data for a single vertex element all at once.

~~~~~~~~~~~~~{.cpp}
// Fill out the data for the 0th VES_POSITION element
Vector3 myVertexPositions[8];
for(UINT32 i = 0; i < 8; i++)
	myVertexPositions = Vector3(i, i, 0); // Arbitrary

// Write the vertices
meshData->setVertexData(VES_POSITION, myVertexPositions, sizeof(myVertexPositions));
~~~~~~~~~~~~~

You can also use @ref bs::MeshData::getElementData "MeshData::getElementData()" which will return a pointer to the starting point of the vertex data for a specific element. You can then iterate over the pointer to read/write values. Make sure to use @ref bs::VertexDataDesc::getVertexStride "VertexDataDesc::getVertexStride()" to know how many bytes to advance between elements. This ensures you don't need to create an intermediate buffer like we did above.

~~~~~~~~~~~~~{.cpp}
// Fill out the data for the 0th VES_POSITION element
UINT8* vertices = meshData->getElementData(VES_POSITION);
UINT32 stride = meshData->getVertexDesc()->getVertexStride();

for(UINT32 i = 0; i < 8; i++)
{
	Vector3 myPosition(i, i, 0); // Arbitrary
	memcpy(vertices, &myPosition, sizeof(myPosition));
	
	vertices += stride;
}
~~~~~~~~~~~~~

And finally you can use iterators: @ref bs::MeshData::getVec2DataIter "MeshData::getVec2DataIter()", @ref bs::MeshData::getVec3DataIter "MeshData::getVec3DataIter()", @ref bs::MeshData::getVec4DataIter "MeshData::getVec4DataIter()", @ref bs::MeshData::getDWORDDataIter "MeshData::getDWORDDataIter()". They are similar to the previous example but you don't need to manually worry about the vertex stride, or going outside of valid bounds.

~~~~~~~~~~~~~{.cpp}
// Fill out the data for the VES_POSITION element
auto iter = meshData->getVec3DataIter(VES_POSITION);

Vector3 myPosition(0, 0, 0)
do {
	myPosition.x += 1.0f; // Arbitrary
	myPosition.y += 1.0f; // Arbitrary
} while(vecIter.addValue(myPosition)); // // Automatically advances the iterator, and returns false when there's no more room
~~~~~~~~~~~~~

Writing indices is simpler and is done through @ref bs::MeshData::getIndices32 "MeshData::getIndices32()" or @ref bs::MeshData::getIndices16 "MeshData::getIndices16()" depending if the indices are 32 or 16 bit. The returned value is a pointer to the index buffer you can use to read/write the indices directly.

~~~~~~~~~~~~~{.cpp}
// Write 6 32-bit indices
UINT32* indices = meshData->getIndices32();
indices[0] = 0;
indices[1] = 1;
indices[2] = 2;

indices[3] = 2;
indices[4] = 1;
indices[5] = 3;
~~~~~~~~~~~~~

## Discard on write {#creatingMeshes_b_c}
When you are sure you will overwrite all the contents of a mesh, make sure to set the last parameter of **Mesh::writeData()** to true. This ensures the system can more optimally execute the transfer, without requiring the GPU to finish its current action (which can be considerably slow if it is currently using that particular mesh).
 
# Reading cached CPU data {#creatingMeshes_c}
Reading cached CPU data allows you to read-back any data you have written to the mesh when calling **Mesh::writeData()**. It is particularily useful when importing meshes from external files and wish to access their vertex/index data. Note that mesh must be created with the **MeshUsage::MU_CPUCACHED** usage flag in order for CPU cached data to be available. When importing meshes this flag will automatically be set if the relevant property is enabled in **MeshImportOptions**.

Cached CPU data can be read by calling @ref bs::Mesh::readCachedData "Mesh::readCachedData()". It accepts a **MeshData** parameter to which to output the index and vertex data.

~~~~~~~~~~~~~{.cpp}
SPtr<MeshData> meshData = mesh->allocBuffer();
mesh->readCachedData(*meshData);
~~~~~~~~~~~~~

After reading the data you can access it through @ref bs::MeshData::getVertexData "PixelData::getVertexData()", @ref bs::MeshData::getElementData "PixelData::getElementData()" or through iterators.

~~~~~~~~~~~~~{.cpp}
// Read the data for the VES_POSITION element, using iterators
auto iter = meshData->getVec3DataIter(VES_POSITION);

UINT32 numVertices = meshData->getNumVertices();
Vector3* output = bs_newN<Vector>(numVertices);

for(UINT32 i = 0; i < numVertices; i++)
{
	output[i] = iter.getValue(); // Returns current value
	iter.moveNext(); // Move to next vertex
}
~~~~~~~~~~~~~