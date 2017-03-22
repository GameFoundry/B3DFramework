Drawing									{#drawing}
===============
[TOC]

With all the objects from the previous chapters bound to the pipeline we are almost ready to draw an object. When drawing you must ensure a pipeline state of **GraphicsPipelineState** type is bound, instead of **ComputePipelineState**.

Next you need to specify what type of primitives you wish to draw by calling @ref bs::ct::RenderAPI::setDrawOperation "ct::RenderAPI::setDrawOperation()", which accepts any of the type defined in @ref bs::DrawOperationType "DrawOperationType". This determines how are contents of the index buffer interpreted (or vertex buffer if index isn't available). The available draw types are:
 - @ref bs::DOT_POINT_LIST "DOT_POINT_LIST" - Each vertex represents a point.
 - @ref bs::DOT_LINE_LIST "DOT_POINT_LIST" - Each sequential pair of vertices represent a line.
 - @ref bs::DOT_LINE_STRIP "DOT_LINE_STRIP" - Each vertex (except the first) forms a line with the previous vertex.
 - @ref bs::DOT_TRIANGLE_LIST "DOT_TRIANGLE_LIST" - Each sequential 3-tuple of vertices represent a triangle.
 - @ref bs::DOT_TRIANGLE_STRIP "DOT_TRIANGLE_STRIP" - Each vertex (except the first two) form a triangle with the previous two vertices.
 - @ref bs::DOT_TRIANGLE_FAN "DOT_TRIANGLE_FAN" - Each vertex (except the first two) form a triangle with the first vertex and previous vertex.

~~~~~~~~~~~~~{.cpp}
// We're drawing a triangle list
RenderAPI& rapi = RenderAPI::instance();
rapi.setDrawOperation(DOT_TRIANGLE_LIST);
~~~~~~~~~~~~~

Finally you can now draw the object by calling @ref bs::ct::RenderAPI::drawIndexed() "ct::RenderAPI::drawIndexed()".

TODO