---
title: Geometry
---

In this chapter we'll show how is geometry of an object represented, and how to bind that geometry to be rendered. The geometry is represented using three object types:
 - @b3d::render::GpuBuffer - Contains per-vertex information or indices
 - @b3d::render::VertexDescription - Contains meta-data about which properties each entry in the vertex buffer contains
 - @b3d::render::VertexData - A container holding vertex buffers and their description

# Vertex buffer
**render::GpuBuffer** is a buffer that contains all vertices of the object we wish to render. When drawing the vertices will be interpreted as primitives (either points, lines or triangles) and rendered. Each vertex can have one or multiple properties associated with it.

To create a vertex buffer call @b3d::render::GpuDevice::CreateGpuBuffer with a populated @b3d::GpuBufferCreateInformation structure. You can use the helper method @b3d::GpuBufferCreateInformation::CreateVertex to build the descriptor. You need to know the size of an individual vertex (determined by the properties each vertex requires) and the number of vertices.

~~~~~~~~~~~~~{.cpp}
// Create a vertex buffer containing 8 vertices with just a vertex position (3D float)
GpuBufferCreateInformation createInformation = GpuBufferCreateInformation::CreateVertex(sizeof(Vector3), 8);
TShared<GpuBuffer> vertexBuffer = gpuDevice->CreateGpuBuffer(createInformation);
~~~~~~~~~~~~~

Once the vertex buffer is created you will want to populate it with some data. For this you can use any of the following methods:
 - @b3d::render::GpuBuffer::Map - Maps a specific region of the vertex buffer and returns a @b3d::render::GpuBufferMappedScope RAII wrapper containing a pointer you can use for reading and writing. The buffer is automatically flushed when the scope exits.
 - @b3d::render::GpuBuffer::Write - Writes an entire block of memory at once.

~~~~~~~~~~~~~{.cpp}
// Fill out a vertex buffer using Map approach
{
	GpuBufferMappedScope mappedScope = vertexBuffer->Map(0, sizeof(Vector3) * 8, GpuMapOption::Write);
	Vector3* positions = static_cast<Vector3*>(mappedScope.GetMappedMemory());
	positions[0] = Vector3(0, 0, 0);
	positions[1] = Vector3(10, 0, 0);
	// ... assign other 6 positions
} // Automatically flushes on scope exit
~~~~~~~~~~~~~

Once a vertex buffer is created and populated with data, you can bind it to the pipeline by calling @b3d::render::GpuCommandBuffer::SetVertexBuffers. You can bind one or multiple vertex buffers at once. If binding multiple vertex buffers they must all share the same vertex count, but each may contain different vertex properties (e.g. one might contain just positions and UV, while another might contain tangents and normals).

~~~~~~~~~~~~~{.cpp}
// Bind a single vertex buffer
TShared<GpuBuffer> buffers[] = { vertexBuffer };
commandBuffer->SetVertexBuffers(0, buffers, 1);
~~~~~~~~~~~~~

# Vertex description
Before vertex buffer(s) can be used for rendering, you need to tell the pipeline what kind of information does each vertex in the vertex buffer(s) contain. This information lets the GPU know how to map per-vertex properties like position & UV coordinates, to vertex GPU program inputs. This is done by creating a @b3d::render::VertexDescription object.

In order to create one you construct a **VertexDescription** with an array of **VertexElement** objects. Each element describes a single vertex property.

~~~~~~~~~~~~~{.cpp}
// Create a vertex description with a position, normal and UV coordinates
VertexElement elements[] = {
	VertexElement(VET_FLOAT3, VES_POSITION, 0, 0, 0),
	VertexElement(VET_FLOAT3, VES_NORMAL, 0, 0, 0),
	VertexElement(VET_FLOAT2, VES_TEXCOORD, 0, 0, 0)
};

TShared<VertexDescription> vertexDescription = B3DMakeShared<VertexDescription>(TArrayView<VertexElement>(elements, 3));
~~~~~~~~~~~~~

If you are binding multiple vertex buffers, then make use of the `streamIndex` parameter when constructing **VertexElement** objects. This index will let the pipeline know in which vertex buffer to find the provided element.

Once created you can bind the description to the pipeline by calling @b3d::render::GpuCommandBuffer::SetVertexDescription.

~~~~~~~~~~~~~{.cpp}
commandBuffer->SetVertexDescription(vertexDescription);
~~~~~~~~~~~~~

**render::VertexDescription** can also be used for querying information about vertex size and offsets, which can be useful for creating the vertex buffer. You can query for information like vertex size by calling @b3d::VertexDescription::GetVertexStride.

~~~~~~~~~~~~~{.cpp}
// Gets the size of an individual vertex in the 0th stream (stream index maps to bound vertex buffer at the same index)
u32 vertexSize = vertexDescription->GetVertexStride(0);
~~~~~~~~~~~~~

# Index buffer
Finally, before drawing you will usually also want to also bind an index buffer. Index buffers are optional, but should be used in most cases. Each entry in an index buffer points to a vertex in the vertex buffer, and sequential indices are used to form primitives for rendering (e.g. every three indices will form a triangle). This ensures you can re-use same vertex in multiple primitives, saving on memory and bandwidth, as well as create more optimal vertex order for GPU processing. Without an index buffer the vertices are instead read sequentially in the order they are defined in the vertex buffer.

To create an index buffer call @b3d::render::GpuBuffer::Create with a populated @b3d::GpuBufferCreateInformation structure. You can use the helper method @b3d::GpuBufferCreateInformation::CreateIndex to build the descriptor. The call requires a number of indices and their type. Indices can be either 16- or 32-bit.

~~~~~~~~~~~~~{.cpp}
// Create an index buffer containing 36 16-bit indices
GpuBufferCreateInformation createInformation = GpuBufferCreateInformation::CreateIndex(IT_16BIT, 36);
TShared<GpuBuffer> indexBuffer = gpuDevice->CreateGpuBuffer(createInformation);
~~~~~~~~~~~~~

Reading and writing from/to the index buffer has the identical interface to the vertex buffer, so we won't show it again.

To bind an index buffer to the pipeline call @b3d::render::GpuCommandBuffer::SetIndexBuffer.

~~~~~~~~~~~~~{.cpp}
// Bind an index buffer
commandBuffer->SetIndexBuffer(indexBuffer);
~~~~~~~~~~~~~

# Geometry from meshes
All the objects we described so far can be retrieved directly from a **render::Mesh**. This allows you to manually bind imported mesh geometry to the pipeline.

This fact can also be exploited for easier vertex/index buffer and description creation, as creating a **render::Mesh** is usually simpler than creating these objects individually.

To retrieve an index buffer from a **render::Mesh** call @b3d::render::Mesh::GetIndexBuffer(). Vertex buffer(s) and vertex description can be retrieved from a @b3d::render::VertexData structure returned by @b3d::render::Mesh::GetVertexData(). **render::VertexDescription** can then be retrieved from @b3d::render::VertexData::VertexDescription, and vertex buffers from @b3d::render::VertexData::GetBuffer.

~~~~~~~~~~~~~{.cpp}
TShared<Mesh> mesh = ...;
TShared<GpuBuffer> meshIndexBuffer = mesh->GetIndexBuffer();
TShared<VertexData> vertexData = mesh->GetVertexData();
TShared<VertexDescription> meshVertexDescription = vertexData->VertexDescription;
TShared<GpuBuffer> meshVertexBuffer = vertexData->GetBuffer(0);
~~~~~~~~~~~~~
