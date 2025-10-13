---
title: GPU programs
---

Now that we understand how command buffers work, let's explore what operations we can perform with them. The first step in rendering is setting up GPU programs - the programmable parts of the GPU pipeline. In other literature these are often called shaders (the framework uses the word shader for a higher level concept, so we won't call them that).

There are six types of GPU programs: vertex, hull (tesselation control), domain (tesselation evaluation), geometry, fragment (pixel) and compute programs. Each is used for a different purpose but has the same interface. We assume the user is familiar with the GPU pipeline and what the different program types do.

> Note that if you are using Banshee Shading Language you do not need to create GPU programs manually - any shaders you import and materials created from those shaders will have GPU programs created internally, but they will be hidden from the normal user.

# Creating GPU programs
To create a GPU program you need to fill out a @b3d::GpuProgramCreateInformation structure and pass it to the GPU device. The structure needs to have the following fields populated:
 - @b3d::GpuProgramCreateInformation::Source - Source code of the GPU program. This should be in a language supported by the current render API (e.g. HLSL for DirectX, GLSL for Vulkan).
 - @b3d::GpuProgramCreateInformation::EntryPoint - Name of the entry point into the GPU program. This is the name of the function that will be called when the program is ran. Must be "main" for Vulkan.
 - @b3d::GpuProgramCreateInformation::Language - Language the source code is written in. This can be "hlsl" or "vksl".
 - @b3d::GpuProgramCreateInformation::Type - @b3d::GpuProgramType of the GPU program (vertex, fragment, etc.).

GPU programs are created on the GPU device. You access the GPU device through a @b3d::render::GpuCommandBuffer object.

For example if we wanted to create a HLSL fragment program (HLSL source not shown):
~~~~~~~~~~~~~{.cpp}
String hlslSource = "...";

GpuProgramCreateInformation createInformation;
createInformation.Type = GPT_FRAGMENT_PROGRAM;
createInformation.Source = hlslSource;
createInformation.EntryPoint = "main";
createInformation.Language = "hlsl";

// Assuming you have a command buffer
SPtr<GpuCommandBuffer> commandBuffer = ...;
SPtr<GpuProgram> myProgram = commandBuffer->GetGpuDevice().CreateGpuProgram(createInformation);
~~~~~~~~~~~~~

Once the GPU program has been created it is not guaranteed to be usable. The compilation of the provided source code could have failed, which you can check by calling @b3d::GpuProgram::IsCompiled, and retrieve the error message by calling @b3d::GpuProgram::GetCompileErrorMessage.

~~~~~~~~~~~~~{.cpp}
if(!myProgram->IsCompiled())
	B3D_LOG(Error, Uncategorized, "GPU program compilation failed with error: {0}", myProgram->GetCompileErrorMessage());
~~~~~~~~~~~~~

Be aware that shader compilation happens on the render thread. Therefore if calling these methods on the simulation thread GPU program, you must first ensure the GPU program's render proxy is initialized by calling @b3d::CoreObject::BlockUntilRenderProxyInitialized.

~~~~~~~~~~~~~{.cpp}
// If program is used on simulation thread
myProgram->BlockUntilRenderProxyInitialized();

if(!myProgram->IsCompiled())
	B3D_LOG(Error, Uncategorized, "GPU program compilation failed with error: {0}", myProgram->GetCompileErrorMessage());
~~~~~~~~~~~~~

# Using GPU programs for rendering
To use a GPU program in a draw or dispatch call, you must first create a GPU pipeline object using the relevant GPU programs. There are two types of pipeline states:
 - @b3d::GpuGraphicsPipelineState - Supports vertex, hull, domain, geometry and fragment programs. At minimum requires a vertex program, while most pipelines will use vertex & fragment programs.
 - @b3d::GpuComputePipelineState - Supports only the compute GPU program type.

To create a graphics pipeline you need to fill out the @b3d::GpuGraphicsPipelineStateCreateInformation structure with references to relevant GPU programs, followed by a call to GPU device's CreateGpuGraphicsPipelineState method.

