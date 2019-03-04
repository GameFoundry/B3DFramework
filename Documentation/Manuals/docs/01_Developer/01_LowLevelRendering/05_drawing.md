Drawing									{#drawing}
===============
[TOC]

With all the objects from the previous chapters bound to the pipeline we are almost ready to draw an object. When drawing you must ensure a pipeline state of **GraphicsPipelineState** type is bound, instead of **ComputePipelineState**.

Next you need to specify what type of primitives you wish to draw by calling @ref bs::ct::RenderAPI::setDrawOperation "ct::RenderAPI::setDrawOperation()", which accepts any of the types defined in @ref bs::DrawOperationType "DrawOperationType". This determines how are contents of the index buffer interpreted (or vertex buffer if index isn't available). The available draw types are:
 - @ref bs::DOT_POINT_LIST "DOT_POINT_LIST" - Each vertex represents a point.
 - @ref bs::DOT_LINE_LIST "DOT_LINE_LIST" - Each sequential pair of vertices represent a line.
 - @ref bs::DOT_LINE_STRIP "DOT_LINE_STRIP" - Each vertex (except the first) forms a line with the previous vertex.
 - @ref bs::DOT_TRIANGLE_LIST "DOT_TRIANGLE_LIST" - Each sequential 3-tuple of vertices represent a triangle.
 - @ref bs::DOT_TRIANGLE_STRIP "DOT_TRIANGLE_STRIP" - Each vertex (except the first two) form a triangle with the previous two vertices.
 - @ref bs::DOT_TRIANGLE_FAN "DOT_TRIANGLE_FAN" - Each vertex (except the first two) forms a triangle with the first vertex and previous vertex.

~~~~~~~~~~~~~{.cpp}
// We're drawing a triangle list
RenderAPI& rapi = RenderAPI::instance();
rapi.setDrawOperation(DOT_TRIANGLE_LIST);
~~~~~~~~~~~~~

# Indexed drawing {#drawing_a}
Finally you can now draw the object by calling @ref bs::ct::RenderAPI::drawIndexed() "ct::RenderAPI::drawIndexed()". It requires the following parameters:
 - @p startIndex - Offset into the bound index buffer to start drawing from. In most cases this will be zero.
 - @p indexCount - Number of indices to draw. Specify total number of indices in the index buffer to draw them all (most common case).
 - @p vertexOffset - Offset to append to each index in the index buffer. Allows you to draw different set of vertices using the same index buffer. In most cases this will be zero.
 - @p vertexCount - Number of vertices to draw. Since the actual number of primitives drawn is determined by the index count, this value is internally used just for tracking purposes and wont affect your rendering. In most cases you can specify the number of vertices in the vertex buffer(s).
 
~~~~~~~~~~~~~{.cpp}
SPtr<IndexBuffer> ib = ...;
SPtr<VertexBuffer> vb = ...;

UINT32 numIndices = ib->getProperties()->getNumIndices();
UINT32 numVertices = vb->getProperties()->getNumVertices();

RenderAPI& rapi = RenderAPI::instance();
rapi.drawIndexed(0, numIndices, 0, numVertices);
~~~~~~~~~~~~~

# Non-indexed drawing {#drawing_b}
If drawing without an index buffer you can call @ref bs::ct::RenderAPI::draw() "ct::RenderAPI::draw()" instead. It requires only the @p vertexOffset and @p vertexCount parameters, with same meaning as above (except @p vertexCount in this case does affect the rendering).

~~~~~~~~~~~~~{.cpp}
SPtr<VertexBuffer> vb = ...;
UINT32 numVertices = vb->getProperties()->getNumVertices();

RenderAPI& rapi = RenderAPI::instance();
rapi.draw(0, numVertices);
~~~~~~~~~~~~~

# Instanced drawing {#drawing_c}
Both **ct::RenderAPI::draw()** and **ct::RenderAPI::drawIndexed()** support drawing multiple instances of the same object using the @p instanceCount parameter. This can be used as an alternative for issuing multiple **draw** calls, as they may have a significant CPU overhead. Using instanced drawing you can draw the same geometry multiple times with almost no additional CPU overhead.

~~~~~~~~~~~~~{.cpp}
// Draw 5 instances of the currently bound geometry
rapi.drawIndexed(0, numIndices, 0, numVertices, 5);
~~~~~~~~~~~~~

In order for instanced drawing to actually be useful, in most cases we need a way to differentiate the instances. At the very least we usually want to draw the instances at different positions in the world.

This is done by creating a separate **VertexBuffer** that contains per-instance properties (like position). This buffer is created same as a normal vertex buffer, except it doesn't contain per-vertex data, and instead contains per-instance data.

In order to use such a buffer we need to let the pipeline know by creating an appropriate **VertexDeclaration**. We need to define a **VertexDataDesc** that contains per-instance data. This is done by specifying the @p instanceStepRate parameter when calling **VertexDataDesc::addVertElem()**.

~~~~~~~~~~~~~{.cpp}
SPtr<VertexDataDesc> vertexDesc = VertexDataDesc::create();

// Per vertex data (same as for non-instanced drawing)
vertexDesc->addVertElem(VET_FLOAT3, VES_POSITION);
vertexDesc->addVertElem(VET_FLOAT3, VES_NORMAL);
vertexDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD);

// Per instance data
vertexDesc->addVertElem(
 VET_FLOAT3, // Each entry in the instance vertex buffer is a 3D float
 VES_POSITION, // We map it to the position semantic as vertex shader input
 1, // We use the semantic index 1, as 0th index is taken by per-vertex VES_POSITION semantic
 1, // We use the second bound vertex buffer for instance data
 1  // Instance step rate of 1 means the new element will be fetched from the vertex buffer for each drawn instance
 );

// Create a vertex declaration
SPtr<VertexDeclaration> vertexDecl = VertexDeclaration::create(vertexDesc);

RenderAPI& rapi = RenderAPI::instance();

// Bind vertex declaration
rapi.setVertexDeclaration(vertexDecl);

// Bind per-vertex and per-instance vertex buffers
rapi.setVertexBuffers(0, { perVertexVB, perInstanceVB });

// Draw ...
~~~~~~~~~~~~~

# Drawing helper {#drawing_d}
As a way of making drawing easier you can also use @ref bs::ct::RendererUtility::draw "ct::RendererUtility::draw()" helper method, accessible globally through @ref bs::ct::gRendererUtility() "ct::gRendererUtility()". This method accepts a **ct::Mesh** as input and will automatically:
 - Bind vertex & index buffer
 - Bind vertex declaration
 - Set draw operation type
 - Execute a draw call

~~~~~~~~~~~~~{.cpp}
SPtr<Mesh> mesh = ...;
gRendererUtility().draw(mesh);
~~~~~~~~~~~~~
