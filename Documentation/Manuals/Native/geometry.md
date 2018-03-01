Geometry									{#geometry}
===============
[TOC]

In this chapter we'll show how is geometry of an object represented, and how to bind that geometry to be rendered. The geometry is represented using three object types:
 - @ref bs::ct::VertexBuffer "ct::VertexBuffer" - Contains per-vertex information
 - @ref bs::ct::VertexDeclaration "ct::VertexDeclaration" - Contains meta-data about which properties each entry in the vertex buffer contains
 - @ref bs::ct::IndexBuffer "ct::IndexBuffer" - Contains a list of indices that map into the vertex buffer and determine in what order are vertices assembled into primitives (triangles, lines or points)

# Vertex buffer {#geometry_a}
**ct::VertexBuffer** is a buffer that contains all vertices of the object we wish to render. When drawing the vertices will be interpreted as primitives (either points, lines or triangles) and rendered. Each vertex can have one or multiple properties associated with it.

To create a vertex buffer call @ref bs::ct::VertexBuffer::create "ct::VertexBuffer::create()" with a populated @ref bs::VERTEX_BUFFER_DESC "VERTEX_BUFFER_DESC" structure. You need to know the size of an individual vertex (determined by the properties each vertex requires) and the number of vertices. 

~~~~~~~~~~~~~{.cpp}
// Create a vertex buffer containing 8 vertices with just a vertex position (3D float)
VERTEX_BUFFER_DESC desc;
desc.vertexSize = sizeof(Vector3);
desc.numVerts = 8;

SPtr<VertexBuffer> vb = VertexBuffer::create(desc);
~~~~~~~~~~~~~

Once the vertex buffer is created you will want to populate it with some data. For this you can use any of the following methods:
 - @ref bs::ct::VertexBuffer::lock "ct::VertexBuffer::lock()" - Locks a specific region of the vertex buffer and returns a pointer you can then use for reading and writing. Make sure to specify valid a @ref bs::GpuLockOptions "GpuLockOptions" signaling whether you are planning on reading or writing from the buffer. Once done call @ref bs::ct::VertexBuffer::unlock "ct::VertexBuffer::unlock()" to make the locked region accessible to the GPU again.
 - @ref bs::ct::VertexBuffer::writeData "ct::VertexBuffer::writeData()" - Writes an entire block of memory at once.
 
~~~~~~~~~~~~~{.cpp}
// Fill out a vertex buffer using lock/unlock approach
Vector3* positions = (Vector3*)vb->lock(0, sizeof(Vector3) * 8, GBL_WRITE_ONLY_DISCARD);
positions[0] = Vector3(0, 0, 0);
positions[1] = Vector3(10, 0, 0);
// ... assign other 6 positions
vb->unlock();
~~~~~~~~~~~~~  
 
Once a vertex buffer is created and populated with data, you can bind it to the pipeline by calling @ref bs::ct::RenderAPI::setVertexBuffers "ct::RenderAPI::setVertexBuffers()". You can bind one or multiple vertex buffers at once. If binding multiple vertex buffers they must all share the same vertex count, but each may contain different vertex properties (e.g. one might contain just positions and UV, while another might contain tangents and normals).

~~~~~~~~~~~~~{.cpp}
// Bind a single vertex buffer
RenderAPI& rapi = RenderAPI::instance();
rapi.setVertexBuffers(0, { vb });
~~~~~~~~~~~~~

# Vertex declaration {#geometry_b}
Before vertex buffer(s) can be used for rendering, you need to tell the pipeline what kind of information does each vertex in the vertex buffer(s) contain. This information lets the GPU know how to map per-vertex properties like position & UV coordinates, to vertex GPU program inputs. This is done by creating a @ref bs::ct::VertexDeclaration "ct::VertexDeclaration" object.

In order to create one you call @ref bs::ct::VertexDeclaration::create "ct::VertexDeclaration::create()" which accepts a **VertexDataDesc** as its parameter. We already explained to how create a **VertexDataDesc** in the @ref creatingMeshes manual.

~~~~~~~~~~~~~{.cpp}
// Create a vertex descriptor with a position, normal and UV coordinates
SPtr<VertexDataDesc> vertexDesc = VertexDataDesc::create();
vertexDesc->addVertElem(VET_FLOAT3, VES_POSITION);
vertexDesc->addVertElem(VET_FLOAT3, VES_NORMAL);
vertexDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD);