~~~~~~~~~~~~~{.cpp}
GpuGraphicsPipelineStateCreateInformation createInformation;
createInformation.VertexProgram = ...;
createInformation.FragmentProgram = myProgram; // Program we created in the example above
createInformation.GeometryProgram = ...;
createInformation.HullProgram = ...;
createInformation.DomainProgram = ...;

SPtr<GpuCommandBuffer> commandBuffer = ...;
SPtr<GpuGraphicsPipelineState> graphicsPipeline = commandBuffer->GetGpuDevice().CreateGpuGraphicsPipelineState(createInformation);
~~~~~~~~~~~~~

> Note that graphics pipelines also support a set of fixed (non-programmable) states we'll discuss later.

Compute pipeline states are simpler, accepting just a single compute GPU program as a parameter to the GPU device's CreateGpuComputePipelineState method.

~~~~~~~~~~~~~{.cpp}
SPtr<GpuProgram> computeProgram = ...;

GpuComputePipelineStateCreateInformation createInformation;
createInformation.Program = computeProgram;

SPtr<GpuCommandBuffer> commandBuffer = ...;
SPtr<GpuComputePipelineState> computePipeline = commandBuffer->GetGpuDevice().CreateGpuComputePipelineState(createInformation);
~~~~~~~~~~~~~

Once created pipelines can be bound for rendering through the @b3d::render::GpuCommandBuffer interface. The command buffer is the primary entry point in the low-level rendering API and it will be used for most low-level rendering operations, as we'll see throughout this set of manuals.

Call @b3d::render::GpuCommandBuffer::SetGpuGraphicsPipelineState or @b3d::render::GpuCommandBuffer::SetGpuComputePipelineState to bind a graphics or a compute pipeline state, respectively.

~~~~~~~~~~~~~{.cpp}
// Bind pipeline for use
SPtr<GpuCommandBuffer> commandBuffer = ...;
commandBuffer->SetGpuGraphicsPipelineState(graphicsPipeline);
// Or: commandBuffer->SetGpuComputePipelineState(computePipeline);
~~~~~~~~~~~~~

Once bound any *draw* or *dispatch* calls will be executed using the bound pipeline states. Draw/dispatch calls are explained in more detail later on.

# GPU program parameters
Although you can use a GPU program without any parameters, most will require some additional data in order to perform their operations. Program parameters represent data that is static throughout a single GPU program execution (a draw/dispatch call). For example, when drawing a 3D object you will usually want to provide a projection matrix that transforms the object from 3D to 2D, according to the camera the user is viewing the object through.

You can access information about GPU program parameters by calling @b3d::GpuProgram::GetParameterDescription. This will return a @b3d::GpuProgramParameterDescription structure containing information about all GPU parameters used by that GPU program. This includes primitives (int, float, etc.), textures, samplers, buffers and uniform buffers (constant buffers in DirectX or uniform blocks in OpenGL/Vulkan).

You generally don't need to use this information directly. It is instead automatically parsed when you create a GPU pipeline. Once you have a pipeline you can use it to create a @b3d::render::GpuParameters object that allows you to assign values to all parameters of a specific pipeline.

## GpuParameters
**GpuParameters** is a container for all parameters required by a single GPU pipeline (graphics or compute). It allows you to set textures, samplers, buffers, and primitive values used by GPU programs. Parameter values are stored on the CPU side and are only submitted to the GPU when the parameters are bound to the command buffer during rendering.

### Creating GpuParameters
To create a **GpuParameters** object, use the GPU device's @b3d::GpuDevice::CreateGpuParameters method with either a graphics or a compute pipeline state as a parameter.

~~~~~~~~~~~~~{.cpp}
SPtr<GpuDevice> device = ...;
SPtr<GpuGraphicsPipelineState> graphicsPipeline = ...;
SPtr<GpuParameters> parameters = device->CreateGpuParameters(graphicsPipeline);
~~~~~~~~~~~~~

The created **GpuParameters** object will be initialized with parameter layout information from the pipeline, allowing the system to validate parameter assignments and manage internal storage.

