---
title: GPU programs
---

GPU programs are programmable parts of the GPU pipeline, in other literature often called shaders (bs::f uses the word shader for a higher level concept, so we won't call them that). They are core objects, meaning they can be used on both sim and core threads. We'll focus on the core thread version and note the differences between the two where relevant.

In bs::f they are represented with the @bs::ct::GpuProgram class. There are six types of GPU programs: vertex, hull (tesselation control), domain (tesselation evaluation), geometry, fragment (pixel) and compute programs. Each is used for a different purpose but has the same interface. We assume the user is familiar with the GPU pipeline and what the different program types do. 

> Note that if you are using bs::f Shading Language you do not need to create GPU programs manually - any shaders you import and materials created from those shaders will have GPU programs created internally, but they will be hidden from the normal user.

# Creating GPU programs
To create a GPU program call @bs::ct::GpuProgram::create() with a @bs::GPU_PROGRAM_DESC structure. The structure needs to have the following fields populated:
 - @bs::GPU_PROGRAM_DESC::source - Source code of the GPU program. This should be in a language supported by the current render API (e.g. HLSL for DirectX, GLSL for OpenGL/Vulkan).
 - @bs::GPU_PROGRAM_DESC::entryPoint - Name of the entry point into the GPU program. This is the name of the function that will be called when the program is ran. Must be "main" for OpenGL & Vulkan.
 - @bs::GPU_PROGRAM_DESC::language - Language the source code is written in. This can be "hlsl", "glsl" or "vksl".
 - @bs::GPU_PROGRAM_DESC::type - @bs::GpuProgramType of the GPU program (vertex, fragment, etc.).
 
For example if we wanted to create a HLSL fragment program (HLSL source not shown):
~~~~~~~~~~~~~{.cpp}
String hlslSource = "...";

GPU_PROGRAM_DESC desc;
desc.type = GPT_FRAGMENT_PROGRAM;
desc.source = hlslSource;
desc.entryPoint = "main";
desc.language = "hlsl";

SPtr<GpuProgram> myProgram = GpuProgram::create(desc);
~~~~~~~~~~~~~ 
 
Once the GPU program has been created it is not guaranteed to be usable. The compilation of the provided source code could have failed, which you can check by calling @bs::ct::GpuProgram::isCompiled(), and retrieve the error message by calling @bs::ct::GpuProgram::getCompileErrorMessage(). 

~~~~~~~~~~~~~{.cpp}
if(!myProgram->isCompiled())
	gDebug().logError("GPU program compilation failed with error: " + myProgram->getCompileErrorMessage());
~~~~~~~~~~~~~ 

Be aware that shader compilation happens on the core thread. Therefore if calling these methods on the sim thread GPU program, you must first ensure the GPU program is initialized by calling **CoreThread::submit()** with @p blockUntilComplete parameter set to true, or use the core-object-specific method @bs::CoreObject::blockUntilCoreInitialized.

~~~~~~~~~~~~~{.cpp}
// If program is used on sim thread
myProgram->blockUntilCoreInitialized();

if(!myProgram->isCompiled())
	gDebug().logError("GPU program compilation failed with error: " + myProgram->getCompileErrorMessage());
~~~~~~~~~~~~~ 

# Using GPU programs for rendering
To use a GPU program in a draw or dispatch call, you must first create a GPU pipeline object using the relevant GPU programs. There are two types of pipeline states:
 - @bs::ct::GraphicsPipelineState - Supports vertex, hull, domain, geometry and fragment programs. At minimum requires a vertex program, while most pipelines will use vertex & fragment programs.
 - @bs::ct::ComputePipelineState - Supports only the compute GPU program type.

To create a graphics pipeline you need to fill out the @bs::ct::PIPELINE_STATE_DESC structure with references to relevant GPU programs, followed by a call to @bs::ct::GraphicsPipelineState::create().

~~~~~~~~~~~~~{.cpp}
PIPELINE_STATE_DESC desc;
desc.vertexProgram = ...;
desc.fragmentProgram = myProgram; // Program we created in the example above
desc.geometryProgram = ...;
desc.hullProgram = ...;
desc.domainProgram = ...;

SPtr<GraphicsPipelineState> graphicsPipeline = GraphicsPipelineState::create(desc);
~~~~~~~~~~~~~

> Note that graphics pipelines also support a set of fixed (non-programmable) states we'll discuss later.
 
Compute pipeline states are simpler, accepting just a single compute GPU program as a parameter to their @bs::ct::ComputePipelineState::create() method.

~~~~~~~~~~~~~{.cpp}
SPtr<GpuProgram> computeProgram = ...;
SPtr<ComputePipelineState> computePipeline = ComputePipelineState::create(computeProgram);
~~~~~~~~~~~~~

Once created pipelines can be bound for rendering through the globally accessible @bs::ct::RenderAPI interface. This interface is the primary entry point in the low-level rendering API and it will be used for most low-level rendering operations, as we'll see throughout this set of manuals.

Call @bs::ct::RenderAPI::setGraphicsPipeline or @bs::ct::RenderAPI::setComputePipeline to bind a graphics or a compute pipeline state, respectively.

~~~~~~~~~~~~~{.cpp}
// Bind pipeline for use

RenderAPI& rapi = RenderAPI::instance();
rapi.setGraphicsPipeline(graphicsPipeline);
// Or: rapi.setComputePipeline(computePipeline);
~~~~~~~~~~~~~

Once bound any *draw* or *dispatch* calls will be executed using the bound pipeline states. Draw/dispatch calls are explained in more detail later on.

# GPU program parameters
Although you can use a GPU program without any parameters, most will require some additional data in order to perform their operations. Program parameters represent data that is static throughout a single GPU program execution (a draw/dispatch call). For example, when drawing a 3D object you will usually want to provide a projection matrix that transforms the object from 3D to 2D, according to the camera the user is viewing the object through.

You can access information about GPU program parameters by calling @bs::ct::GpuProgram::getParamDesc. This will return a @bs::GpuParamDesc structure containing information about all GPU parameters used by that GPU program. This includes primitives (int, float, etc.), textures, samplers, buffers and parameter buffers (constant/uniform buffers in DirectX/OpenGL lingo). 

You generally don't need to use this information directly. It is instead automatically parsed when you create a GPU pipeline. Once you have a pipeline you can use it to create a @bs::ct::GpuParams object that allows you to assign values to all parameters of a specific pipeline.

## GpuParams
**GpuParams** is a container for all parameters required by a single GPU pipeline (graphics or compute). It allows you to set primitive/texture/sampler/buffer parameters used by the GPU programs, which it stores in an internal buffer. You can then bind it
to a **RenderAPI** similar to how you bind the pipeline themselves. Assigned parameter will then be used with the current pipeline in any following *draw* or *dispatch* calls.

To create a **GpuParams** object call @bs::ct::GpuParams::create with either a graphics or a compute pipeline state as a parameter.

~~~~~~~~~~~~~{.cpp}
SPtr<GraphicsPipelineState> graphicsPipeline = ...;
SPtr<GpuParams> params = GpuParams::create(graphicsPipeline);
~~~~~~~~~~~~~

Once created you can assign values to parameter by calling any of the following methods (depending on parameter type):
 - @ref bs::ct::GpuParams::setTexture(GpuProgramType, const String&, const TextureType&, const TextureSurface&) "ct::GpuParams::setTexture()" - Assigns a read-only (sampled) texture to a GPU program.
 - @ref bs::ct::GpuParams::setLoadStoreTexture(GpuProgramType, const String&, const TextureType&, const TextureSurface&) "ct::GpuParams::setLoadStoreTexture()" - Assign a load-store (writable) texture to a GPU program.
 - @ref bs::ct::GpuParams::setBuffer(GpuProgramType, const String&, const BufferType&) "ct::GpuParams::setBuffer()" - Assigns a buffer (either read-only or read-write) to a GPU program.
 - @ref bs::ct::GpuParams::setSamplerState(GpuProgramType, const String&, const SamplerType&) "ct::GpuParams::setSamplerState()" - Assigns a sampler state that determines how is a sampled texture read by the shader.
 - @ref bs::ct::GpuParams::setParam<T>(GpuProgramType, const String&, const T&) "ct::GpuParams::setParam()" - Assigns a primitive type like *float*, *int*, **Vector3**, **Matrix4** or others. Supported primitive types are:
  - *float*
  - **Vector2**
  - **Vector3**
  - **Vector4**
  - *int*
  - **Vector2I**
  - **Vector3I**
  - **Vector4I**
  - **Matrix3**
  - **Matrix4**
  - **Color** (maps to 4-component float in GPU program, same as **Vector4**)
 
Each of the methods accepts a **GpuProgramType** of the program whose parameter to assign, name of the parameter (as specified in the GPU program code) and a value of the parameter.

~~~~~~~~~~~~~{.cpp}
Matrix4 viewProjMat = ...;
params->setParam(GPT_VERTEX_PROGRAM, "vertProjMatrix", viewProjMat);

SPtr<Texture> someTexture = ...;
params->setTexture(GPT_FRAGMENT_PROGRAM, "mainTexture", someTexture);
~~~~~~~~~~~~~

If parameters are modified often you can instead use *parameter handles* for faster access. Use the following methods to retrieve a handle to the parameter:
 - @bs::ct::GpuParams::getTextureParam - Outputs a @bs::TGpuParamTexture<Core> handle that can be used for reading & writing the parameter value.
 - @bs::ct::GpuParams::getLoadStoreTextureParam - Outputs a @bs::TGpuParamLoadStoreTexture<Core> handle that can be used for reading & writing the parameter value.
 - @bs::ct::GpuParams::getBufferParam - Outputs a @bs::TGpuParamBuffer<Core> handle that can be used for reading & writing the parameter value.
 - @bs::ct::GpuParams::getSamplerStateParam - Outputs a @bs::TGpuParamSampState<Core> handle that can be used for reading & writing the parameter value.
 - @bs::ct::GpuParams::getParam<T> - Outputs a @ref bs::TGpuDataParam<Core, T> "TGpuDataParam<T>" handle that can be used for reading & writing the parameter value.

Each of the methods accepts a **GpuProgramType** of the program whose parameter to assign, name of the parameter (as specified in the GPU program code), and outputs a handle to the parameter as specified above. Handles provide **set()** and **get()** methods that can be used for writing and reading the parameter values.
 
~~~~~~~~~~~~~{.cpp}
// Same result as above, only using handles
GpuParamMat4 myMatParam;
GpuParamTexture myTextureParam;

params->getParam(GPT_VERTEX_PROGRAM, "vertProjMatrix", myMatParam);
params->getTextureParam(GPT_FRAGMENT_PROGRAM, "mainTexture", myTextureParam);

Matrix4 viewProjMat = ...;
SPtr<Texture> someTexture = ...;

myVectorParam.set(viewProjMat);
myTextureParam.set(someTexture);
~~~~~~~~~~~~~

Parameter handles can be retrieved once and stored, then used for quick access to the parameter without an otherwise expensive lookup that happens when setting parameter directly.

## Parameter blocks
All primitive (*int*, *float*, **Vector3**, etc.) GPU parameters are grouped in parameter blocks - each GPU program must have at least one. These are better known as *constant buffers* in DirectX or *uniform blocks* in OpenGL/Vulkan. 

Before we can assign primitive parameters to **GpuParams** we must first allocate their parameter blocks (in the examples above we skipped this step). Parameter blocks are represented by @bs::ct::GpuParamBlockBuffer. They are essentially just blocks of memory and only require a size in bytes in their call to @bs::ct::GpuParamBlockBuffer::create().

~~~~~~~~~~~~~{.cpp}
// Buffer with 256 bytes in size
SPtr<GpuParamBlockBuffer> buffer = GpuParamBlockBuffer::create(256);
~~~~~~~~~~~~~

You can find out which parameter blocks (and their sizes) a GPU program contains from its **GpuParamDesc** structure retrieved from **GpuProgram::getParamDesc()**.

~~~~~~~~~~~~~{.cpp}
// Create the first available parameter block, with adequate size to fit all of its fields
SPtr<GpuParamDesc> paramDesc = myProgram->getParamDesc();

SPtr<GpuParamBlockBuffer> buffer;
if(paramDesc.paramBlocks.size() > 0)
{
	UINT32 sizeBytes = paramDesc.paramBlocks[0].blockSize * 4; // Block size is in multiples of 4 bytes
	buffer = GpuParamBlockBuffer::create(sizeBytes);
}
~~~~~~~~~~~~~

Finally you can bind the parameter block to **GpuParams** by calling @bs::ct::GpuParams::setParamBlockBuffer with the GPU program type, parameter block variable name, and the parameter block buffer.

~~~~~~~~~~~~~{.cpp}
params->setParamBlockBuffer(GPT_FRAGMENT_PROGRAM, "myParamBlock", buffer);
~~~~~~~~~~~~~

After it is bound you we free to set primitive parameters and they will be stored in the bound buffer. This buffer can then be shared between multiple GPU programs and/or GPU pipelines.

> You can also write to **GpuParamBlockBuffer** directly by calling its @bs::ct::GpuParamBlockBuffer::write method. In this case you must be careful to respect the layout of the variables in the buffer as expected by the render backend. This layout can be determined by examining the other entries in **GpuParamDesc** structure.

## Binding GPU params
Once **GpuParams** has been created and populated with necessary data, you can bind it to the GPU by calling @bs::ct::RenderAPI::setGpuParams. 

~~~~~~~~~~~~~{.cpp}
// This should be called after the pipeline expecting these parameters is bound
RenderAPI::instance().setGpuParams(params);
~~~~~~~~~~~~~

# Vertex input
Vertex GPU programs provide information about their inputs in the form of a **VertexDeclaration**. This is the same structure that we used for describing per-vertex components while creating a mesh. Per-vertex input declaration can be retrieved from a GPU program by calling @bs::ct::GpuProgram::getInputDeclaration.

~~~~~~~~~~~~~{.cpp}
SPtr<GpuProgram> vertProg = ...;
SPtr<VertexDeclaration> inputs = vertProg->getInputDeclaration();
~~~~~~~~~~~~~

Input declaration can be used for creating meshes or vertex buffers that provide per-vertex information that a GPU program expects.

> Note: This method is only available on the core thread version of **GpuProgram**.

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
 
This allows the system to map the semantics specified in **VertexDeclaration** of the bound mesh or vertex buffer to the GPU program inputs. This is not required for HLSL as HLSL has built-in support for semantics which are used instead. 
 
You can append 0-8 to the names to receive more than one element of the same name. Actual types of these elements, as well as the data stored by them doesn't need to match the names and it's up to the user to provide whatever data he needs.