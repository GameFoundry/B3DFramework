---
title: Drawing
---

We've now covered all the essential components needed for rendering: GPU programs and pipeline states, render targets, and geometry buffers. With all these pieces in place, we're ready to issue draw calls that will actually render something to the screen.

When drawing you must ensure a pipeline state of **GpuGraphicsPipelineState** type is bound, instead of **GpuComputePipelineState**.

Next you need to specify what type of primitives you wish to draw by calling @b3d::render::GpuCommandBuffer::SetDrawOperation, which accepts any of the types defined in @b3d::DrawOperationType. This determines how are contents of the index buffer interpreted (or vertex buffer if index isn't available). The available draw types are:
 - @b3d::DOT_POINT_LIST - Each vertex represents a point.
 - @b3d::DOT_LINE_LIST - Each sequential pair of vertices represent a line.
 - @b3d::DOT_LINE_STRIP - Each vertex (except the first) forms a line with the previous vertex.
 - @b3d::DOT_TRIANGLE_LIST - Each sequential 3-tuple of vertices represent a triangle.
 - @b3d::DOT_TRIANGLE_STRIP - Each vertex (except the first two) form a triangle with the previous two vertices.
 - @b3d::DOT_TRIANGLE_FAN - Each vertex (except the first two) forms a triangle with the first vertex and previous vertex.

~~~~~~~~~~~~~{.cpp}
// We're drawing a triangle list
SPtr<GpuCommandBuffer> commandBuffer = ...;
commandBuffer->SetDrawOperation(DOT_TRIANGLE_LIST);
~~~~~~~~~~~~~

# Indexed drawing
Finally you can now draw the object by calling @b3d::render::GpuCommandBuffer::DrawIndexed(). It requires the following parameters:
 - `startIndex` - Offset into the bound index buffer to start drawing from. In most cases this will be zero.
 - `indexCount` - Number of indices to draw. Specify total number of indices in the index buffer to draw them all (most common case).
 - `vertexOffset` - Offset to append to each index in the index buffer. Allows you to draw different set of vertices using the same index buffer. In most cases this will be zero.
 - `vertexCount` - Number of vertices to draw. Since the actual number of primitives drawn is determined by the index count, this value is internally used just for tracking purposes and wont affect your rendering. In most cases you can specify the number of vertices in the vertex buffer(s).

~~~~~~~~~~~~~{.cpp}
SPtr<GpuBuffer> indexBuffer = ...;
SPtr<GpuBuffer> vertexBuffer = ...;

u32 numIndices = indexBuffer->GetProperties().IndexCount;
u32 numVertices = vertexBuffer->GetProperties().ElementCount;

SPtr<GpuCommandBuffer> commandBuffer = ...;
commandBuffer->DrawIndexed(0, numIndices, 0, numVertices);
~~~~~~~~~~~~~

# Non-indexed drawing
If drawing without an index buffer you can call @b3d::render::GpuCommandBuffer::Draw() instead. It requires only the `vertexOffset` and `vertexCount` parameters, with same meaning as above (except `vertexCount` in this case does affect the rendering).

~~~~~~~~~~~~~{.cpp}
SPtr<GpuBuffer> vertexBuffer = ...;
u32 numVertices = vertexBuffer->GetProperties().ElementCount;

SPtr<GpuCommandBuffer> commandBuffer = ...;
commandBuffer->Draw(0, numVertices);
~~~~~~~~~~~~~

# Instanced drawing
Both **GpuCommandBuffer::Draw()** and **GpuCommandBuffer::DrawIndexed()** support drawing multiple instances of the same object using the `instanceCount` parameter. This can be used as an alternative for issuing multiple **Draw** calls, as they may have a significant CPU overhead. Using instanced drawing you can draw the same geometry multiple times with almost no additional CPU overhead.

~~~~~~~~~~~~~{.cpp}
// Draw 5 instances of the currently bound geometry
commandBuffer->DrawIndexed(0, numIndices, 0, numVertices, 5);
~~~~~~~~~~~~~

In order for instanced drawing to actually be useful, in most cases we need a way to differentiate the instances. At the very least we usually want to draw the instances at different positions in the world.

This is done by creating a separate **GpuBuffer** that contains per-instance properties (like position). This buffer is created same as a normal vertex buffer, except it doesn't contain per-vertex data, and instead contains per-instance data.

In order to use such a buffer we need to let the pipeline know by creating an appropriate **VertexDescription**. We need to define vertex elements that contain per-instance data. This is done by specifying the `instanceStepRate` parameter when constructing **VertexElement**.

~~~~~~~~~~~~~{.cpp}
// Create vertex elements for per-vertex data
VertexElement positionElement(VET_FLOAT3, VES_POSITION, 0, 0, 0);
VertexElement normalElement(VET_FLOAT3, VES_NORMAL, 0, 0, 0);
VertexElement texcoordElement(VET_FLOAT2, VES_TEXCOORD, 0, 0, 0);

// Create vertex element for per-instance data
// Each entry in the instance vertex buffer is a 3D float mapped to the position semantic
// We use semantic index 1, as index 0 is taken by per-vertex VES_POSITION semantic
// We use the second bound vertex buffer (streamIndex = 1) for instance data
// Instance step rate of 1 means new element will be fetched from vertex buffer for each drawn instance
VertexElement instancePositionElement(VET_FLOAT3, VES_POSITION, 1, 1, 1);

// Create vertex description
TInlineArray<VertexElement, 4> elements = { positionElement, normalElement, texcoordElement, instancePositionElement };
SPtr<VertexDescription> vertexDescription = B3DMakeShared<VertexDescription>(elements.data(), elements.size());

SPtr<GpuCommandBuffer> commandBuffer = ...;

// Bind vertex description
commandBuffer->SetVertexDescription(vertexDescription);

// Bind per-vertex and per-instance vertex buffers
SPtr<GpuBuffer> buffers[2] = { perVertexVertexBuffer, perInstanceVertexBuffer };
commandBuffer->SetVertexBuffers(0, buffers, 2);

// Draw ...
~~~~~~~~~~~~~

# Drawing helper
As a way of making drawing easier you can also use @b3d::render::RendererUtility::Draw helper method, accessible globally through @b3d::render::GetRendererUtility(). This method accepts a **MeshBase** as input and will automatically:
 - Bind vertex & index buffer
 - Bind vertex description
 - Set draw operation type
 - Execute a draw call

~~~~~~~~~~~~~{.cpp}
SPtr<MeshBase> mesh = ...;
SPtr<GpuCommandBuffer> commandBuffer = ...;
GetRendererUtility().Draw(*commandBuffer, mesh);
~~~~~~~~~~~~~
