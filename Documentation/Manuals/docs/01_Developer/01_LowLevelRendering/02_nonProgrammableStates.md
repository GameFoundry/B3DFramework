Non-programmable states									{#nonProgrammableStates}
===============
[TOC]

When creating a **GraphicsPipelineState** and filling out a **PIPELINE_STATE_DESC** you can provide it with a set of non-programmable states, along with GPU programs. These states control the non-programmable (fixed) parts of the GPU pipeline which control rasterization, depth/stencil and blending.

~~~~~~~~~~~~~{.cpp}
PIPELINE_STATE_DESC desc;
// Bind GPU programs
desc.rasterizerState = ...;
desc.depthStencilState = ...;
desc.blendState = ...;

SPtr<GraphicsPipelineState> graphicsPipeline = GraphicsPipelineState::create(desc);
~~~~~~~~~~~~~

There are three non-programmable state objects in total:
 - @ref bs::ct::RasterizerState "ct::RasterizerState"
 - @ref bs::ct::DepthStencilState "ct::DepthStencilState"
 - @ref bs::ct::BlendState "ct::BlendState"
 
> If using bs::f Shading Language you can specify these states directly in a BSL file and should have no need to create them manually. 
 
# Rasterizer state {#nonProg_a}
Rasterizer state allows you to control how are 3D polygons, lines or points converted to 2D pixels. You can create it by filling out the @ref bs::RASTERIZER_STATE_DESC "RASTERIZER_STATE_DESC" structure and passing it to @ref bs::ct::RasterizerState::create "ct::RasterizerState::create()".

~~~~~~~~~~~~~{.cpp}
// Draw wireframe geometry with no backface culling
RASTERIZER_STATE_DESC desc;
desc.polygonMode = PM_WIREFRAME; // Draw wireframe instead of solid
desc.cullMode = CULL_NONE; // Disable blackface culling

SPtr<RasterizerState> rasterizerState = RasterizerState::create(desc);
~~~~~~~~~~~~~

# Depth-stencil state {#nonProg_b}
Depth-stencil state allows you to control how are depth and/or stencil buffers modified during rendering. You can create it by filling out the @ref bs::DEPTH_STENCIL_STATE_DESC "DEPTH_STENCIL_STATE_DESC" structure and passing it to @ref bs::ct::DepthStencilState::create "ct::DepthStencilState::create()".

~~~~~~~~~~~~~{.cpp}
// Draw with no depth testing or writing, and with a stencil operation that writes 1 for each sample written
DEPTH_STENCIL_STATE_DESC desc;
desc.depthReadEnable = false; // Don't test against current contents of depth buffer
desc.depthWriteEnable = false; // Don't make any changes to depth buffer
desc.stencilEnable = true; // Enable stencil operations
desc.frontStencilPassOp = SOP_INCREMENT; // Increment by one whenever a front-face stencil operation passes
desc.frontStencilComparisonFunc = CMPF_ALWAYS_PASS; // Always pass the stencil operation

SPtr<DepthStencilState> depthStencilState = DepthStencilState::create(desc);
~~~~~~~~~~~~~

# Blend state {#nonProg_c}
Blend state allows to you to control how is a rendered pixel blended with any previously rendered pixels. You can create it by filling out the @ref bs::BLEND_STATE_DESC "BLEND_STATE_DESC" structure and passing it to @ref bs::ct::BlendState::create "ct::BlendState::create()". Most of blend state options can be controlled individually for up to 8 render targets.

~~~~~~~~~~~~~{.cpp}
// Set up blending (e.g. for transparent rendering) for the first render target
BLEND_STATE_DESC desc;
desc.renderTargetDesc[0].blendEnable = true; // Enable blending
desc.renderTargetDesc[0].srcBlend = BF_SOURCE_ALPHA; // Use the current alpha value to blend the source (new value)
desc.renderTargetDesc[0].dstBlend = BF_INV_SOURCE_ALPHA; // Use the inverse of the current alpha value to blend the destination (stored value)
desc.renderTargetDesc[0].blendOp = BO_ADD; // Add the source and destination together

SPtr<BlendState> blendState = BlendState::create(desc);
~~~~~~~~~~~~~