### Setting parameters by name
Once created you can assign values to parameters by calling any of the following methods (depending on parameter type):
 - @b3d::render::GpuParameters::SetSampledTexture - Assigns a read-only (sampled) texture to a GPU program.
 - @b3d::render::GpuParameters::SetStorageTexture - Assign a load-store (writable) texture to a GPU program.
 - @b3d::render::GpuParameters::SetStorageBuffer - Assigns a storage buffer (either read-only or read-write) to a GPU program.
 - @b3d::render::GpuParameters::SetSamplerState - Assigns a sampler state that determines how is a sampled texture read by the shader.
 - @b3d::render::GpuParameters::SetParameter<T> - Assigns a primitive type like *float*, *int*, **Vector3**, **Matrix4** or others.
 - @b3d::render::GpuParameters::SetUniformBuffer - Assigns a uniform buffer that contains primitive parameters (discussed in detail below).

> **Important:** Before you can use **SetParameter** to set primitive values, you must first bind a uniform buffer using **SetUniformBuffer**. Primitive parameters are written into the bound uniform buffer, not stored directly in GpuParameters. Textures, storage buffers, and sampler states do not have this requirement.

Supported primitive types for **SetParameter** include:
 - *float*, *double*
 - **Vector2**, **Vector3**, **Vector4**
 - **Vector2D**, **Vector3D**, **Vector4D** (double precision)
 - *i32*, *u32*
 - **Vector2I**, **Vector3I**, **Vector4I**
 - **Vector2UI**, **Vector3UI**, **Vector4UI**
 - **Matrix2**, **Matrix3**, **Matrix4** and variants (**Matrix2x3**, **Matrix2x4**, **Matrix3x2**, **Matrix3x4**, **Matrix4x2**, **Matrix4x3**)
 - **Color** (maps to 4-component float in GPU program)

Each of the methods accepts the name of the parameter (as specified in the GPU program code) and the value to assign.

~~~~~~~~~~~~~{.cpp}
// NOTE: Before setting primitive parameters, you must bind a uniform buffer first!
// See the "Uniform buffers" section below for details.

// Set a primitive parameter (writes to the bound uniform buffer)
Matrix4 viewProjectionMatrix = ...;
parameters->SetParameter("viewProjMatrix", viewProjectionMatrix);

// Set a sampled texture (no uniform buffer required)
SPtr<Texture> diffuseTexture = ...;
parameters->SetSampledTexture("mainTexture", diffuseTexture);

// Set a sampler state (no uniform buffer required)
SPtr<SamplerState> linearSampler = ...;
parameters->SetSamplerState("mainSampler", linearSampler);
~~~~~~~~~~~~~

You can also bind to specific texture surfaces (e.g. mip levels, array slices) by providing a @b3d::TextureSurface parameter:

~~~~~~~~~~~~~{.cpp}
// Bind a specific mip level of a texture
TextureSurface surface(0, 1, 2, 1); // Mip level 2, one mip, face 0, one face
parameters->SetSampledTexture("mainTexture", diffuseTexture, surface);
~~~~~~~~~~~~~

### Checking parameter existence
Before setting a parameter, you can check if it exists in the pipeline using the **Has*** methods:

~~~~~~~~~~~~~{.cpp}
if(parameters->HasParameter("viewProjMatrix"))
	parameters->SetParameter("viewProjMatrix", viewProjectionMatrix);

if(parameters->HasSampledTexture("mainTexture"))
	parameters->SetSampledTexture("mainTexture", diffuseTexture);

if(parameters->HasUniformBuffer("PerObjectData"))
	parameters->SetUniformBuffer("PerObjectData", uniformBuffer);
~~~~~~~~~~~~~

Additional check methods include @b3d::render::GpuParameters::HasStorageTexture, @b3d::render::GpuParameters::HasStorageBuffer, and @b3d::render::GpuParameters::HasSamplerState.

### Using parameter handles
If parameters are modified often you can use *parameter handles* for faster access. Handles avoid the name lookup that occurs when setting parameters directly:
 - @b3d::render::GpuParameters::GetSampledTextureParameter - Outputs a handle that can be used for reading & writing the parameter value.
 - @b3d::render::GpuParameters::GetStorageTextureParameter - Outputs a handle that can be used for reading & writing the parameter value.
 - @b3d::render::GpuParameters::GetStorageBufferParameter - Outputs a handle that can be used for reading & writing the parameter value.
 - @b3d::render::GpuParameters::GetSamplerStateParameter - Outputs a handle that can be used for reading & writing the parameter value.
 - @b3d::render::GpuParameters::GetParameter<T> - Outputs a handle that can be used for reading & writing the parameter value.