// Create a vertex declaration
SPtr<VertexDeclaration> vertexDecl = VertexDeclaration::create(vertexDesc);
~~~~~~~~~~~~~

If you are binding multiple vertex buffers, then make use of the @p streamIdx parameter when registering elements in **VertexDataDesc**. This index will let the pipeline know in which vertex buffer to find the provided element.

Once created you can bind the declaration to the pipeline by calling @ref bs::ct::RenderAPI::setVertexDeclaration "ct::RenderAPI::setVertexDeclaration()".

~~~~~~~~~~~~~{.cpp}
RenderAPI& rapi = RenderAPI::instance();
rapi.setVertexDeclaration(vertexDecl);
~~~~~~~~~~~~~

**ct::VertexDeclaration** can also be used for querying information about vertex size and offsets, which can be useful for creating the vertex buffer. First retrieve the @ref bs::VertexDeclarationProperties "VertexDeclarationProperties" object by calling @ref bs::ct::VertexDeclaration::getProperties() "ct::VertexDeclaration::getProperties()". Then you can query for information like vertex size by calling @ref bs::VertexDeclarationProperties::getVertexSize "VertexDeclarationProperties::getVertexSize()".

~~~~~~~~~~~~~{.cpp}
auto& props = vertexDecl->getProperties();

// Gets the size of an individual vertex in the 0th stream (stream index maps to bound vertex buffer at the same index)
UINT32 vertexSize = props.getVertexSize(0);
~~~~~~~~~~~~~

# Index buffer {#geometry_c}
Finally, before drawing you will usually also want to also bind an index buffer. Index buffers are optional, but should be used in most cases. Each entry in an index buffer points to a vertex in the vertex buffer, and sequential indices are used to form primitives for rendering (e.g. every three indices will form a triangle). This ensures you can re-use same vertex in multiple primitives, saving on memory and bandwidth, as well as create more optimal vertex order for GPU processing. Without an index buffer the vertices are instead read sequentially in the order they are defined in the vertex buffer.

To create an index buffer call @ref bs::ct::IndexBuffer::create "ct::IndexBuffer::create()" with a populated @ref bs::INDEX_BUFFER_DESC "INDEX_BUFFER_DESC" structure. The call requires a number of indices and their type. Indices can be either 16- or 32-bit. 

~~~~~~~~~~~~~{.cpp}
// Create an index buffer containing 36 16-bit indices
INDEX_BUFFER_DESC desc;
desc.indexType = IT_16BIT;
desc.numIndices = 36;

SPtr<IndexBuffer> ib = IndexBuffer::create(desc);
~~~~~~~~~~~~~

Reading and writing from/to the index buffer has the identical interface to the vertex buffer, so we won't show it again.

To bind an index buffer to the pipeline call @ref bs::ct::RenderAPI::setIndexBuffer "ct::RenderAPI::setIndexBuffer()".

~~~~~~~~~~~~~{.cpp}
// Bind an index buffer
RenderAPI& rapi = RenderAPI::instance();
rapi.setIndexBuffer(ib);
~~~~~~~~~~~~~

# Geometry from meshes {#geometry_d}
All the objects we described so far can be retrieved directly from a **ct::Mesh**. This allows you to manually bind imported mesh geometry to the pipeline. 

This fact can also be exploited for easier vertex/index buffer and declaration creation, as creating a **ct::Mesh** is usually simpler than creating these objects individually.

To retrieve an index buffer from a **ct::Mesh** call @ref bs::ct::Mesh::getIndexBuffer() "ct::Mesh::getIndexBuffer()". Vertex buffer(s) and vertex declaration can be retrieved from a @ref bs::ct::VertexData "ct::VertexData" structure returned by @ref bs::ct::Mesh::getVertexData() "ct::Mesh::getVertexData()". **ct::VertexDeclaration** can then be retrieved from @ref bs::ct::VertexData::vertexDeclaration "ct::VertexData::vertexDeclaration", and vertex buffers from @ref bs::ct::VertexData::getBuffer "ct::VertexData::getBuffer()"

~~~~~~~~~~~~~{.cpp}
SPtr<Mesh> mesh = ...;
SPtr<IndexBuffer> meshIB = mesh->getIndexBuffer();
SPtr<VertexData> vertexData = mesh->getVertexData();
SPtr<VertexDeclaration> = vertexData->vertexDeclaration;
SPtr<VertexBuffer> meshVB = vertexData->getBuffer(0);
~~~~~~~~~~~~~