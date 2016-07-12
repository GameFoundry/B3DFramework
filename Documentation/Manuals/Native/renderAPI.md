Render API									{#renderAPI}
===============
[TOC]

Render API is an interface that allows you to perform low-level rendering operations akin to DirectX or OpenGL. In Banshee this API is provided through the @ref BansheeEngine::RenderAPI "RenderAPI" for the simulation thread, and @ref BansheeEngine::RenderAPICore "RenderAPICore" for the core thread. If you are confused by the dual nature of  objects, read the [core thread](@ref coreThread) manual. 

For the remainder of this manual we'll focus on the core thread portion of the API, but both provide essentially identical functionality, with the main difference that the simulation thread one expects a @ref BansheeEngine::CoreThreadAccessor "CoreAccessor" as input, and the execution of the actual commands is delayed until they reach the core thread.

Render API lets you manipulate the GPU pipeline by setting various states (depth stencil, blend and rasterizer), binding GPU programs, textures and executing draw calls. In this manual we'll cover how to use this API to perform rendering manually. 

To render something using this API you need to:
 - Create and bind a render target
  - Set up a viewport (if rendering to a part of a render target)
  - Clear render target (if we're re-using a render target)
 - Create and bind depth stencil, blend and/or rasterizer states (optionally)
 - Create and bind GPU programs
   - Create and bind vertex/fragment GPU program
   - Create and bind geometry/hull/domain GPU program (optionally)
 - Bind GPU program parameters (if any)
  - Bind textures
  - Bind samplers
  - Bind parameter buffers (better known constant/uniform buffers)
 - Create and bind vertices
  - Create and bind the vertex declaration
  - Create and bind the vertex buffer
 - Create and bind the index buffer (optionally)
 - Issue a draw command
 
We'll cover each step of this process, and at the end also show an alternate pipeline for compute operations.

# Render targets {#renderAPI_a}
Before any rendering can be done you must bind at least one render target. To learn more about, and how to create a render target check out the [render target](@ref renderTargets) manual. 

Binding a render target involves calling @ref BansheeEngine::RenderAPICore::setRenderTarget "RenderAPICore::setRenderTarget". This will cause any rendering to be output to the entirety of the render target. Optionally you can also call @ref BansheeEngine::RenderAPICore::setViewport "RenderAPICore::setViewport" to select a sub-rectangle of the target to render to. 

Binding a render target means you cannot use it for reading within a GPU program. However if your render target has a depth-buffer you can optionally set the `readOnlyDepthStencil` parameter of @ref BansheeEngine::RenderAPICore::setRenderTarget "RenderAPICore::setRenderTarget" to true, which will allow you to have a depth buffer be bound for both depth-testing and reading from the GPU program.
 
Before doing any rendering it's always good to clear the render target to some default value. Use @ref BansheeEngine::RenderAPICore::clearRenderTarget "RenderAPICore::clearRenderTarget" to clear the entire target, or @ref BansheeEngine::RenderAPICore::clearViewport "RenderAPICore::clearViewport" to clear just the viewport portion of the target. When clearing you can choose whether to clear color, depth or stencil buffers (or all) determined by @ref BansheeEngine::FrameBufferType "FrameBufferType" flags. You can also choose the values to clear each of the buffers to. And finally if your render target has multiple surfaces, you can choose to clear only some of the surfaces by providing a bitmask.

Once you are done rendering make sure to call @ref BansheeEngine::RenderAPICore::swapBuffers "RenderAPICore::swapBuffers" if your render target has multiple buffers (like a window). This will swap the buffers and present the rendered image on the screen.

A simple example covering all these commands:
~~~~~~~~~~~~~{.cpp}
SPtr<RenderTarget> myRt = ...; // Assuming we created this earlier, and it's a double-buffered window

RenderAPICore& rapi = RenderAPICore::instance();
rapi.setRenderTarget(myRt);
rapi.setViewport(Rect2(0.5f, 0.0f, 0.5f, 1.0f)); // Draw only to the right side of the target

// Clear all buffers: color to white, depth to 1.0, stencil to 0
rapi.clearViewport(FBT_COLOR | FBT_DEPTH | FBT_STENCIL, Color::White, 1.0f, 0);
... execute some draw calls ...
rapi.swapBuffers();
~~~~~~~~~~~~~
 
# Pipeline states {#renderAPI_b}
Before executing the drawing operation you can optionally set up depth-stencil, rasterizer or blend states. If you do not set them, the default values will be used. The states allow you to customize the execution of the fixed (non-programmable) part of the GPU pipeline.

States can be created by:
 - @ref BansheeEngine::DepthStencilStateCore "DepthStencilStateCore" - Populate @ref BansheeEngine::DEPTH_STENCIL_STATE_DESC "DEPTH_STENCIL_STATE_DESC" and call @ref BansheeEngine::DepthStencilStateCore::create "DepthStencilStateCore::create" 
 - @ref BansheeEngine::BlendStateCore "BlendStateCore" - Populate @ref BansheeEngine::BLEND_STATE_DESC "BLEND_STATE_DESC" and call @ref BansheeEngine::BlendStateCore::create "BlendStateCore::create" 
 - @ref BansheeEngine::RasterizerStateCore "RasterizerStateCore" - Populate @ref BansheeEngine::RASTERIZER_STATE_DESC "RASTERIZER_STATE_DESC" and call @ref BansheeEngine::RasterizerStateCore::create "RasterizerStateCore::create" 
 
They can then be bound to the pipeline by calling:
 - @ref BansheeEngine::RenderAPICore::setDepthStencilState "RenderAPICore::setDepthStencilState" for @ref BansheeEngine::DepthStencilStateCore "DepthStencilStateCore"
 - @ref BansheeEngine::RenderAPICore::setBlendState "RenderAPICore::setBlendState" for @ref BansheeEngine::BlendStateCore "BlendStateCore".
 - @ref BansheeEngine::RenderAPICore::setRasterizerState "RenderAPICore::setRasterizerState" for @ref BansheeEngine::RasterizerStateCore "RasterizerStateCore".

We won't explain what each of the state's does. For that you can check out the class documentation of the states themselves, or familiarize yourself with the modern GPU pipeline in general, as the states mirror it exactly.

# GPU programs {#renderAPI_c}
Before drawing you must bind at least a vertex and a fragment GPU program. Optionally you can also bind geometry, hull or domain programs. While pipeline states allow you to control the fixed portions of the pipeline, GPU programs are fully programmable. To learn how to create GPU programs read the [manual](@ref gpuPrograms).

You can bind a GPU program by calling @ref BansheeEngine::RenderAPICore::bindGpuProgram() "RenderAPICore::bindGpuProgram". You can also use @ref BansheeEngine::RenderAPICore::unbindGpuProgram() "RenderAPICore::unbindGpuProgram" to remove the GPU program from the pipeline, or @ref BansheeEngine::RenderAPICore::isGpuProgramBound() "RenderAPICore::isGpuProgramBound" to check if one is currently bound to a specific slot.

For example:
~~~~~~~~~~~~~{.cpp}
SPtr<GpuProgramCore> fragmentProgram = ...;
SPtr<GpuProgramCore> vertexProgram = ...;

RenderAPICore& rapi = RenderAPICore::instance();
rapi.bindGpuProgram(vertexProgram);
rapi.bindGpuProgram(fragmentProgram);
... execute some draw calls ...

~~~~~~~~~~~~~

With most GPU programs you will also likely want to customize their execution by assigning them some parameters.

## GPU program parameters {#renderAPI_c_a}
Most GPU programs also accept a number of parameters, whether textures, buffers, sampler states or primitive values like floats or integers. Parameters can be split into two categories:
 - Object - Textures, sampler states
 - Data - Float, int, bool (and their vectors and arrays)
 
In order to find out which parameters does a certain GPU program expect, as well as their type and other information, read the [GPU program](@ref gpuPrograms) manual. 
 
### Object parameters {#renderAPI_c_a_a} 
To assign a texture to a GPU program call @ref BansheeEngine::RenderAPICore::setTexture "RenderAPICore::setTexture". You will need to provide a GPU program type to assign the texture to, and a slot to bind the texture in. Each GPU program will provide a list of slots onto which you can bind textures in its parameter descriptor. 

By default the texture will be bound for sampling. You can customize how is the texture sampled by creating a @ref BansheeEngine::SamplerStateCore "SamplerStateCore", by populating the @ref BansheeEngine::SAMPLER_STATE_DESC "SAMPLER_STATE_DESC" and calling @ref BansheeEngine::SamplerStateCore::create "SamplerStateCore::create". The state can then be bound to the pipeline by calling @ref BansheeEngine::RenderAPICore::setSamplerState "RenderAPICore::setSamplerState" which accepts similar parameters to @ref BansheeEngine::RenderAPICore::setTexture "RenderAPICore::setTexture".

You can also bind a texture for both reading and writing by calling @ref BansheeEngine::RenderAPICore::setLoadStoreTexture "RenderAPICore::setLoadStoreTexture". Such textures are known as unordered-access (UAV) in DirectX, and load-store in OpenGL and are often used for compute operations (explained later). Be aware that the texture must have been created with the @ref BansheeEngine::TU_LOADSTORE "TU_LOADSTORE" flag in order to be bindable as a load-store texture. Load-store textures also cannot have multiple surfaces (i.e. faces or mip-maps) so you must choose which one to bind when calling @ref BansheeEngine::RenderAPICore::setLoadStoreTexture "RenderAPICore::setLoadStoreTexture".
 
To learn more about textures read the [texture](@ref textures) manual.

An example binding a texture and its sampler state:
~~~~~~~~~~~~~{.cpp}

// Use nearest neighbor filtering when sampling
SAMPLER_STATE_DESC ssDesc;
ssDesc.magFilter = FO_POINT;
ssDesc.minFilter = FO_POINT;
ssDesc.mipFilter = FO_POINT;

SPtr<SamplerStateCore> mySamplerState = SamplerStateCore::create(ssDesc);
SPtr<TextureCore> myTexture = ...;

RenderAPICore& rapi = RenderAPICore::instance();

... bind GPU program ...

// Bind the texture and sampler state to the 0th slot of the currently bound fragment program
rapi.setTexture(GPT_FRAGMENT_PROGRAM, 0, myTexture);
rapi.setSamplerState(GPT_FRAGMENT_PROGRAM, 0, mySamplerState);

... execute some draw calls ...

~~~~~~~~~~~~~

### Data parameters {#renderAPI_c_a_b}
Data parameters are bound to calling @ref BansheeEngine::RenderAPICore::setParamBuffer "RenderAPICore::setParamBuffer". You can manually populate the parameter buffer with required values, but generally you should use either @ref BansheeEngine::GpuParamsCore "GpuParamsCore" or @ref BansheeEngine::MaterialCore "MaterialCore" to set the parameters and then retrieve the existing parameter buffer.

Each buffer need to be bound to a specific slot, and these slots can be read from the @ref BansheeEngine::GpuParamDesc "GpuParamDesc" structure accessible from @ref BansheeEngine::GpuProgramCore::getParamDesc "GpuProgramCore::getParamDesc". [GPU program](@ref gpuPrograms) manual has more information about GPU parameters.

Alternatively you can call @ref BansheeEngine::RendererUtility::setGpuParams "RendererUtility::setGpuParams" helper method which will bind both object and data parameters in the @ref BansheeEngine::GpuParamsCore "BansheeEngine::GpuParamsCore" object.

You can also call @ref BansheeEngine::RendererUtility::setPassParams "RendererUtility::setPassParams" to bind parameters for all GPU programs in a specific @ref BansheeEngine::MaterialCore "MaterialCore" pass.

For example:
~~~~~~~~~~~~~{.cpp}
SPtr<GpuProgramCore> program = ...;
SPtr<GpuParamsCore> params = program->createParameters();

... set param values ...
... bind GPU program ...

// Set all parameters for the currently bound fragment program
gRendererUtility().setGpuParams(GPT_FRAGMENT_PROGRAM, params);

... execute some draw calls ...

~~~~~~~~~~~~~

# Vertex buffer {#renderAPI_d}
@ref BansheeEngine::VertexBufferCore "Vertex buffer" is a buffer that contains all vertices of the object we wish to render. When drawing the vertices will be interpreted as primitives (either points, lines or triangles) and rendered. Each vertex can have one or multiple properties associated with it.

To create a vertex buffer call @ref BansheeEngine::VertexBufferCore::create "VertexBufferCore::create". You need to know the size of an individual vertex (determined by the properties each vertex requires) and the number of vertices. Optionally if your vertex buffer is used for output from the geometry GPU program you can toggle on the `streamOut` parameter.

Once the vertex buffer is created you will want to populate it with some data (detailed below) and then bind it to the pipeline using @ref BansheeEngine::RenderAPICore::setVertexBuffers "RenderAPICore::setVertexBuffers". You can bind one or multiple vertex buffers at once. They all must have the same vertex counts but can have different properties, which will all be fed to the pipeline when rendering.

Creation of an example vertex buffer:
~~~~~~~~~~~~~{.cpp}
// Create a vertex buffer containing 8 vertices with just a vertex position
SPtr<VertexBufferCore> vb = VertexBufferCore::create(sizeof(Vector3), 8);

RenderAPICore& rapi = RenderAPICore::instance();
rapi.setVertexBuffers(0, { vb });
~~~~~~~~~~~~~

## Reading/writing {#renderAPI_d_a}
@ref BansheeEngine::VertexBufferCore "VertexBuffer" provides a couple of way to read and write data from/to it:
 - @ref BansheeEngine::VertexBufferCore::lock "VertexBufferCore::lock" locks a specific region of the vertex buffer and returns a pointer you can then use for reading and writing. Make sure to specify valid @ref BansheeEngine::GpuLockOptions "GpuLockOptions" signaling whether you are planning on read or writing from the buffer. Once done call @ref BansheeEngine::VertexBufferCore::unlock "VertexBufferCore::unlock" to make the locked region accessible to the GPU again.
 - @ref BansheeEngine::VertexBufferCore::readData "VertexBufferCore::readData" and @ref BansheeEngine::VertexBufferCore::writeData "VertexBufferCore::writeData" to write or read entire blocks at once, but are more or less similar to the previous method.
 - @ref BansheeEngine::VertexBufferCore::copyData "VertexBufferCore::copyData" can be used to efficiently copy data between two vertex buffers.

An example of writing to the vertex buffer:
~~~~~~~~~~~~~{.cpp}
// Create a vertex buffer containing 8 vertices with just a vertex position
SPtr<VertexBufferCore> vb = VertexBufferCore::create(sizeof(Vector3), 8);

Vector3* positions = (Vector3)vb->lock(0, sizeof(Vector3) * 8, GBL_WRITE_ONLY_DISCARD);
... write to the positions array ...
vb->unlock();
~~~~~~~~~~~~~

When your vertices contain multiple properties it can be difficult to keep track of which offset to write which property, or determine the stride between two vertices. For this purpose you can use @ref BansheeEngine::VertexDataDesc "VertexDataDesc" which allows you to easily set up vertex properties like so:
~~~~~~~~~~~~~{.cpp}
// Create a vertex with a position, normal and UV coordinates
SPtr<VertexDataDesc> vertexDesc = VertexDataDesc::create();
vertexDesc->addVertElem(VET_FLOAT3, VES_POSITION);
vertexDesc->addVertElem(VET_FLOAT3, VES_NORMAL);
vertexDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD);
~~~~~~~~~~~~~

You can then use methods like @ref BansheeEngine::VertexDataDesc::getElementSize "VertexDataDesc::getElementSize" to learn the size of a particular element, @ref BansheeEngine::VertexDataDesc::getVertexStride "VertexDataDesc::getVertexStride" to learn the stride between elements. You can also retrieve detailed per-property information by iterating over all properties with @ref BansheeEngine::VertexDataDesc::getNumElements "VertexDataDesc::getNumElements" and @ref BansheeEngine::VertexDataDesc::getElement "VertexDataDesc::getElement". These methods return a @ref BansheeEngine::VertexElement "VertexElement" which can be used for finding out the offset of the individual element.

To learn more about vertex descriptors read the [mesh](@ref meshes) manual.

## Vertex declaration {#renderAPI_d_b}
Before a vertex buffer can be used for rendering, you need to tell the pipeline in what format are its vertices structured in. You do that by creating a @ref BansheeEngine::VertexDeclarationCore "VertexDeclarationCore" object using the @ref BansheeEngine::VertexDataDesc "VertexDataDesc" we described in the previous section. This object can then be passed to @ref BansheeEngine::RenderAPICore::setVertexDeclaration "RenderAPICore::setVertexDeclaration" to bind it to the pipeline.

For example:
~~~~~~~~~~~~~{.cpp}
SPtr<VertexDataDesc> vertexDesc = ...; // Creating vertex desc as above
SPtr<VertexDeclarationCore> vertexDecl = VertexDeclarationCore::create(vertexDesc);

RenderAPICore& rapi = RenderAPICore::instance();
rapi.setVertexDeclaration(vertexDecl);
~~~~~~~~~~~~~

It is important that the vertex declaration contains properties needed by the bound vertex GPU program, as well as that it matches the vertex layout in the vertex buffer. See the [gpu program](@ref gpuPrograms) manual to learn how to retrieve vertex properties expected by a GPU program.

# Index buffer {#renderAPI_e}
Normally when you draw data from a vertex buffer, the vertices are assumed to form primitives sequentially (e.g. every three vertices is a triangle). By using an @ref BansheeEngine::IndexBufferCore "index buffer" you an provide an additional layer of abstraction. Index buffer is fully optional, but when bound it will be used for forming primitives instead of the vertex buffer (i.e. every three indices will form a triangle). Each entry in an index buffer points to a vertex in the vertex buffer. This level of abstraction allows you to re-use the same vertex in multiple primitives, as well as create more optimal vertex order for GPU processing.

To create an index buffer call @ref BansheeEngine::IndexBufferCore::create "IndexBufferCore::create". It expects a number of indices, and the type of indices. Index type can be either 16- or 32-bit. To bind an index buffer to the pipeline call @ref BansheeEngine::RenderAPICore::setIndexBuffer "RenderAPICore::setIndexBuffer".

Reading and writing from/to the index buffer has the identical interface to the vertex buffer, so we won't show it again.

# Drawing {#renderAPI_f}
Once all the previous states, programs and buffers have been set up, we can finally render our object. First of we must set up the type of primitives we wish to render by calling @ref BansheeEngine::RenderAPICore::setDrawOperation "RenderAPICore::setDrawOperation" with a @ref BansheeEngine::DrawOperationType "DrawOperationType" specifying the primitive type. This determines how the the vertices (or indices) in our buffers interpreted.

After that you can issue a @ref BansheeEngine::RenderAPICore::draw "RenderAPICore::draw" call if rendering without an index buffer. It expects the vertex index to start rendering from, and the number of vertices to render. The number of vertices must be divisible by the number of vertices expected by the @ref BansheeEngine::DrawOperationType "DrawOperationType" you're using (e.g. three for triangles, two for lines, one for points). The vertices will then be pulled from the vertex buffers, processed by the fixed pipeline controlled by the states, and by the programmable pipeline controlled by the GPU programs and the output image will be rendered to the bound render target.

If using an index buffer you should issue a @ref BansheeEngine::RenderAPICore::drawIndexed "RenderAPICore::drawIndexed" call. Aside from vertex offset/count, it also expects an offset into the index buffer to start rendering from, and number of indices to render from. In this case the vertex offset will be added to every read index, allowing you to re-use the index buffer for potentially different geometry. 
 
And this wraps up the rendering pipeline. After this step your object should be rendered to your render target and ready to display. 
 
# Compute {#renderAPI_g}
The compute pipeline is a very simple pipeline that can be used for general purpose calculations. It is separate from the render pipeline we have been describing so far, but uses the same functionality, just in a more limited way. You don't have to set states, render targets, vertex/index buffers and only one GPU program type is supported (compute GPU program).

When the pipeline is set up you can execute it by calling @ref BansheeEngine::RenderAPICore::dispatchCompute "RenderAPICore::dispatchCompute". You should provide it a three dimensional number that determines how many instances of the currently bound GPU program to execute. The total number of executions will be X * Y * Z.

Since compute pipeline doesn't support render targets, you will want to use load-store textures for output. An example of a simple compute pipeline:
~~~~~~~~~~~~~{.cpp}
SPtr<GpuProgramCore> computeProgram = ...;
SPtr<TextureCore> loadStoreTexture = ...;

RenderAPICore& rapi = RenderAPICore::instance();
rapi.bindGpuProgram(computeProgram);
rapi.setLoadStoreTexture(GPT_COMPUTE_PROGRAM, 0, true, loadStoreTexture, TextureSurface(0, 0));
rapi.dispatchCompute(512, 512);

... read from our texture to get the result ...
~~~~~~~~~~~~~

We won't go any deeper about the details of the compute pipeline as this information can be found by learning about the GPU pipeline in general from other sources.

# API specifics {#renderAPI_h}
@ref BansheeEngine::RenderAPI "RenderAPI" can be internally implemented by a variety of actual rendering API's like DirectX or OpenGL. Most of the functionality is shared, but there are always some differences between them to be noted (for example DirectX uses a depth range of [0, 1] while OpenGL uses [-1, 1]). Often those differences can be important for various rendering algorithms.

Use @ref BansheeEngine::RenderAPI::getAPIInfo "RenderAPI::getAPIInfo" to receive the @ref BansheeEngine::RenderAPIInfo "RenderAPIInfo" containing such information, so you may modify your rendering accordingly. 

For convenience a specialized @ref BansheeEngine::RenderAPI::convertProjectionMatrix "RenderAPI::convertProjectionMatrix" method is also provided, which converts a generic engine projection matrix, into a render API specific one.