Each method accepts the parameter name and outputs a handle. Handles provide **Set()** and **Get()** methods for reading and writing parameter values. They can be retrieved once and reused, avoiding repeated name lookups.

~~~~~~~~~~~~~{.cpp}
// Retrieve parameter handles
TGpuParameterPrimitive<Matrix4, false> matrixParameter;
TGpuParameterSampledTexture<false> textureParameter;

parameters->GetParameter("viewProjMatrix", matrixParameter);
parameters->GetSampledTextureParameter("mainTexture", textureParameter);

// Later in performance-critical code, use handles to set values quickly
Matrix4 viewProjectionMatrix = ...;
SPtr<Texture> someTexture = ...;

matrixParameter.Set(viewProjectionMatrix);
textureParameter.Set(someTexture);
~~~~~~~~~~~~~

For cases where a parameter may not exist, use the **Try*** variants which return *false* instead of logging warnings:

~~~~~~~~~~~~~{.cpp}
TGpuParameterPrimitive<Matrix4, false> optionalParameter;
if(parameters->TryGetParameter("optionalMatrix", optionalParameter))
{
	optionalParameter.Set(myMatrix);
}
~~~~~~~~~~~~~

The **Try*** methods are available for all parameter types: @b3d::render::GpuParameters::TryGetParameter, @b3d::render::GpuParameters::TryGetSampledTextureParameter, @b3d::render::GpuParameters::TryGetStorageTextureParameter, @b3d::render::GpuParameters::TryGetStorageBufferParameter, and @b3d::render::GpuParameters::TryGetSamplerStateParameter.

## Uniform buffers
Primitive parameters (*int*, *float*, **Vector3**, **Matrix4**, etc.) in GPU programs are grouped in uniform buffers. These are memory blocks that store multiple related parameters together, better known as *constant buffers* in DirectX or *uniform blocks* in OpenGL/Vulkan.

### Understanding uniform buffers and primitive parameters
When you call **SetParameter** on primitive types, the framework writes those values into a uniform buffer that must be bound to the **GpuParameters** object. The relationship works as follows:

1. Your GPU program declares uniform buffers (e.g., `cbuffer PerObjectData` in HLSL or `layout(binding=0) uniform PerObjectData` in GLSL)
2. Each uniform buffer contains one or more primitive parameters
3. You must create a **GpuBuffer** and bind it to **GpuParameters** using **SetUniformBuffer** **before** calling **SetParameter**
4. Once bound, calling **SetParameter** writes values into the appropriate uniform buffer

> **Important:** Calling **SetParameter** without first binding a uniform buffer will result in the parameter write being ignored or causing errors. The framework needs to know which uniform buffer to write to.

### Manually creating and binding uniform buffers
You can manually create uniform buffers when you need fine control over memory layout:

~~~~~~~~~~~~~{.cpp}
// Create a GPU buffer to use as a uniform buffer
GpuBufferCreateInformation createInformation;
createInformation.Type = GBT_UNIFORM;
createInformation.Size = sizeof(MyUniformData);
createInformation.Usage = GBU_DYNAMIC;

SPtr<GpuBuffer> uniformBuffer = device->CreateGpuBuffer(createInformation);

// Write data to the buffer directly
MyUniformData data = ...;
uniformBuffer->WriteData(0, sizeof(MyUniformData), &data);

// Bind the uniform buffer by set and slot
parameters->SetUniformBuffer(0, 0, uniformBuffer);

// Or bind by name (matches uniform block name in shader)
parameters->SetUniformBuffer("PerObjectData", uniformBuffer);
~~~~~~~~~~~~~

When manually creating uniform buffers, you are responsible for ensuring the buffer layout matches what the GPU program expects, including proper alignment and padding according to the render API's rules.

You can also use the **Get*** methods to retrieve currently bound uniform buffers:

