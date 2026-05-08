---
title: Non-programmable states
---

In the previous manual we learned how to create GPU programs and bind them into pipeline states. However, GPU programs alone aren't enough to fully define rendering behavior. When creating a **GpuGraphicsPipelineState** you can also provide it with a set of non-programmable states that control the fixed parts of the GPU pipeline - namely rasterization, depth/stencil operations, and blending.

~~~~~~~~~~~~~{.cpp}
GpuGraphicsPipelineStateCreateInformation createInformation;
// Bind GPU programs
createInformation.RasterizerState = ...;
createInformation.DepthStencilState = ...;
createInformation.BlendState = ...;

TShared<GpuGraphicsPipelineState> graphicsPipeline = device->CreateGpuGraphicsPipelineState(createInformation);
~~~~~~~~~~~~~

There are three non-programmable state descriptors in total:
 - @b3d::RasterizerStateInformation
 - @b3d::DepthStencilStateInformation
 - @b3d::BlendStateInformation

> If using Banshee Shading Language you can specify these states directly in a BSL file and should have no need to create them manually.

# Rasterizer state
Rasterizer state allows you to control how are 3D polygons, lines or points converted to 2D pixels. You can create it by filling out the @b3d::RasterizerStateInformation structure and assigning it to a pipeline state descriptor.

~~~~~~~~~~~~~{.cpp}
// Draw wireframe geometry with no backface culling
GpuGraphicsPipelineStateCreateInformation createInformation;
createInformation.RasterizerState.PolygonMode = PM_WIREFRAME; // Draw wireframe instead of solid
createInformation.RasterizerState.CullMode = CULL_NONE; // Disable backface culling

TShared<GpuGraphicsPipelineState> graphicsPipeline = device->CreateGpuGraphicsPipelineState(createInformation);
~~~~~~~~~~~~~

# Depth-stencil state
Depth-stencil state allows you to control how are depth and/or stencil buffers modified during rendering. You can create it by filling out the @b3d::DepthStencilStateInformation structure and assigning it to a pipeline state descriptor.

~~~~~~~~~~~~~{.cpp}
// Draw with no depth testing or writing, and with a stencil operation that writes 1 for each sample written
GpuGraphicsPipelineStateCreateInformation createInformation;
createInformation.DepthStencilState.DepthReadEnable = false; // Don't test against current contents of depth buffer
createInformation.DepthStencilState.DepthWriteEnable = false; // Don't make any changes to depth buffer
createInformation.DepthStencilState.StencilEnable = true; // Enable stencil operations
createInformation.DepthStencilState.FrontStencilPassOp = SOP_INCREMENT; // Increment by one whenever a front-face stencil operation passes
createInformation.DepthStencilState.FrontStencilComparisonFunc = CMPF_ALWAYS_PASS; // Always pass the stencil operation

TShared<GpuGraphicsPipelineState> graphicsPipeline = device->CreateGpuGraphicsPipelineState(createInformation);
~~~~~~~~~~~~~

# Blend state
Blend state allows to you to control how is a rendered pixel blended with any previously rendered pixels. You can create it by filling out the @b3d::BlendStateInformation structure and assigning it to a pipeline state descriptor. Most of blend state options can be controlled individually for up to 8 render targets.

~~~~~~~~~~~~~{.cpp}
// Set up blending (e.g. for transparent rendering) for the first render target
GpuGraphicsPipelineStateCreateInformation createInformation;
createInformation.BlendState.RenderTargets[0].BlendEnable = true; // Enable blending
createInformation.BlendState.RenderTargets[0].ColorSourceFactor = BF_SOURCE_ALPHA; // Use the current alpha value to blend the source (new value)
createInformation.BlendState.RenderTargets[0].ColorDestinationFactor = BF_INV_SOURCE_ALPHA; // Use the inverse of the current alpha value to blend the destination (stored value)
createInformation.BlendState.RenderTargets[0].ColorBlendOperation = BO_ADD; // Add the source and destination together

TShared<GpuGraphicsPipelineState> graphicsPipeline = device->CreateGpuGraphicsPipelineState(createInformation);
~~~~~~~~~~~~~