~~~~~~~~~~~~~{.cpp}
// Get uniform buffer bound at set 0, slot 0
SPtr<GpuBuffer> buffer = parameters->GetUniformBuffer(0, 0);
~~~~~~~~~~~~~

### Parameter block definitions
Manually creating uniform buffers and ensuring correct memory layout can be error-prone. The framework provides a set of helper macros that make it easier to define and work with uniform buffers by automatically handling layout calculations.

Use @b3d::B3D_PARAM_BLOCK_BEGIN, @b3d::B3D_PARAM_BLOCK_ENTRY and @b3d::B3D_PARAM_BLOCK_END macros to define a parameter block structure that corresponds to a uniform buffer in your GPU program:

~~~~~~~~~~~~~{.cpp}
B3D_PARAM_BLOCK_BEGIN(MyParamDef)
	B3D_PARAM_BLOCK_ENTRY(Matrix4, viewProjMatrix)
	B3D_PARAM_BLOCK_ENTRY(Color, tintColor)
	B3D_PARAM_BLOCK_ENTRY(Vector2, screenSize)
B3D_PARAM_BLOCK_END

// Declare a global instance (typically in a .cpp file)
MyParamDef gMyParamDef;
~~~~~~~~~~~~~

The macro generates a structure with methods for creating buffers and setting parameter values. Each entry becomes a member that knows its offset and layout within the buffer.

To use the parameter block definition:

~~~~~~~~~~~~~{.cpp}
// Create a uniform buffer using the parameter block definition
SPtr<GpuBuffer> uniformBuffer = gMyParamDef.CreateBuffer();

// Bind the buffer to GpuParameters
parameters->SetUniformBuffer("MyUniformBlock", uniformBuffer);

// Set individual parameter values using the parameter block definition
Matrix4 matrix = ...;
gMyParamDef.viewProjMatrix.Set(uniformBuffer, matrix);
gMyParamDef.tintColor.Set(uniformBuffer, Color::kWhite);
gMyParamDef.screenSize.Set(uniformBuffer, Vector2(1920.0f, 1080.0f));

// Flush cached writes to GPU (if needed before rendering)
uniformBuffer->FlushCache();
~~~~~~~~~~~~~

For array parameters, use @b3d::render::B3D_PARAM_BLOCK_ENTRY_ARRAY:

~~~~~~~~~~~~~{.cpp}
B3D_PARAM_BLOCK_BEGIN(LightingParamDef)
	B3D_PARAM_BLOCK_ENTRY_ARRAY(Vector4, lightPositions, 8)
	B3D_PARAM_BLOCK_ENTRY_ARRAY(Color, lightColors, 8)
	B3D_PARAM_BLOCK_ENTRY(i32, lightCount)
B3D_PARAM_BLOCK_END

// Set array elements by specifying array index
gLightingParamDef.lightPositions.Set(uniformBuffer, lightPos0, 0);
gLightingParamDef.lightPositions.Set(uniformBuffer, lightPos1, 1);
gLightingParamDef.lightColors.Set(uniformBuffer, lightColor0, 0);
~~~~~~~~~~~~~

The parameter block system automatically handles:
- Correct memory layout and alignment according to GPU backend conventions
- Matrix transposition when required by the rendering API
- Array stride calculations
- Buffer size calculation

You can also create multiple instances of the same buffer layout:

~~~~~~~~~~~~~{.cpp}
// Create buffer for multiple objects (useful for instancing)
SPtr<GpuBuffer> multiInstanceBuffer = gMyParamDef.CreateBuffer(100); // 100 instances
~~~~~~~~~~~~~

## Binding GPU parameters
Once **GpuParameters** has been created and populated with necessary data, you can bind it to the GPU by calling @b3d::render::GpuCommandBuffer::SetGpuParameters.

~~~~~~~~~~~~~{.cpp}
// This should be called after the pipeline expecting these parameters is bound
commandBuffer->SetGpuParameters(parameters);
~~~~~~~~~~~~~

## Complete example
Here is a complete example showing how to set up and use GPU programs with parameters:

~~~~~~~~~~~~~{.cpp}
// 1. Create GPU programs
String vertexShader = "..."; // Your vertex shader code
String fragmentShader = "..."; // Your fragment shader code

GpuProgramCreateInformation vertexCreateInfo;
vertexCreateInfo.Type = GPT_VERTEX_PROGRAM;
vertexCreateInfo.Source = vertexShader;
vertexCreateInfo.EntryPoint = "main";
vertexCreateInfo.Language = "hlsl";

GpuProgramCreateInformation fragmentCreateInfo;
fragmentCreateInfo.Type = GPT_FRAGMENT_PROGRAM;
fragmentCreateInfo.Source = fragmentShader;
fragmentCreateInfo.EntryPoint = "main";
fragmentCreateInfo.Language = "hlsl";

SPtr<GpuCommandBuffer> commandBuffer = ...;
SPtr<GpuDevice> gpuDevice = commandBuffer->GetGpuDevice();

SPtr<GpuProgram> vertexProgram = gpuDevice.CreateGpuProgram(vertexCreateInfo);
SPtr<GpuProgram> fragmentProgram = gpuDevice.CreateGpuProgram(fragmentCreateInfo);

// 2. Create pipeline state
GpuGraphicsPipelineStateCreateInformation pipelineInfo;
pipelineInfo.VertexProgram = vertexProgram;
pipelineInfo.FragmentProgram = fragmentProgram;

SPtr<GpuGraphicsPipelineState> pipeline = gpuDevice.CreateGpuGraphicsPipelineState(pipelineInfo);

// 3. Create GpuParameters for the pipeline
SPtr<GpuParameters> parameters = GpuParameters::Create(pipeline);

// 4. Set up uniform buffer using parameter block definition
B3D_PARAM_BLOCK_BEGIN(PerObjectParamDef)
	B3D_PARAM_BLOCK_ENTRY(Matrix4, worldViewProj)
	B3D_PARAM_BLOCK_ENTRY(Color, tintColor)
B3D_PARAM_BLOCK_END

PerObjectParamDef gPerObjectParamDef; // Declare globally

SPtr<GpuBuffer> perObjectBuffer = gPerObjectParamDef.CreateBuffer();
parameters->SetUniformBuffer("PerObjectData", perObjectBuffer);

// 5. Set parameters
gPerObjectParamDef.worldViewProj.Set(perObjectBuffer, myMatrix);
gPerObjectParamDef.tintColor.Set(perObjectBuffer, Color::kWhite);

SPtr<Texture> texture = ...;
parameters->SetSampledTexture("mainTexture", texture);

// 6. Bind pipeline and parameters, then draw
commandBuffer->SetGpuGraphicsPipelineState(pipeline);
commandBuffer->SetGpuParameters(parameters);

// Your draw calls here...
~~~~~~~~~~~~~

# Vertex input
Vertex GPU programs provide information about their inputs in the form of a **VertexDescription**. This is the same structure that we used for describing per-vertex components while creating a mesh. Per-vertex input declaration can be retrieved from a GPU program by calling @b3d::GpuProgram::GetVertexInputDescription.

~~~~~~~~~~~~~{.cpp}
SPtr<GpuProgram> vertexProgram = ...;
SPtr<VertexDescription> inputs = vertexProgram->GetVertexInputDescription();
~~~~~~~~~~~~~

Input declaration can be used for creating meshes or vertex buffers that provide per-vertex information that a GPU program expects.

# GLSL specifics
When declaring vertex inputs for a GPU program written in GLSL you should use the following variable names depending on the input usage:
 - bs_position - Vertex position
 - bs_normal - Vertex normal
 - bs_tangent - Vertex tangent
 - bs_bitangent - Vertex bitangent
 - bs_texcoord - Vertex UV
 - bs_color - Vertex color
 - bs_blendweights - Blend weights used for skinning
 - bs_blendindices - Blend indices used for skinning

This allows the system to map the semantics specified in **VertexDescription** of the bound mesh or vertex buffer to the GPU program inputs. This is not required for HLSL as HLSL has built-in support for semantics which are used instead.

You can append 0-8 to the names to receive more than one element of the same name. Actual types of these elements, as well as the data stored by them doesn't need to match the names and it's up to the user to provide whatever data he needs.
